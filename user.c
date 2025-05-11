#include <libc.h>

char buff[24];

int pid;

int add(int par1, int par2) { return par1 + par2; }
int addAsm(int, int);

int __attribute__((__section__(".text.main"))) main(void) {
  /* Next line, tries to move value 0 to CR3 register. This register is a
   * privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  int ret = fork();

  if (ret == 0) {
    char *mensaje = "\nSoy el hijo, me bloqueare y mi padre me desbloqueara\n";
    if (write(1, mensaje, strlen(mensaje)) < 0)
      perror();

    block();

    mensaje = "\nYa me han desbloqueado\n";
    if (write(1, mensaje, strlen(mensaje)) < 0)
      perror();
    char b[5];
    read(b, 5);
    if (write(1, b, 5) < 0)
      perror();
  } else {
    char b[5];
    read(b, 5);
    if (write(1, b, 5) < 0)
      perror();
    unblock(ret);
  }
  while (1) {
  }
}
