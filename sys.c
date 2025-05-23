/*
 * sys.c - Syscalls implementation
 */
#include "include/interrupt.h"
#include "include/io.h"
#include "include/list.h"
#include "include/mm.h"
#include "include/mm_address.h"
#include <devices.h>
#include <linux/limits.h>
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

int semIDglobal = 100000;
int semIDsystem = 0;

int ret_from_fork() { return 0; }
int ret_from_new_thread() { return 0; }


int sys_semDestroy_extended(int semid, int userMode);

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
  allocate_heap((struct task_struct *)fill);
  fillTs->my_heap = (current()->my_heap);

  int frames[NUM_PAG_DATA + *(current()->my_heap)];
  for (int i = 0; i < NUM_PAG_DATA + *(current()->my_heap); i++) {
    frames[i] = alloc_frame();

    if (frames[i] < 0) {
      for (int j = 0; j < i; j++) {
        free_frame(frames[j]);
      }
      list_add_tail(&fill->task.list, &freequeue);
      return -ENOMEM;
    }
  }

  page_table_entry *fillTP = get_PT((struct task_struct *)fill);
  page_table_entry *pareTP = get_PT(current());

  for (int i = 0; i < NUM_PAG_KERNEL; i++)
    set_ss_pag(fillTP, i, get_frame(pareTP, i));

  for (int i = 0; i < NUM_PAG_CODE; i++) {
    set_ss_pag(fillTP, PAG_LOG_INIT_CODE + i,
               get_frame(pareTP, PAG_LOG_INIT_CODE + i));
  }

  for (int i = 0; i < NUM_PAG_DATA; i++)
    set_ss_pag(fillTP, PAG_LOG_INIT_DATA + i, frames[i]);

  for (int i = 0; i < *(current()->my_heap); i++)
    set_ss_pag(fillTP, PAG_LOG_INIT_HEAP + i, frames[NUM_PAG_DATA + i]);

  int offset = NUM_PAG_DATA + NUM_PAG_CODE + *(current()->my_heap);

  for (int i = PAG_LOG_INIT_DATA; i < (PAG_LOG_INIT_DATA + NUM_PAG_DATA); i++) {
    set_ss_pag(pareTP, i + offset, get_frame(fillTP, i));
    copy_data((void *)(i << 12), (void *)((i + offset) << 12), PAGE_SIZE);
    del_ss_pag(pareTP, i + offset);
  }

  offset = NUM_PAG_DATA + *(current()->my_heap);
  
  for (int i = PAG_LOG_INIT_HEAP;
       i < (PAG_LOG_INIT_HEAP + *(current()->my_heap)); i++) {
    set_ss_pag(pareTP, i + offset, get_frame(fillTP, i));
    copy_data((void *)(i << 12), (void *)((i + offset) << 12), PAGE_SIZE);
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
  INIT_LIST_HEAD(&(fillTs->waitList));

  fillTs->pending_unblocks = 0;

  // i) en el document:
  // posem el ebp fals de 0 i el ret_from_fork per posar a 0 el valor de retorn
  // de la funcio
  ((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 19] = (unsigned long)0;
  ((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 18] =
      (unsigned long)&ret_from_fork;
  // fem que el kernel esp apunti al 0 del ebp per fer el truquillo
  fill->task.k_esp =
      (unsigned long)&((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 19];

  // j) en el document:
  set_quantum(&(fill->task), DEFAULT_QUANTUM_TICKS);
  update_process_state_rr(&(fill->task), &readyqueue);

  // k) en el document:
  return fill->task.PID;
}

int sys_create_thread(void (*function)(void *arg), void *stack,
                      void *parameter) {
  if (list_empty(&freequeue))
    return -ENOMEM; // Retorna un error de que no hi ha memoria

  else if (!access_ok(VERIFY_READ, function, sizeof(long)))
    return -EFAULT;
  else if (!access_ok(VERIFY_WRITE, stack, sizeof(long)))
    return -EFAULT;

  // S'agafa un PCB de la freequeue
  struct list_head *e = list_first(&freequeue);
  union task_union *fill = (union task_union *)list_head_to_task_struct(e);
  struct task_struct *fillTs = list_head_to_task_struct(e);
  list_del(e);

  copy_data(current(), fill, sizeof(union task_union));
  add_DIR_ref((struct task_struct *)fill);

  // set_cr3(get_DIR(current()));

  fill->task.PID = pidGlobal++;
  fillTs->pare = current();
  list_add_tail(&fillTs->parentAnchor, &(current()->fills));
  INIT_LIST_HEAD(&(fillTs->fills));
  INIT_LIST_HEAD(&(fillTs->waitList));
  fillTs->pending_unblocks = 0;

  ((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 19] = (unsigned long)0;
  ((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 18] =
      (unsigned long)&ret_from_new_thread;

  ((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 5] =
      (unsigned long)function;

  // Hacemos push de los parametros
  unsigned long *new_stack = stack;
  --new_stack;
  *new_stack = (unsigned long)parameter;
  --new_stack;    // Apuntem a retun addr
  *new_stack = 0; // la return addres sera 0

  ((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 2] =
      (unsigned long)new_stack;

  // fem que el kernel esp apunti al 0 del ebp per fer el truquillo
  fill->task.k_esp =
      (unsigned long)&((union task_union *)fill)->stack[KERNEL_STACK_SIZE - 19];

  // j) en el document:
  set_quantum(&(fill->task), DEFAULT_QUANTUM_TICKS);
  update_process_state_rr(&(fill->task), &readyqueue);

  // k) en el document:
  return fill->task.PID;
}

void sys_exit() {
  struct task_struct *ct = current();
  page_table_entry *ctTP = get_PT(ct);

  for (int i = 0; i < NUM_SEMS; i++) {
    if (semaphores[i].creatorPID == ct->PID)
      sys_semDestroy_extended(semaphores[i].id, 0);
  }

  if (sub_DIR_ref(ct) <= 0) {
    for (int i = PAG_LOG_INIT_DATA; i < PAG_LOG_INIT_DATA + NUM_PAG_DATA; i++) {
      free_frame(get_frame(ctTP, i));
      del_ss_pag(ctTP, i);
    }
    for (int i = PAG_LOG_INIT_HEAP;
         i < PAG_LOG_INIT_HEAP + *(current()->my_heap); i++) {
      free_frame(get_frame(ctTP, i));
      del_ss_pag(ctTP, i);
    }
  }

  ct->PID = -1;
  ct->dir_pages_baseAddr = NULL;
  ct->pare = NULL;

  list_del(&(current()->parentAnchor));

  struct list_head *pos;
  struct list_head *tmp;

  list_for_each_safe(pos, tmp, &(current()->fills)) {
    list_head_to_task_struct(pos)->pare = idle_task;
    list_add_tail(pos, &(idle_task->fills));
  }

  list_for_each_safe(pos, tmp, &(current()->waitList)) {
    update_process_state_rr(list_head_to_task_struct(pos), &readyqueue);
    list_del(pos);
  }

  update_process_state_rr(current(), &freequeue);
  sched_next_rr();
}

void sys_block() {
  struct task_struct *ct = current();
  if (ct->pending_unblocks <= 0) {
    update_process_state_rr(current(), &blocked);
    sched_next_rr();
  } else
    --(ct->pending_unblocks);
}

int sys_wait_thread(int pid) {
  // tasks_union
  int trobat = 0;
  struct task_struct *bt;
  for (int i = 0; i < NR_TASKS; i++) {
    if (task[i].task.PID == pid) {
      bt = &task[i].task;
      trobat = 1;
      break;
    }
  }
  if (!trobat)
    return -EINVAL;

  struct task_struct *ct = current();
  list_add_tail(&(ct->waitAnchor), &(bt->waitList));
  update_process_state_rr(current(), &blocked);
  sched_next_rr();

  return bt->PID;
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
  sched_next_rr();
  return ret;
}

char bufferAux[WRITE_AUX_BUFF_MAX_SIZE];

int sys_write(int fd, char *buffer, int size) {
  int checkFd = check_fd(fd, ESCRIPTURA);
  if (checkFd)
    return checkFd;
  else if (buffer == NULL)
    return -EFAULT;
  else if (size <= 0)
    return -EINVAL;
  else if (!access_ok(VERIFY_READ, buffer, size))
    return -EFAULT;

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

char tmpbuff[READ_AUX_BUFF_MAX_SIZE];

int sys_read(char *b, int maxchars) {
  if (b == NULL)
    return -EFAULT;
  else if (maxchars < 0)
    return -EINVAL;
  else if (!access_ok(VERIFY_WRITE, b, maxchars))
    return -EFAULT;

  update_process_state_rr(current(), &read_blocked);

  if (!list_empty(&read_blocked) &&
      list_head_to_task_struct(list_first(&read_blocked))->PID !=
          current()->PID) {
    sched_next_rr();
  }

  while (kb_buffer_size() < maxchars) {
    sched_next_rr();
  }

  int i;
  for (i = 0; i < maxchars; ++i)
    tmpbuff[i] = kb_buffer_pop();

  copy_to_user(tmpbuff, b, i);
  update_process_state_rr(current(), NULL);

  return i * sizeof(char);
}

int sys_gettime() { return zeos_tick; }

int sys_gotoxy(int novaX, int novaY) {
  if (novaY < 0 || novaY >= 25 || novaX < 0 || novaX >= 80)
    return -EINVAL;

  x = (Byte)novaX;
  y = (Byte)novaY;
  return novaX + novaY;
}

int sys_set_color(int fg, int bg) {
  if (fg < 0 || bg < 0 || fg > 16 || bg > 16)
    return -EINVAL;
  foreground = (Byte)fg;
  background = (Byte)bg;
  return 0;
}

char *sys_dyn_mem(int num_pags) {

  if (num_pags == 0)
    return (char *)((PAG_LOG_INIT_HEAP + *(current()->my_heap)) << 12);
  else if (num_pags > 0) {
    int frames[num_pags];
    for (int i = 0; i < num_pags; i++) {
      frames[i] = alloc_frame();

      if (frames[i] < 0) {
        // Si no hi ha mes espai, s'allibera el espai inutil que haviem reservat
        for (int j = 0; j < i; j++) {
          free_frame(frames[j]);
          del_ss_pag(get_PT(current()),
                     PAG_LOG_INIT_HEAP + *(current()->my_heap) + j);
        }
        return (char *)(-ENOMEM);
      } else {
        set_ss_pag(get_PT(current()),
                   PAG_LOG_INIT_HEAP + *(current()->my_heap) + i, frames[i]);
      }
    }
    unsigned long ret = ((PAG_LOG_INIT_HEAP + *(current()->my_heap)) << 12);
    *(current()->my_heap) += num_pags;
    return (char *)ret;
  } else {
    num_pags = num_pags * -1;
    if (*(current()->my_heap) < num_pags)
      return (char *)-EINVAL;

    for (int i = PAG_LOG_INIT_HEAP + *(current()->my_heap) - num_pags;
         i < PAG_LOG_INIT_HEAP + *(current()->my_heap); ++i) {
      free_frame(get_frame(get_PT(current()), i));
      del_ss_pag(get_PT(current()), i);
    }
    *(current()->my_heap) -= num_pags;
    set_cr3(get_DIR(current()));
    return (char *)((PAG_LOG_INIT_HEAP + *(current()->my_heap)) << 12);
  }
}

int sys_semCreate_extended(int initial_value, int userMode) {
  int i = 0;
  while (i < NUM_SEMS) {
    if (semaphores[i].free) {
      semaphores[i].free = 0;
      semaphores[i].value = initial_value;
      semaphores[i].creatorPID = sys_getpid();
      INIT_LIST_HEAD(&(semaphores[i].blockedThreads));
      if(userMode) {
      	semaphores[i].id = semIDglobal;
      	++semIDglobal;
      	return (semIDglobal - 1);
      }
      else {
      	semaphores[i].id = semIDsystem;
      	++semIDsystem;
      	return (semIDsystem - 1);
      }
    }
    ++i;
  }
  return -1;
}

int sys_semCreate(int initial_value) {
  return sys_semCreate_extended(initial_value, 1);
}

int sys_semWait_extended(int semid, int userMode) {
  int i = 0;
  if(userMode && semid < 100000) return -EINVAL;
  if(!userMode && semid >= 100000) return -EINVAL;
  
  while (i < NUM_SEMS) {
    if (semaphores[i].id == semid) {
      if (semaphores[i].free)
        return -EINVAL;
      --(semaphores[i].value);
      if (semaphores[i].value < 0) {
        list_add_tail(&(current()->semList), &(semaphores[i].blockedThreads));
        update_process_state_rr(current(), &blocked);
        sched_next_rr();
      }
      return semaphores[i].creatorPID;
    }
    ++i;
  }
  return -1;
}

int sys_semWait(int semid) {
  return sys_semWait_extended(semid, 1);
}

int sys_semSignal_extended(int semid, int userMode) {
  int i = 0;
  if(userMode && semid < 100000) return -1;
  if(!userMode && semid >= 100000) return -1;
  
  while (i < NUM_SEMS) {
    if (semaphores[i].id == semid) {
      if (semaphores[i].free)
        return -EINVAL;
      ++(semaphores[i].value);
      if (semaphores[i].value <= 0) {
        struct list_head *e = list_first(&(semaphores[i].blockedThreads));
        struct task_struct *firstBlocked = list_head_to_task_struct(e);
        list_del(e);
 	update_process_state_rr(firstBlocked, &readyqueue);
 	sched_next_rr();
      }
      return semaphores[i].value;
    }
    ++i;
  }
  return -1;
}

int sys_semSignal(int semid) {
  return sys_semSignal_extended(semid, 1);
}

int sys_semDestroy_extended(int semid, int userMode) {
  int i = 0;
  if(userMode && semid < 100000) return -EINVAL;
  
  while (i < NUM_SEMS) {
    if (semaphores[i].id == semid) {
      if (semaphores[i].free)
        return -EINVAL;
      if (current()->PID != semaphores[i].creatorPID)
        return -EINVAL;

      // Desbloqueamos los threads que estuvieran bloqueados por el semaforo
      while (!(list_empty(&(semaphores[i].blockedThreads)))) {
        struct list_head *e = list_first(&(semaphores[i].blockedThreads));
        struct task_struct *firstBlocked = list_head_to_task_struct(e);
        semaphores[i].creatorPID = -1;
        list_del(e);
        update_process_state_rr(firstBlocked, &readyqueue);
 	sched_next_rr();
      }

      semaphores[i].free = 1;
    }
    ++i;
  }
  return -1;
}

int sys_semDestroy(int semid) {
  return sys_semDestroy_extended(semid, 1);
}
