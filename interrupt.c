/*
 * interrupt.c -
 */
#include "include/interrupt.h"
#include "include/io.h"
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <sched.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
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
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
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

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void keyboard_handler();
void clock_handler();
void syscall_handler_sysenter();
void system_call_handler();
void writeMSR(unsigned long msr, unsigned long val);
void page_fault_handler_2();
void task_switch(union task_union *new);

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
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

void stringNumHex(char * res, unsigned long num) {
	char mapaHex[] = "0123456789ABCDEF";
	
	for(int i = 7; i >= 0; --i){
		res[i] = mapaHex[num%16];
		num /= 16;
	}
	
	res[8] = '\0';

}

void keyboard_routine() {
  unsigned char read_inp = inb(0x60);
  char is_break = read_inp >> 6;
  char p_ch = read_inp & 0b01111111;
  if(p_ch < 0 && p_ch > 98) return;
  if (!is_break) {
      char char_print = char_map[p_ch];
      if(char_print == '\0') char_print = 'C';
      printc_xy(0,0, char_print);
  

      char pidProceso[9];
      stringNumHex(pidProceso, current()->PID);
      printk("\nProceso actual con PID: 0x");
      printk(pidProceso);
      printk("\n");

      if(current()->PID == task1->PID)
        task_switch((union task_union*)idle_task);
      else
        task_switch((union task_union*)task1);

      stringNumHex(pidProceso, current()->PID);
      printk("Ha cambiado a proceso con PID: 0x");
      printk(pidProceso);
      printk("\n");

  }
}



void clock_routine(){
  zeos_show_clock();
  zeos_tick += 10;
}



void page_fault_routine_2(unsigned long error, unsigned long eip){
  char errorStr[9];
  stringNumHex(errorStr, eip);
  
  printk("\nProcess generates a PAGE FAULT exception at EIP: 0x");
  printk(errorStr);
  printk("\n");
  while(1);
}
