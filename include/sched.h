/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <mm_address.h>
#include <types.h>

#define NR_TASKS 10
#define KERNEL_STACK_SIZE 1024
#define DEFAULT_QUANTUM_TICKS 10

enum state_t { ST_RUN, ST_FREE, ST_READY,  ST_BLOCKED, ST_READBLOCKED };

struct task_struct {
  int PID; /* Process ID. This MUST be the first field of the struct. */
  page_table_entry *dir_pages_baseAddr;
  struct list_head list;
  unsigned long k_esp;
  int quantum;
  int pending_unblocks;
  struct list_head fills;
  struct list_head parentAnchor;
  struct list_head waitList;
  struct list_head waitAnchor;
  struct list_head semList;
  struct task_struct * pare;
  enum state_t current_state;
};

struct semaphore {
  int value;
  int id;
  int creatorPID;
  struct list_head list;
  struct list_head blockedThreads;
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE]; /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */

extern struct list_head blocked;
extern struct list_head read_blocked;
extern struct list_head freequeue;
extern struct list_head readyqueue;
extern struct list_head semaphores;
extern struct task_struct *idle_task;
extern struct task_struct *task1;

extern int tick_counter;

#define KERNEL_ESP(t) (DWord) & (t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

void cambio_stack(unsigned long *save_sp, unsigned long new_sp);

struct task_struct *current();

void task_switch(union task_union *t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);
int add_DIR_ref(struct task_struct *t);
int sub_DIR_ref(struct task_struct *t);

page_table_entry *get_PT(struct task_struct *t);

page_table_entry *get_DIR(struct task_struct *t);

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();
int get_quantum(struct task_struct *t);
void set_quantum(struct task_struct *t, int new_quantum);

#endif /* __SCHED_H__ */
