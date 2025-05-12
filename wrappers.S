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

ENTRY(getpid)
	pushl %ebp
	mov %esp,%ebp
	
	//Ponemos el numero de la syscall en el eax
	movl $20, %eax
	
	pushl $getpidRet
	pushl %ebp
	mov %esp,%ebp
	
	sysenter

getpidRet:
	
	popl %ebp
	addl $4, %esp
	cmpl $0, %eax
	jge getpidSinError
	negl %eax
	movl %eax, errno
	movl -1, %eax
	
getpidSinError:

	popl %ebp
	ret
	
ENTRY(fork)
	pushl %ebp
	mov %esp,%ebp
	
	//Ponemos el numero de la syscall en el eax
	movl $2, %eax
	
	pushl $forkRet
	pushl %ebp
	mov %esp,%ebp
	
	sysenter

forkRet:
	
	popl %ebp
	addl $4, %esp
	cmpl $0, %eax
	jge forkSinError
	negl %eax
	movl %eax, errno
	movl -1, %eax
	
forkSinError:

	popl %ebp
	ret
	
ENTRY(exit)
	pushl %ebp
	mov %esp,%ebp
	
	//Ponemos el numero de la syscall en el eax
	movl $1, %eax
	
	pushl $exitRet
	pushl %ebp
	mov %esp,%ebp
	
	sysenter

exitRet:
	
	popl %ebp
	addl $4, %esp
	cmpl $0, %eax
	jge exitSinError
	negl %eax
	movl %eax, errno
	movl -1, %eax
	
exitSinError:

	popl %ebp
	ret	

ENTRY(block)
	pushl %ebp
	mov %esp,%ebp
	
	//Ponemos el numero de la syscall en el eax
	movl $5, %eax
	
	pushl $blockRet
	pushl %ebp
	mov %esp,%ebp
	
	sysenter

blockRet:
	
	popl %ebp
	addl $4, %esp
	cmpl $0, %eax
	jge blockSinError
	negl %eax
	movl %eax, errno
	movl -1, %eax
	
blockSinError:

	popl %ebp
	ret
	
ENTRY(unblock)
	pushl %ebp
	mov %esp,%ebp
	
	pushl %edx
	
	mov 0x08(%ebp), %edx //pid -> edx
	
	//Ponemos el numero de la syscall en el eax
	movl $6, %eax
	
	pushl $unblockRet
	pushl %ebp
	mov %esp,%ebp
	
	sysenter

unblockRet:
	
	popl %ebp
	addl $4, %esp
	popl %edx
	cmpl $0, %eax
	jge unblockSinError
	negl %eax
	movl %eax, errno
	movl -1, %eax
	
unblockSinError:

	popl %ebp
	ret

ENTRY(gotoxy)
	pushl %ebp
	mov %esp,%ebp
	
	pushl %edx
	pushl %ecx
	
	//Se guardan los valores en el orden que se guardan en el SAVE_ALL
	mov 0x08(%ebp), %edx  //x -> edx
	mov 0x0c(%ebp), %ecx  //y -> ecx
	
	//Ponemos el numero de la syscall en el eax
	movl $8, %eax
	
	pushl $gotoxyRet
	pushl %ebp
	mov %esp,%ebp
	
	sysenter
	
gotoxyRet:
	
	popl %ebp
	addl $4, %esp
	popl %ecx
	popl %edx
	cmpl $0, %eax
	jge gotoxySinError
	negl %eax
	movl %eax, errno
	movl -1, %eax
	
gotoxySinError:

	popl %ebp
	ret
	
ENTRY(set_color)
	pushl %ebp
	mov %esp,%ebp
	
	pushl %edx
	pushl %ecx
	
	//Se guardan los valores en el orden que se guardan en el SAVE_ALL
	mov 0x08(%ebp), %edx  //x -> edx
	mov 0x0c(%ebp), %ecx  //y -> ecx
	
	//Ponemos el numero de la syscall en el eax
	movl $9, %eax
	
	pushl $setColorRet
	pushl %ebp
	mov %esp,%ebp
	
	sysenter
	
setColorRet:
	
	popl %ebp
	addl $4, %esp
	popl %ecx
	popl %edx
	cmpl $0, %eax
	jge setColorSinError
	negl %eax
	movl %eax, errno
	movl -1, %eax
	
setColorSinError:

	popl %ebp
	ret
