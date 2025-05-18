#include <asm.h>
#include <segment.h>

ENTRY (sys_call_table)
	.long sys_ni_syscall//0
	.long sys_exit//1
	.long sys_fork//2
	.long sys_ni_syscall//3
	.long sys_write//4
	.long sys_block//5
	.long sys_unblock//6
	.long sys_read//7
	.long sys_gotoxy//8
	.long sys_set_color//9
	.long sys_gettime//10
	.long sys_create_thread//11
	.long sys_wait_thread//12
	.long sys_dyn_mem//13
	.long sys_semCreate//14
	.long sys_semWait//15
	.long sys_semSignal//16
	.long sys_semDestroy//17
	.long sys_ni_syscall//18
	.long sys_ni_syscall//19
	.long sys_getpid//20
			//se pueden implementar mas...
.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table)/4


