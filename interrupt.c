/*
 * interrupt.c -
 */
#include "include/interrupt.h"
#include "include/io.h"
#include "include/list.h"
#include <hardware.h>
#include <interrupt.h>
#include <io.h>
#include <sched.h>
#include <segment.h>
#include <types.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register idtR;
struct Key_buffer kb_buffer = {.head = 0, .tail = 0, .can_be_read = 1};

void kb_buffer_push(char c) {
  unsigned int next = (kb_buffer.head + 1) % KB_BUFFER_SIZE;
  // if (next != kb_buffer.tail) { // evitar sobreescribir si está lleno
  kb_buffer.buffer[kb_buffer.head] = c;
  kb_buffer.head = next;
  //}
}

char kb_buffer_pop() {
  char c = kb_buffer.buffer[kb_buffer.tail];
  kb_buffer.tail = (kb_buffer.tail + 1) % KB_BUFFER_SIZE;
  return c;
}

char kb_buffer_emtpy() {
  if (kb_buffer.head == kb_buffer.tail)
    return 0;
  return 1;
}

unsigned int kb_buffer_size() {
  if (kb_buffer.head >= kb_buffer.tail) {
    return kb_buffer.head - kb_buffer.tail;
  } else {
    return KB_BUFFER_SIZE - kb_buffer.tail + kb_buffer.head;
  }
}

char kb_buffer_can_be_read() { return kb_buffer.can_be_read; }

int kb_buffer_ocupar() {
  if (kb_buffer.can_be_read) {
    kb_buffer.can_be_read = 0;
    return 0;
  }
  return -1;
}

void kb_buffer_desocupar() { kb_buffer.can_be_read = 1; }

char char_map[] = {'\0', '\0', '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
                   '9',  '0',  '\'', '¡',  '\0', '\0', 'q',  'w',  'e',  'r',
                   't',  'y',  'u',  'i',  'o',  'p',  '`',  '+',  '\0', '\0',
                   'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  'ñ',
                   '\0', 'º',  '\0', 'ç',  'z',  'x',  'c',  'v',  'b',  'n',
                   'm',  ',',  '.',  '-',  '\0', '*',  '\0', '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                   '\0', '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
                   '2',  '3',  '0',  '\0', '\0', '\0', '<',  '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

void setInterruptHandler(int vector, void (*handler)(),
                         int maxAccessibleFromPL) {
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00; /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags = flags;
  idt[vector].highOffset = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL) {
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  // flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00; /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags = flags;
  idt[vector].highOffset = highWord((DWord)handler);
}

void keyboard_handler();
void clock_handler();
void syscall_handler_sysenter();
void system_call_handler();
void writeMSR(unsigned long msr, unsigned long val);
void page_fault_handler_2();
void task_switch(union task_union *new);

void setIdt() {
  /* Program interrups/exception service routines */
  idtR.base = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;

  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  // 3 usuari pot invocar
  // 0 nomes sistema
  setInterruptHandler(33, keyboard_handler, 0);
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(14, page_fault_handler_2, 0);

  setTrapHandler(0x80, system_call_handler, 3);

  writeMSR(0x174, __KERNEL_CS);
  writeMSR(0x175, INITIAL_ESP);
  writeMSR(0x176, (unsigned long)syscall_handler_sysenter);

  set_idt_reg(&idtR);
}

void stringNumHex(char *res, unsigned long num) {
  char mapaHex[] = "0123456789ABCDEF";

  for (int i = 7; i >= 0; --i) {
    res[i] = mapaHex[num % 16];
    num /= 16;
  }

  res[8] = '\0';
}

void keyboard_routine() {
  unsigned char read_inp = inb(0x60);
  char is_break = read_inp >> 6;
  char p_ch = read_inp & 0b01111111;
  if (p_ch < 0 && p_ch > 98)
    return;
  if (!is_break) {
    char char_print = char_map[p_ch];
    if (char_print == '\0')
      char_print = 'C';
    kb_buffer_push(char_print);

    struct list_head *e;
    if (!list_empty(&read_blocked)) {
      e = list_first(&read_blocked);
      struct task_struct *nt = list_head_to_task_struct(e);
      task_switch((union task_union *)nt);
    }
    printc_xy(0, 0, char_print);
  }
}

void clock_routine() {
  zeos_show_clock();
  zeos_tick += 10;
  update_sched_data_rr();
  if (needs_sched_rr()) {
    if (current()->PID != 0)
      update_process_state_rr(current(), &readyqueue);
    sched_next_rr();
  }
}

void page_fault_routine_2(unsigned long error, unsigned long eip) {
  char errorStr[9];
  stringNumHex(errorStr, eip);

  printk("\nProcess generates a PAGE FAULT exception at EIP: 0x");
  printk(errorStr);
  printk("\n");
  while (1)
    ;
}
