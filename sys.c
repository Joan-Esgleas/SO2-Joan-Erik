/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}

char bufferAux[];

int sys_write(int fd, char * buffer, int size) {
	int checkFd = check_fd(fd, ESCRIPTURA);
	if(checkFd) return checkFd;
	else if(buffer == NULL) return EFAULT;
	else if (size <= 0) return EINVAL;
	else {
		copy_from_user(buffer, bufferAux, size);
		int ret = sys_write_console(bufferAux, size);
		return ret;
	}
}
