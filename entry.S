/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>

/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %edx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %ebx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %ebx; \
      pushl %ecx; \
      pushl %edx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define RESTORE_ALL \
      popl %edx; \
      popl %ecx; \
      popl %ebx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \
      popl %eax; \
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs;

#define EOI \
      movb $0x20, %al ; \
      outb %al, $0x20 ;

ENTRY(keyboard_handler)
	SAVE_ALL // Save the current context
	EOI
	call keyboard_routine // Call the corresponding service routine
	RESTORE_ALL // Restore the context
	iret

ENTRY(cambio_stack)
  movl 4(%esp), %eax
  mov %ebp, (%eax)
  mov 8(%esp), %esp
  popl %ebp
  ret

ENTRY(task_switch)
  push %ebp
  movl %esp, %ebp

  pushl %esi
  pushl %edi
  pushl %ebx

  push 8(%ebp)
  call inner_task_switch // Call the corresponding service routine 

  addl $4, %esp


  popl %ebx
  popl %edi
  popl %esi

  mov %ebp, %esp
  pop %ebp
  ret

ENTRY(clock_handler)
	SAVE_ALL // Save the current context
	EOI
	call clock_routine// Call the corresponding service routine
	RESTORE_ALL // Restore the context
	iret

ENTRY(page_fault_handler_2)
	call page_fault_routine_2

ENTRY(writeMSR)
      push %ebp
      mov %esp, %ebp

      mov 0x8(%ebp), %ecx
      movl $0, %edx       
      mov 0xc(%ebp), %eax
      wrmsr

      pop %ebp
      ret

ENTRY(system_call_handler)
      SAVE_ALL // Save the current context
      cmpl $0, %EAX // Is syscall number negative?
      jl err // If it is, jump to return an error
      cmpl $MAX_SYSCALL, %EAX // Is syscall greater than MAX_SYSCALL (4)?
      jg err // If it is, jump to return an error
      call *sys_call_table(, %EAX, 0x04) // Call the corresponding service routine
      jmp fin // Finish
      err:
      movl $-ENOSYS, %EAX // Move to EAX the ENOSYS error
      fin:
      movl %EAX, 0x18(%esp) // Change the EAX value in the stack
      RESTORE_ALL // Restore the context
      iret

ENTRY(syscall_handler_sysenter)
	push $__USER_DS //Guarda el contexto que guardaria el INT
	push %EBP // User stack address
	pushfl
	push $__USER_CS
	push 4(%EBP) // User return address
	SAVE_ALL
	cmpl $0, %EAX
	jl sysenter_err
	cmpl $MAX_SYSCALL, %EAX
	jg sysenter_err
	call *sys_call_table(, %EAX, 0x04)
	jmp sysenter_fin
	
sysenter_err:

	movl $-ENOSYS, %EAX
	
sysenter_fin:

	movl %EAX, 0x18(%ESP)
	RESTORE_ALL
	movl (%ESP), %EDX // Return address
	movl 12(%ESP), %ECX // User stack address
	sti // Enable interrupts again
	sysexit
