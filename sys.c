/*
 * sys.c - Syscalls implementation
 */
#include "include/interrupt.h"
#include "include/mm_address.h"
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern struct list_head freequeue, readyqueue;

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
  if (list_empty(&freequeue))
    return -ENOMEM;
  struct list_head *e = list_first(&freequeue);
  struct task_struct *hijo_task = list_head_to_task_struct(e);
  list_del(e);
  struct task_struct *padre_task = current();
  copy_data(padre_task, hijo_task, sizeof(union task_union));

  hijo_task->PID = pidGlobal++;
  allocate_DIR(hijo_task);

  // alloc de DATA al hijo
  int frames[NUM_PAG_DATA];
  for (int i = 0; i < NUM_PAG_DATA; ++i) {
    int id_frame = alloc_frame();
    if (id_frame < 0) {
      for (int j = 0; j < i; ++j) {
        free_frame(frames[j]);
      }
      list_add_tail(e, &freequeue);
      return -ENOMEM;
    }
    frames[i] = id_frame;
  }

  page_table_entry *hijo_PT = get_PT(hijo_task);
  page_table_entry *padre_PT = get_PT(padre_task);

  // Enlaza el kernel:
  for (int i = 0; i < NUM_PAG_KERNEL; ++i) {
    unsigned int frame_kernel = get_frame(padre_PT, i);
    set_ss_pag(hijo_PT, i, frame_kernel);
  }

  // Enlaza el codigo:
  for (int i = PAG_LOG_INIT_CODE; i < PAG_LOG_INIT_CODE + NUM_PAG_CODE; ++i) {
    unsigned int frame_code = get_frame(padre_PT, i);
    set_ss_pag(hijo_PT, i, frame_code);
  }

  // Copia en mem libre data y pila
  int init_free_mem = PAG_LOG_INIT_CODE + (NUM_PAG_CODE * PAGE_SIZE);

  for(int i = 0; i < NUM_PAG_DATA*PAGE_SIZE; i+=PAGE_SIZE) {
    set_ss_pag(padre_PT, init_free_mem + i, get_frame(hijo_PT, PAG_LOG_INIT_DATA + i));
    copy_data(PAG_LOG_INIT_DATA + i, init_free_mem + i, PAGE_SIZE);
    del_ss_pag(padre_PT, init_free_mem + i);
  }

  // Flush TLB
  set_cr3(get_DIR(padre_task));
  
  ((union task_union *) hijo_task)->stack[KERNEL_STACK_SIZE-19] = (unsigned long) 0;
  ((union task_union *) hijo_task)->stack[KERNEL_STACK_SIZE-18] = (unsigned long) &ret_from_fork;

  hijo_task->k_esp = &((union task_union*)hijo_task)->stack[KERNEL_STACK_SIZE-19];
  
  //j) en el document:
  list_add_tail(&(hijo_task->list), &readyqueue);

  //k) en el document:
  return hijo_task->PID;
}

void sys_exit() {}

int sys_write(int fd, char *buffer, int size) {
  char bufferAux[size];
  int checkFd = check_fd(fd, ESCRIPTURA);
  if (checkFd)
    return checkFd;
  else if (buffer == NULL)
    return EFAULT;
  else if (size <= 0)
    return EINVAL;
  else {
    copy_from_user(buffer, bufferAux, size);
    int ret = sys_write_console(bufferAux, size);
    return ret;
  }
}

int sys_gettime() { return zeos_tick; }
