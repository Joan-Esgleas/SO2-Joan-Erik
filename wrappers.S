#include <asm.h>
#include <errno.h>

ENTRY(write)
	pushl %ebp
	mov %esp,%ebp
	
	//Se guardan estos registros porque hacemos fast syscall
	pushl %edx
	pushl %ecx
	
	//Se guardan los valores en el orden que se guardan en el SAVE_ALL
	mov 0x08(%ebp), %ebx  //fd -> ebx
	mov 0x0c(%ebp), %ecx  //buffer -> ecx
	mov 0x10(%ebp), %edx //size -> edx
	
	//Ponemos el numero de la syscall en el eax
	movl $4, %eax
	
	pushl $writeRet
	//Fake dynamic link
	pushl %ebp
	mov %esp,%ebp
	
	sysenter
	
writeRet:
	
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
	jge writeSinError
	negl %eax
	movl %eax, errno
	movl -1, %eax
	
writeSinError:

	popl %ebp
	ret
