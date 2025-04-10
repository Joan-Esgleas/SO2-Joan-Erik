/*
 * sys.c - Syscalls implementation
 */
#include "include/interrupt.h"
#include "include/list.h"
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define WRITE_AUX_BUFF_MAX_SIZE 1024

int pidGlobal = 1000;

int ret_from_fork() { return 0; }

int check_fd(int fd, int permissions) {
  if (fd != 1)
    return -9; /*EBADF*/
  if (permissions != ESCRIPTURA)
    return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall() { return -38; /*ENOSYS*/ }

int sys_getpid() { return current()->PID; }

int sys_fork() {
  // a) en el document:
  if (list_empty(&freequeue))
    return -ENOMEM; // Retorna un error de que no hi ha memoria

  // S'agafa un PCB de la freequeue
  struct list_head *e = list_first(&freequeue);
  union task_union *fill = (union task_union *)list_head_to_task_struct(e);
  struct task_struct *fillTs = list_head_to_task_struct(e);
  list_del(e);

  // b) en el document:
  // Es fa una copia del task_union del pare en el fill
  copy_data(current(), fill, sizeof(union task_union));

  // c) en el document:
  // Obtenim un nou directori pel fill
  allocate_DIR((struct task_struct *)fill);

  // d) en el document:
  // Obtenim les pagines pel nostre fill
  int frames[NUM_PAG_DATA];
  for (int i = 0; i < NUM_PAG_DATA; i++) {
    frames[i] = alloc_frame();

    if (frames[i] < 0) {
      // Si no hi ha mes espai, s'allibera el espai inutil que haviem reservat
      for (int j = 0; j < i; j++) {
        free_frame(frames[j]);
      }
      list_add_tail(&fill->task.list, &freequeue);
      return -ENOMEM;
    }
  }

  // e) en el document:
  // Obtenim els punters de les TPs del pare i del fill
  page_table_entry *fillTP = get_PT((struct task_struct *)fill);
  page_table_entry *pareTP = get_PT(current());

  // e) -> i)
  // Fer que pagines de kernel i codi apuntin a les del pare
  for (int i = 0; i < NUM_PAG_KERNEL; i++)
    set_ss_pag(fillTP, i, get_frame(pareTP, i));

  for (int i = 0; i < NUM_PAG_CODE; i++) {
    set_ss_pag(fillTP, PAG_LOG_INIT_CODE + i,
               get_frame(pareTP, PAG_LOG_INIT_CODE + i));
  }

  // e) -> ii)
  // Vinculem les pagines logiques amb els frames abans agafats per data+stack
  for (int i = 0; i < NUM_PAG_DATA; i++)
    set_ss_pag(fillTP, PAG_LOG_INIT_DATA + i, frames[i]);

  // RECORDATORI PER AQUEST APARTAT, S'HA IMPLEMENTAT D'UNA FORMA QUE ES MAPEJEN
  // MOLTES PAGINES LOGIQUES AMB MOLTES FISIQUES, SI FUNCIONA, DESPRES PROVAR DE
  // FER-HO AMB NOMES LA PAGINA LOGICA QUE VAGI DIRECTAMENT DESPRES DE
  // NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA I ANAR FENT set_ss_pag(pareTP,
  // LA PAGINA, get_frame(fillTP, i)) I DESPRES del_ss_pag(pareTP, LA PAGINA)

  // f) en el document:
  // Creem una variable que ens fara accedir a les pagines lliures del pare
  int offset = NUM_PAG_DATA + NUM_PAG_CODE;

  for (int i = PAG_LOG_INIT_DATA; i < (PAG_LOG_INIT_DATA + NUM_PAG_DATA); i++) {
    // Ara creem una pagina de copia que apunti al mateix frame que la DATA del
    // fill
    set_ss_pag(pareTP, i + offset, get_frame(fillTP, i));
    // Ara copiem tot en aquesta pagina de copia vinculada al frame del DATA del
    // fill perque aquest tingui la info
    copy_data((void *)(i << 12), (void *)((i + offset) << 12), PAGE_SIZE);
    // Com ja tenim la info copiada, esborrem la pagina creada
    del_ss_pag(pareTP, i + offset);
  }

  // Forçem flush en el TLB per borrar els mapejos temporals
  set_cr3(get_DIR(current()));

  // g) en el document:
  fill->task.PID = pidGlobal++;

  // Informacio que hem d'afegir per el block i el unblock
  fillTs->pare = current();

  list_add_tail(&fillTs->pareList, &(current()->fills));

  INIT_LIST_HEAD(&(fillTs->fills));

  fillTs->pending_unblocks = 0;

  // i) en el document:
  // posem el ebp fals de 0 i el ret_from_fork per posar a 0 el valor de retorn
  // de la funcio
  ((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 19] = (unsigned long)0;
  ((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 18] =
      (unsigned long)&ret_from_fork;
  // fem que el kernel esp apunti al 0 del ebp per fer el truquillo
  fill->task.k_esp = &((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 19];

  // j) en el document:
  list_add_tail(&(fill->task.list), &readyqueue);

  // k) en el document:
  return fill->task.PID;
}

void sys_exit() {
  struct task_struct *ct = current();
  page_table_entry *ctTP = get_PT(ct);

  for (int i = PAG_LOG_INIT_DATA; i < PAG_LOG_INIT_DATA + NUM_PAG_DATA; i++) {
    free_frame(get_frame(ctTP, i));
    del_ss_pag(ctTP, i);
  }

  ct->PID = -1;
  ct->dir_pages_baseAddr = NULL;
  ct->pare = NULL;

  list_del(&(current()->pareList));

  struct list_head *e;
  list_for_each(e, &(current()->fills)) {
    list_head_to_task_struct(e)->pare = idle_task;
    list_add_tail(e, &(idle_task->fills));
  }

  update_process_state_rr(current(), &freequeue);
  if (list_empty(&readyqueue))
    task_switch((union task_union *)idle_task);
  else {
    e = list_first(&readyqueue);
    struct task_struct *nt = list_head_to_task_struct(e);
    update_process_state_rr(nt, NULL);
    tick_counter = get_quantum(nt);
    task_switch((union task_union *)nt);
  }
}

void sys_block() {
  struct task_struct *ct = current();
  if (ct->pending_unblocks == 0) {
    list_add_tail(&(ct->list), &blocked);
    struct list_head *e;
    if (list_empty(&readyqueue))
      task_switch((union task_union *)idle_task);
    else {
      e = list_first(&readyqueue);
      struct task_struct *nt = list_head_to_task_struct(e);
      update_process_state_rr(nt, NULL);
      tick_counter = get_quantum(nt);
      task_switch((union task_union *)nt);
    }
  } else
    --(ct->pending_unblocks);
}
char bufferAux[WRITE_AUX_BUFF_MAX_SIZE];

int sys_write(int fd, char *buffer, int size) {
  int checkFd = check_fd(fd, ESCRIPTURA);
  if (checkFd)
    return checkFd;
  else if (buffer == NULL)
    return EFAULT;
  else if (size <= 0)
    return EINVAL;
  else {
    int ret = 0;
    while (size > WRITE_AUX_BUFF_MAX_SIZE) {
      copy_from_user(buffer, bufferAux, WRITE_AUX_BUFF_MAX_SIZE);
      ret += sys_write_console(bufferAux, WRITE_AUX_BUFF_MAX_SIZE);
      buffer += WRITE_AUX_BUFF_MAX_SIZE * sizeof(char);
      size -= WRITE_AUX_BUFF_MAX_SIZE;
    }
    copy_from_user(buffer, bufferAux, size);
    ret += sys_write_console(bufferAux, size);
    return ret;
  }
}
int sys_unblock(int pid) {
  struct list_head *e;

  list_for_each(e, &(current()->fills)) {
    struct task_struct *fill = list_head_to_task_struct(e);

    if (fill->PID == pid) {
      struct list_head *e2;

      int estaBloquejat = 0;

      list_for_each(e2, &blocked) {
        struct task_struct *block = list_head_to_task_struct(e2);
        if (block->PID == pid) {
          list_del(&(block->list));
          list_add_tail(&(block->list), &readyqueue);
          estaBloquejat = 1;
        }
      }

      if (estaBloquejat == 0)
        ++(fill->pending_unblocks);

      return 0;
    }
  }

  return -1;
}

int sys_gettime() { return zeos_tick; }
