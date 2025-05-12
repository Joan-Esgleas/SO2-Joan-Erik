/*
 * interrupt.h - Definició de les diferents rutines de tractament d'exepcions
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>

#define IDT_ENTRIES 256
#define KB_BUFFER_SIZE 255

extern Gate idt[IDT_ENTRIES];
extern Register idtR;
extern int zeos_tick;

struct Key_buffer {
  char buffer[KB_BUFFER_SIZE];
  unsigned int head;
  unsigned int tail;
};

extern struct Key_buffer kb_buffer;

void kb_buffer_push(char c);
char kb_buffer_pop();
unsigned int kb_buffer_size();

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);
void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

void setIdt();

#endif  /* __INTERRUPT_H__ */
