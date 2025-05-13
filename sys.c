/*
 * sys.c - Syscalls implementation
 */
#include "include/interrupt.h"
#include "include/io.h"
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
#define READ_AUX_BUFF_MAX_SIZE 1024

extern Byte x;
extern Byte y;
extern Byte foreground;
extern Byte background;

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

  list_add_tail(&fillTs->parentAnchor, &(current()->fills));

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
  update_process_state_rr(&(fill->task), &readyqueue);

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

  list_del(&(current()->parentAnchor));

  struct list_head *e;
  list_for_each(e, &(current()->fills)) {
    list_head_to_task_struct(e)->pare = idle_task;
    list_add_tail(e, &(idle_task->fills));
  }
  update_process_state_rr(current(), &freequeue);
  sched_next_rr();
}

void sys_block() {
  struct task_struct *ct = current();
  if (ct->pending_unblocks == 0) {
    update_process_state_rr(current(), &blocked);
    sched_next_rr();
  } else
    --(ct->pending_unblocks);
}
char bufferAux[WRITE_AUX_BUFF_MAX_SIZE];

int sys_write(int fd, char *buffer, int size) {
  int checkFd = check_fd(fd, ESCRIPTURA);
  if (checkFd)
    return checkFd;
  else if (buffer == NULL) return -EFAULT;
  else if (size <= 0) return -EINVAL;
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
  int ret = -1;
  list_for_each(e, &(current()->fills)) {
    struct task_struct *fill = list_head_to_task_struct(e);
    if (fill->PID == pid) {
      if (fill->current_state == ST_BLOCKED)
        update_process_state_rr(fill, &readyqueue);
      else
        ++(fill->pending_unblocks);
      ret = 0;
    }
  }
  return ret;
}

char tmpbuff[READ_AUX_BUFF_MAX_SIZE];

int sys_read(char *b, int maxchars) {
  if (b == NULL)
    return -EFAULT;
  else if (maxchars < 0)
    return -EINVAL;
  else if (maxchars >= READ_AUX_BUFF_MAX_SIZE)
    return -EINVAL;

  if (kb_buffer_size() < maxchars)
    update_process_state_rr(current(), &read_blocked);

  while (kb_buffer_size() < maxchars) {
    sched_next_rr();
  }

  for (int i = 0; i < maxchars; ++i) {
    tmpbuff[i] = kb_buffer_pop();
  }
  copy_to_user(tmpbuff, b, maxchars);
  update_process_state_rr(current(), &readyqueue);
  sched_next_rr();

  return maxchars * sizeof(char);
}

int sys_gettime() { return zeos_tick; }

int sys_gotoxy(int novaX, int novaY) 
{
	if(novaX < 0 || novaX >= 25 || novaY < 0 || novaY >= 80) return -EINVAL; //NUM_ROWS = 25, NUM_COLUMNS = 80
	
	x = (Byte) novaX;
	y = (Byte) novaY;
	return novaX + novaY;
}

int sys_set_color(int fg, int bg)
{
	if(fg < 0 || bg < 0 || fg > 16 || bg > 16) return -EINVAL;
	foreground = (Byte) fg;
	background = (Byte) bg;
	return 0;
}
