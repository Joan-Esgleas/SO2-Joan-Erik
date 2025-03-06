#include <asm.h>
#include <errno.h>

ENTRY(write)
	pushl %ebp
	mov %esp,%ebp
	
	//Se guardan estos registros porque hacemos fast syscall
	pushl %edx
	pushl %ecx
	pushl %ebx
	
	//Se guardan los valores en el orden que se guardan en el SAVE_ALL
	mov 0x08(%ebp), %edx  //fd -> edx
	mov 0x0c(%ebp), %ecx  //buffer -> ecx
	mov 0x10(%ebp), %ebx //size -> ebx
	
	//Ponemos el numero de la syscall en el eax
	movl $4, %eax
	
	pushl $writeRet
	pushl %ebp
	mov %esp,%ebp
	
	sysenter
	
writeRet:
	
	popl %ebp
	addl $4, %esp
	popl %ebx
	popl %ecx
	popl %edx
	cmpl $0, %eax
	jge writeSinError
	negl %eax
	movl %eax, errno
	movl -1, %eax
	
writeSinError:

	popl %ebp
	ret

ENTRY(gettime)
  pushl %ebp
  mov %esp,%ebp

  movl $0x0A, %eax 
  int $0x80

  pop %ebp
  ret
