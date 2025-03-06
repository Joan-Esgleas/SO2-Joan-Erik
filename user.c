#include <libc.h>

char buff[24];

int pid;

int add(int par1, int par2) {
  return par1 + par2;
}
int addAsm(int, int);

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
	
	char * mensaje = "\nPrueba del write en el user.c\n";
	if(write(1, mensaje, 31) < 0) perror();
	
	char* p = 0;
	*p = 'x';

  while(1) {
    int temp2 = add(0x42,0x666);
    int temp1 = addAsm(0x42,0x666);
    int time = gettime();
    char m[12];
    itoa(time,m);
	if(write(1, m, 12) < 0) perror();

  }
}
