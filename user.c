#include <libc.h>

char buff[24];

int pid;

int add(int par1, int par2) { return par1 + par2; }
int addAsm(int, int);

void print(char *s) { write(1, s, strlen(s)); }

void test_write() {
  print("\n==== Test de write ====\n");

  const char *msg = "Probando write\n";
  if (write(1, (char *)msg, strlen(msg)) < 0) {
    perror();
  }
}

void test_set_color() {
  print("\n==== Test de set color ====\n");
  if (set_color(4, 7) < 0) {
    perror();
  } else {
    print("Texto en color modificado\n");
  }
}

void test_gotoxy() {
  print("\n==== Test de gotoxy ====\n");
  if (gotoxy(10, 5) < 0) {
    perror();
  } else {
    print("Cursor movido a (10,5)\n");
  }
}

void test_fork() {
  print("\n==== Test de fork ====\n");
  int pid = fork();
  if (pid < 0) {
    perror();
  } else if (pid == 0) {
    char *men = "Soy el hijo del fork\n";
    print(men);
    exit();
  } else {
    char *men = "Soy el padre del fork\n";
    print(men);
  }
}

void test_block_unblock() {
  print("\n==== Test de block ====\n");
  int pid = fork();
  if (pid < 0) {
    perror();
  } else if (pid == 0) {
    // Hijo
    print("Hijo: Me bloqueo a mi mismo\n");
    block();
    print("Hijo: He sido desbloqueado\n");
    exit();
  } else {
    // Padre
    int i = 0;
    while (i <= 10000) {
      int temp = 1;
      i += temp;
    }
    int result = unblock(pid);
    if (result < 0) {
      perror();
    } else {
      print("Padre: Hijo desbloqueado\n");
    }
  }
}

void test_combinado1() {
  print("\n==== Test de comb 1 ====\n");
  int pid = fork();
  if (pid < 0) {
    perror();
    return;
  }

  if (pid == 0) {
    set_color(2, 0);
    gotoxy(20, 10);
    print("Hijo: Texto combinado\n");
    block();
    print("Hijo: Reanudado tras desbloqueo\n");
    exit();
  } else {
    int i = 0;
    while (i <= 10000) {
      int temp = 1;
      i += temp;
    }
    print("Padre: Desbloqueando al hijo...\n");
    if (unblock(pid) < 0) {
      perror();
    }
  }
}

void test_read() {
  print("\n==== Test de read ====\n");
  char buffer[5];
  print("Introduce 5 caracteres: ");
  int n = read(buffer, 5);
  if (n < 0) {
    perror();
  } else {
    print("\nHas introducido: ");
    write(1, buffer, 5);
    write(1, "\n", 1);
  }
}

/*
void test_combinado2() {
  print("\n==== Test de comb 2 ====\n");
  int pid = fork();
  if (pid < 0) {
    perror();
    return;
  }

  if (pid == 0) {
    print("\nHijo: Me bloqueo ahora...\n");
    block();
    set_color(3, 0);
    gotoxy(5, 15);
    print("Hijo: Introduce 4 caracteres: ");
    char buf[10];
    int n = read(buf, 4);
    if (n < 0) {
      perror();
    } else {
      buf[n] = '\0';
      gotoxy(5, 17);
      print("Hijo leyo: ");
      write(1, buf, 4);
      print("\nHijo: Desbloqueado y finalizado\n");
    }
    exit();
  } else {
    write(1, "Padre: Introduce 4 caracteres: ", 30);
    char buf[10];
    int n = read(buf, 4);
    if (n < 0) {
      perror();
    } else {
      print("Padre leyo: ");
      write(1, buf, 4);
      write(1, " \nPadre: Desbloqueando al hijo...\n", 33);
      if (unblock(pid) < 0) {
        perror();
      }
    }
  }
}
*/

void test_combinado2() {
  print("\n==== Test de combinado 2 ====\n");
  int pid = fork();
  if (pid < 0) {
    perror();
  } else if (pid == 0) {
    // Hijo
    print("Hijo: Me bloqueo a mi mismo\n");
    block();
    print("Hijo: He sido desbloqueado\n");
    exit();
  } else {
    // Padre
    char buffer[5];
    print("Introduce 5 caracteres: ");
    int n = read(buffer, 5);
    if (n < 0) {
      perror();
    } else {
      print("\nHas introducido: ");
      write(1, buffer, 5);
      write(1, "\n", 1);
    }

    int result = unblock(pid);
    if (result < 0) {
      perror();
    } else {
      print("Padre: Hijo desbloqueado\n");
    }
  }
}

int __attribute__((__section__(".text.main"))) main(void) {
  /* Next line, tries to move value 0 to CR3 register. This register is a
   * privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  // test_write();
  // test_set_color();
  // test_gotoxy();
  // test_fork();
  // test_block_unblock();
  // test_read();
  // test_combinado1();
   test_combinado2();

  while (1) {
  }
}
