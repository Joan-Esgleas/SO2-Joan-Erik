/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include "errno.h"
#include "include/interrupt.h"
#include "include/io.h"
#include "include/list.h"
#include "include/mm.h"
#include "include/mm_address.h"
#include "include/types.h"
#include <io.h>
#include <mm.h>
#include <sched.h>

union task_union task[NR_TASKS] __attribute__((__section__(".data.task")));

#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif
struct semaphore semaphores[NUM_SEMS];
struct list_head read_blocked;
struct list_head freequeue;
struct list_head readyqueue;
struct task_struct *idle_task;
struct task_struct *task1;
int tick_counter;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry *get_DIR(struct task_struct *t) {
  return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry *get_PT(struct task_struct *t) {
  return (page_table_entry
              *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))
                 << 12);
}

struct task_struct *list_head_to_task_struct(struct list_head *l) {
  return (struct task_struct *)((int)l & 0xfffff000);
}

int sub_DIR_ref(struct task_struct *t) {
  int pos;

  pos = ((int)t->dir_pages_baseAddr - (int)dir_pages) /
        (TOTAL_PAGES * sizeof(page_table_entry));
  --dir_pages_num_references[pos];

  return dir_pages_num_references[pos];
}

int add_DIR_ref(struct task_struct *t) {
  int pos;

  pos = ((int)t->dir_pages_baseAddr - (int)dir_pages) /
        (TOTAL_PAGES * sizeof(page_table_entry));
  ++dir_pages_num_references[pos];

  return 1;
}

int allocate_DIR(struct task_struct *t) {
  int pos;

  pos = -1;

  for (int i = 0; i < NR_TASKS; ++i) {
    if (dir_pages_num_references[i] <= 0) {
      pos = i;
      break;
    }
  }
  if (pos < 0)
    return -ENOMEM;
  ++dir_pages_num_references[pos];
  t->dir_pages_baseAddr = (page_table_entry *)&dir_pages[pos];

  return 1;
}

int allocate_heap(struct task_struct *t) {
  int pos;

  pos = -1;

  for (int i = 0; i < NR_TASKS; ++i) {
    if (heap_zones[i].ref <= 0) {
      pos = i;
      break;
    }
  }
  if (pos < 0)
    return -ENOMEM;
  t->my_heap = &heap_zones[pos];
  t->my_heap->ref = 1;
  t->my_heap->size = 0;
  return 1;
}

void cpu_idle(void) {
  __asm__ __volatile__("sti" : : : "memory");

  while (1) {
    ;
  }
}

void init_idle(void) {
  struct list_head *e = list_first(&freequeue);
  struct task_struct *ct = list_head_to_task_struct(e);
  list_del(e);

  ct->PID = 0;
  ct->pending_unblocks = 0;
  ct->current_state = ST_READY;
  INIT_LIST_HEAD(&(ct->fills));
  INIT_LIST_HEAD(&(ct->waitList));
  set_quantum(ct, 0);
  allocate_DIR(ct);
  ((union task_union *)ct)->stack[KERNEL_STACK_SIZE - 1] =
      (unsigned long)cpu_idle;
  ((union task_union *)ct)->stack[KERNEL_STACK_SIZE - 2] = (unsigned long)0;
  ct->k_esp =
      (unsigned long)&(((union task_union *)ct)->stack[KERNEL_STACK_SIZE - 2]);
  idle_task = ct;
}

void init_task1(void) {
  struct list_head *e = list_first(&freequeue);
  struct task_struct *ct = list_head_to_task_struct(e);

  ct->PID = 1;
  ct->pending_unblocks = 0;
  allocate_heap(ct);
  update_process_state_rr(ct, NULL);
  INIT_LIST_HEAD(&(ct->fills));
  INIT_LIST_HEAD(&(ct->waitList));
  set_quantum(ct, DEFAULT_QUANTUM_TICKS);
  tick_counter = get_quantum(ct);
  allocate_DIR(ct);

  set_user_pages(ct);
  tss.esp0 =
      (unsigned long)&(((union task_union *)ct)->stack[KERNEL_STACK_SIZE]);
  writeMSR(0x175, (unsigned long)&(
                      ((union task_union *)ct)->stack[KERNEL_STACK_SIZE]));
  set_cr3(ct->dir_pages_baseAddr);
  task1 = ct;
}

void inner_task_switch(union task_union *new) {
  tss.esp0 = (unsigned long)&(new->stack[KERNEL_STACK_SIZE]);
  writeMSR(0x175, (unsigned long)&(new->stack[KERNEL_STACK_SIZE]));
  if (new->task.dir_pages_baseAddr != current()->dir_pages_baseAddr)
    set_cr3(get_DIR(&new->task));

  cambio_stack(&current()->k_esp, new->task.k_esp);
}

void init_heap() {
  for (int i = 0; i < NR_TASKS; ++i) {
    heap_zones[i].size = 0;
    heap_zones[i].ref = 0;
  }
}

void init_sched() {
  INIT_LIST_HEAD(&blocked);
  INIT_LIST_HEAD(&read_blocked);
  INIT_LIST_HEAD(&freequeue);
  INIT_LIST_HEAD(&readyqueue);
  init_heap();
  for (int i = 0; i < NUM_SEMS; i++)
    semaphores[i].free = 1;

  for (int i = 0; i < NR_TASKS; i++) {
    task[i].task.PID = -1;
    update_process_state_rr(&(task[i].task), &freequeue);
  }
}

int get_quantum(struct task_struct *t) { return t->quantum; }

void set_quantum(struct task_struct *t, int new_quantum) {
  t->quantum = new_quantum;
}

void sched_next_rr() {
  struct list_head *e;
  struct task_struct *nt;

  if (list_empty(&readyqueue)) {
    nt = idle_task;
  } else {
    e = list_first(&readyqueue);
    nt = list_head_to_task_struct(e);
  }

  if (current()->current_state == ST_RUN)
    update_process_state_rr(current(), &readyqueue);

  update_process_state_rr(nt, NULL);
  tick_counter = get_quantum(nt);
  if (current()->PID != nt->PID) {
    task_switch((union task_union *)nt);
  }
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest) {
  struct list_head *current_list_head = &(t->list);
  // Check de integridad
  if (t->PID < 0) {
    t->current_state = ST_FREE;
    list_add_tail(current_list_head, &freequeue);
    return;
  }

  if (t->PID > 0 && t->current_state != ST_RUN)
    list_del(current_list_head);

  if (dest == NULL) {
    t->current_state = ST_RUN;
    return;
  } else if (dest == &readyqueue) {
    t->current_state = ST_READY;
  } else if (dest == &blocked) {
    t->current_state = ST_BLOCKED;
  } else if (dest == &read_blocked) {
    t->current_state = ST_READBLOCKED;
  } else if (dest == &freequeue) {
    t->current_state = ST_FREE;
  }

  if (t->PID != 0)
    list_add_tail(current_list_head, dest);
}

int needs_sched_rr() { return (tick_counter <= 0); }

void update_sched_data_rr() {
  if (tick_counter > 0)
    --tick_counter;
}

struct task_struct *current() {
  int ret_value;

  __asm__ __volatile__("movl %%esp, %0" : "=g"(ret_value));
  return (struct task_struct *)(ret_value & 0xfffff000);
}
