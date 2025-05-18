#include "include/libc.h"
#include <libc.h>

char buff[24];

int pid;

int add(int par1, int par2) { return par1 + par2; }
int addAsm(int, int);

void print(char *s) { write(1, s, strlen(s)); }

void test_write() {
  print("\n==== Test de write ====\n");

  char *msg = "Probando write\n";
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
    print(
        "\nXXXXXXXXXXXXXXXXXXXXX Esto no se deveria ver XXXXXXXXXXXXXXXXXX\n");
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
    print(
        "\nXXXXXXXXXXXXXXXXXXXXX Esto no se deveria ver XXXXXXXXXXXXXXXXXX\n");
  } else {
    // Padre
    print("Padre: Desbloqueando al Hijo\n");
    int result = unblock(pid);
    if (result < 0) {
      perror();
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
    print("\n Esto son (bytes): ");
    char size[9];
    itoa(n, size);
    print(size);
    print("\0");
    print("\n");
  }
}

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
    }
    exit();
  } else {
    print("\nPadre: Introduce 4 caracteres: \n");
    char buf[10];
    int n = read(buf, 4);
    if (n < 0) {
      perror();
    } else {
      print("Padre leyo: ");
      write(1, buf, 4);
      print(" \nPadre: Desbloqueando al hijo...\n");
      if (unblock(pid) < 0) {
        perror();
      }
    }
  }
}
void test_read2() {
  print("\n==== Test de read 2 ====\n");
  int pid = fork();
  if (pid == 0) {
    char buf[10];
    int n = read(buf, 4);
    if (n < 0) {
      perror();
    } else {
      print("Hijo leyo: ");
      write(1, buf, 4);
    }
    exit();
  } else {
    char buf[10];
    int n = read(buf, 4);
    if (n < 0) {
      perror();
    } else {
      print("Padre leyo: ");
      write(1, buf, 4);
    }
  }
}

void test_fork2() {
  print("\n==== Test de  fork2 ====\n");
  print("Soy padre\n");
  int p1 = fork();
  if (p1 == 0) {
    print("Soy hijo\n");
    int p2 = fork();
    if (p2 == 0) {
      print("\nSoy nieto\n");
    } else {
      print("Hijo:He hecho el fork\n");
    }
  } else {
    print("Padre:He hecho el fork\n");
  }
}

void test_read3() {
  print("\n==== Test de read 3 ====\n");
  int pid = fork();
  if (pid == 0) {
    block();
    int pid2 = fork();
    if (pid2 != 0) {
      char buf[10];
      int n = read(buf, 4);
      if (n < 0) {
        perror();
      } else {
        print("Hijo leyo: ");
        write(1, buf, 4);
      }
      exit();
    }

  } else {
    char buf[10];
    int n = read(buf, 4);
    if (n < 0) {
      perror();
    } else {
      print("Padre leyo: ");
      write(1, buf, 4);
    }
    unblock(pid);
  }
}

int stack[1024];

void test_thread_func(char *c) {
  print("Thread se activa\n");
  print(c);
  print("Pulsa 4 teclas para desbloquar el read: \n");
  char buf[10];
  int n = read(buf, 4);
  if (n < 0) {
    perror();
  } else {
    print("Thread leyo: ");
    write(1, buf, 4);
    print("\n");
  }

  exit_thread();
}

void test_thread() {
  print("\n==== Test de thread ====\n");
  char *t_c = "Recivo Argumento\n";
  int tid = create_thread((void *)test_thread_func, &stack[1024], t_c);
  wait_thread(tid);
  print("\nSe ha desbloqueado el padre\n");
}

int semid;

void test_sem_func() {
  print("Thread Hijo: me bloqueo con semaforo\n");
  semWait(semid);
  print("Thread Hijo: me he desbloqueado con semaforo\n");
  exit_thread();
}

void test_sem() {
  print("\n==== Test de semaforo ====\n");
  semid = semCreate(0);
  int tid = create_thread((void *)test_sem_func, &stack[1024], 0);
  print("Thread Padre: pulsa una tecla para desbloquear al hijo\n");
  char buf[1];
  read(buf, 1);
  semSignal(semid);
  wait_thread(tid);
  print("Thread padre: Mi hijo se ha desbloqueado y ha terminado.\n");
  semDestroy(semid);
  print("Semaforo borrado\n");
}

void test_sem2() {
  print("\n==== Test de semaforo 2====\n");
  semid = semCreate(0);
  int tid = create_thread((void *)test_sem_func, &stack[1024], 0);
  print("Thread Padre: pulsa una tecla para desbloquear al hijo\n");
  char buf[1];
  read(buf, 1);
  semDestroy(semid);
  wait_thread(tid);
  print("Thread padre: Mi hijo se ha desbloqueado al borrar el semaforo.\n");
}


int __attribute__((__section__(".text.main"))) main(void) {
  /* Next line, tries to move value 0 to CR3 register. This register is a
   * privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  test_write();
  test_set_color();
  test_gotoxy();
  test_fork();
  test_block_unblock();
  test_read();
  test_combinado1();
  test_combinado2();
  test_thread();
  test_sem();
  test_sem2();

  // test_read2();
  // test_read3();
  // test_fork2();

  while (1) {
  }
}
