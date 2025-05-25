#include "include/libc.h"
#include <libc.h>

#define ROWS 25
#define COLS 80
#define sizeYDragon 8
#define sizeXDragon 10
#define sizeYEnemigo 7
#define sizeXEnemigo 7
#define sizeYDiamante 4
#define sizeXDiamante 5
#define sizeXWinner 40
#define sizeYWinner 5
#define sizeXLoser 43
#define sizeYLoser 5
#define NEGRO 0
#define AMARILLO 1;
#define VERDE 2
#define CIAN 3
#define ROJO 4
#define ROSA 5
#define MARRON 6
#define BLANCO 7
#define MAX_PROYECTILES 10

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

void test_dyn_mem() {
  print("\n==== Test de dyn_mem ====\n");
  char *mem1 = dyn_mem(4);
  char size[9];
  itoa((unsigned long)mem1, size);
  print(size);
  print("\0");
  print("\n");

  mem1 = "Este mensaje esta en heap \n \0";

  print(mem1);

  char *mem2 = dyn_mem(4);
  itoa((unsigned long)mem2, size);
  print(size);
  print("\0");
  print("\n");

  char *mem3 = dyn_mem(-4);
  itoa((unsigned long)mem3, size);
  print(size);
  print("\0");
  print("\n");

  int *p = (int *)dyn_mem(4);
  for (int i = 0; i < 4096; i++)
    p[i] = i;

  dyn_mem(-4);

  int pid = fork();
  if (pid == 0) {
    print(mem1);
    for (int i = 0; i < 4 * 0x1000; i++)
      mem1[i] = (char)i;

    exit();
  }
  print(mem1);
  dyn_mem(-4);
}

char *mem1;
int semid;
int my_pid;

void test_thread_dyn_mem(char *c) {

  print("\n Hijo bloquea \n");
  semSignal(semid);
  block();
  print("\n Thread hijo: tiene mem\n");
  print(mem1);

  char size[9];
  itoa((unsigned long)dyn_mem(0), size);
  print(size);
  print("\0");
  print("\n");

  semSignal(semid);
  print("bloqueamos\n");
  block();

  unsigned long ph = (unsigned long)dyn_mem(0);
  itoa(ph, size);
  print(size);
  print("\0");
  print("\n");

  exit_thread();
}

void test_dyn_mem2() {
  print("\n==== Test de dyn_mem 2 ====\n");
  semid = semCreate(0);
  my_pid = getpid();
  int tid = create_thread((void *)test_thread_dyn_mem, &stack[1024], 0);
  print("Hemos cread hijo \n");
  semWait(semid);
  mem1 = dyn_mem(4);
  char size[9];
  itoa((unsigned long)mem1, size);
  print(size);
  print("\0");
  print("\n");
  itoa((unsigned long)dyn_mem(0), size);
  print(size);
  print("\0");
  print("\n");

  for (int i = 0; i < (4 * 0x1000); i++)
    mem1[i] = i;

  mem1 = "Este mensaje esta en heap \n \0";
  print(mem1);
  print("\n unblock hijo \n");
  unblock(tid);
  semWait(semid);
  print("\n Liberamos: \n");
  char *ph = dyn_mem(-4);
  itoa((unsigned long)ph, size);
  print(size);
  print("\0");
  print("\n");

  print("\n unblock hijo 2 \n");
  unblock(tid);
}

void test_sem_func() {
  print("Thread Hijo: me bloqueo con semaforo\n");
  int return_value = semWait(semid);
  char size[10];
  itoa(return_value, size);
  print(size);
  print("\n");
  print("Thread Hijo: me he desbloqueado con semaforo\n");
  exit_thread();
}

void test_sem_func2() {
  print("Thread Hijo: me bloqueo con semaforo\n");
  int return_value = semWait(semid);
  if (return_value < 0)
    print("-");
  char size[10];
  itoa(-return_value, size);
  print(size);
  print("\n");
  print("Thread Hijo: me he desbloqueado con semaforo Borrado\n");
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
  int tid = create_thread((void *)test_sem_func2, &stack[1024], 0);
  print("Thread Padre: pulsa una tecla para desbloquear al hijo\n");
  char buf[1];
  read(buf, 1);
  semDestroy(semid);
  wait_thread(tid);
  print("Thread padre: Mi hijo se ha desbloqueado al borrar el semaforo.\n");
}

void test_malloc_free() {
  print("\n==== Test malloc free ====\n");
  char size[9];
  itoa((unsigned long)dyn_mem(0), size);
  print(size);
  print("\0");
  print("\n");

  char *p = malloc((PAGE_SIZE + PAGE_SIZE - MALLOC_HEADER_SIZE) * sizeof(char));

  itoa((unsigned long)dyn_mem(0), size);
  print(size);
  print("\0");
  print("\n");

  for (int i = 0; i < PAGE_SIZE + PAGE_SIZE - MALLOC_HEADER_SIZE; i++)
    p[i] = 'A';

  free(p);

  itoa((unsigned long)dyn_mem(0), size);
  print(size);
  print("\0");
  print("\n");
}

enum characterState { ST_UP, ST_DOWN, ST_LEFT, ST_RIGHT, ST_STOP };

enum characterDirection { LEFT, RIGHT };

typedef struct {
	int x;
	int y;
	enum characterState state;
	enum characterDirection dir;
} character;

typedef struct {
	char ch;
	int fg;
	int bg;
} pixel;

typedef struct {
	int x;
	int y;
	int activo;
	enum characterDirection dir;
} proyectil;

character *player;
character *enemigo1;
character *enemigo2;

char stackKeyboard[100];
char stackGame[1024];

int semChanges;

char **mapa;
pixel **dragon;
pixel **enemigo;
pixel **diamante;
pixel **winner;
pixel **loser;

proyectil *proyectiles;

int enemigo1muerto = 0;
int enemigo2muerto = 0;

void moverProyectiles() {
  char buf[1];
  buf[0] = ' ';
  
  for (int i = 0; i < MAX_PROYECTILES; ++i) {
      if (proyectiles[i].activo) {
        gotoxy(proyectiles[i].x, proyectiles[i].y);
        if(mapa[proyectiles[i].y][proyectiles[i].x] == '1') set_color(BLANCO, NEGRO);
	else set_color(BLANCO, MARRON);  	
	write(1, buf, 1);

        if (proyectiles[i].dir == LEFT) --proyectiles[i].x;
        else ++proyectiles[i].x;
          
        if (mapa[proyectiles[i].y][proyectiles[i].x] == '0') {
              proyectiles[i].activo = 0;
        }
        else {
              gotoxy(proyectiles[i].x, proyectiles[i].y);
              set_color(BLANCO, ROSA);
              write(1, buf, 1);
        }
      	
      	int x = proyectiles[i].x;
      	int y = proyectiles[i].y;
      	
	if((x >= enemigo1->x) && (x < enemigo1->x + sizeXEnemigo) && (y >= enemigo1->y) && (y < enemigo1->y + sizeYEnemigo) && !enemigo1muerto) {
		enemigo1muerto = 1;
		proyectiles[i].activo = 0;
		pintaTrozoMapa(enemigo1->x, enemigo1->y, sizeXEnemigo, sizeYEnemigo);
	}
	if((x >= enemigo2->x) && (x < enemigo2->x + sizeXEnemigo) && (y >= enemigo2->y) && (y < enemigo2->y + sizeYEnemigo) && !enemigo2muerto) {
		enemigo2muerto = 1;
		proyectiles[i].activo = 0;
		pintaTrozoMapa(enemigo2->x, enemigo2->y, sizeXEnemigo, sizeYEnemigo);
	}
      }
  }
}

void creaProyectil() {
  int trobat = 0;
  int i = 0;
  
  while(!trobat && i < MAX_PROYECTILES) {
  	if(!proyectiles[i].activo) {
  		if(player->dir == LEFT) proyectiles[i].x = player->x - 1;
  		else proyectiles[i].x = player->x + sizeXDragon;
  		
  		proyectiles[i].y = player->y + 4;
  		proyectiles[i].dir = player->dir;
  		proyectiles[i].activo = 1;
  		trobat = 1;	
  	}
  	i++;
  }
}

void keyboardThread() {
  char buffer[1];
  while(1) {
  	if(read(buffer, 1) > 0) {
  		switch(buffer[0]) {
  			case 'w': player->state = ST_UP; break;
  			case 's': player->state = ST_DOWN; break;
  			case 'a': player->state = ST_LEFT; player->dir = LEFT; break;
  			case 'd': player->state = ST_RIGHT; player->dir = RIGHT; break;
  			case 'C': creaProyectil(); break;
  			default: player->state = ST_STOP; break;
  		}
  		semSignal(semChanges);
  	}
  }
	
}

void pintaTrozoMapa(int x, int y, int sizeX, int sizeY) {
  char buffer[1];
  buffer[0] = ' ';
  
  for(int i = y; (i < y + sizeY) && (i < ROWS); ++i) {
  	for(int j = x; (j < x + sizeX) && (j < COLS); ++j) {
  		gotoxy(j, i);
  		if(mapa[i][j] == '1') set_color(BLANCO, NEGRO);
		else set_color(BLANCO, MARRON);  	
		write(1, buffer, 1);
  	}
  }
}

void pintaMapa(char ** map) {
  char buffer[1];
  buffer[0] = ' ';
  
  for(int i = 0; i < ROWS; ++i) {
  	for(int j = 0; j < COLS; ++j) {
  		gotoxy(j, i);
  		if(map[i][j] == '1') set_color(BLANCO, NEGRO);
		else set_color(BLANCO, MARRON);  	
		write(1, buffer, 1);
  	}
  } 
}

void creaMapa(char ** map) {
  for (int i = 0; i < ROWS; ++i) {
      for (int j = 0; j < COLS; ++j) {
          map[i][j] = '1';
      }
  }

  //Bordes
  for (int i = 0; i < ROWS; ++i) {
      for (int j = 0; j < COLS; ++j) {
          if (i < 1 || i >= ROWS - 1 || j < 2 || j >= COLS - 2) {
              map[i][j] = '0';
          }
      }
  }

  for(int i = 0; i < 24; ++i) {
  	//Escalon de abajo 
  	map[ROWS - 2][i + 1] = '0';
  	map[ROWS - 2][COLS - 2 - i] = '0';
  	if(i < 12) {
  		//Escalon de arriba
  		map[ROWS - 3][i + 1] = '0';
  		map[ROWS - 3][COLS - 2 - i] = '0';
  	}
  }
}

void pintaPixel(int x0, int y0, int sizeX, int sizeY, pixel **pix) {
    char buf[1];
    for (int i = 0; i < sizeY; ++i) {
        for (int j = 0; j < sizeX; ++j) {
            gotoxy(x0 + j, y0 + i);
            set_color(pix[i][j].fg, pix[i][j].bg);
            buf[0] = pix[i][j].ch;
            write(1, buf, 1);
        }
    }
}

void pintaDragonIzquierda(int x0, int y0) {
    char buf[1];
    for (int i = 0; i < sizeYDragon; ++i) {
        for (int j = 0; j < sizeXDragon; ++j) {
            int espejoJ = sizeXDragon - 1 - j;
            gotoxy(x0 + j, y0 + i);
            set_color(dragon[i][espejoJ].fg, dragon[i][espejoJ].bg);
            buf[0] = dragon[i][espejoJ].ch;
            write(1, buf, 1);
        }
    }
}

void creaLoser() {
    for (int i = 0; i < sizeYLoser; ++i) {
        for (int j = 0; j < sizeXLoser; ++j) {
            loser[i][j].ch = ' ';
            loser[i][j].fg = NEGRO;
            loser[i][j].bg = NEGRO;
        }
    }
    
    loser[0][0] = (pixel){' ', NEGRO, ROJO};
    loser[0][4] = (pixel){' ', NEGRO, ROJO};
    loser[0][7] = (pixel){' ', NEGRO, ROJO};
    loser[0][8] = (pixel){' ', NEGRO, ROJO};
    loser[0][9] = (pixel){' ', NEGRO, ROJO};
    loser[0][12] = (pixel){' ', NEGRO, ROJO};
    loser[0][16] = (pixel){' ', NEGRO, ROJO};
    loser[0][20] = (pixel){' ', NEGRO, ROJO};
    loser[0][27] = (pixel){' ', NEGRO, ROJO};
    loser[0][28] = (pixel){' ', NEGRO, ROJO};
    loser[0][29] = (pixel){' ', NEGRO, ROJO};
    loser[0][33] = (pixel){' ', NEGRO, ROJO};
    loser[0][34] = (pixel){' ', NEGRO, ROJO};
    loser[0][35] = (pixel){' ', NEGRO, ROJO};
    loser[0][36] = (pixel){' ', NEGRO, ROJO};
    loser[0][38] = (pixel){' ', NEGRO, ROJO};
    loser[0][39] = (pixel){' ', NEGRO, ROJO};
    loser[0][40] = (pixel){' ', NEGRO, ROJO};
    loser[0][41] = (pixel){' ', NEGRO, ROJO};
    loser[0][42] = (pixel){' ', NEGRO, ROJO};
    
    loser[1][1] = (pixel){' ', NEGRO, ROJO};
    loser[1][3] = (pixel){' ', NEGRO, ROJO};
    loser[1][6] = (pixel){' ', NEGRO, ROJO};
    loser[1][10] = (pixel){' ', NEGRO, ROJO};
    loser[1][12] = (pixel){' ', NEGRO, ROJO};
    loser[1][16] = (pixel){' ', NEGRO, ROJO};
    loser[1][20] = (pixel){' ', NEGRO, ROJO};
    loser[1][26] = (pixel){' ', NEGRO, ROJO};
    loser[1][32] = (pixel){' ', NEGRO, ROJO};
    loser[1][38] = (pixel){' ', NEGRO, ROJO};
    
    loser[2][2] = (pixel){' ', NEGRO, ROJO};
    loser[2][6] = (pixel){' ', NEGRO, ROJO};
    loser[2][10] = (pixel){' ', NEGRO, ROJO};
    loser[2][12] = (pixel){' ', NEGRO, ROJO};
    loser[2][16] = (pixel){' ', NEGRO, ROJO};
    loser[2][20] = (pixel){' ', NEGRO, ROJO};
    loser[2][26] = (pixel){' ', NEGRO, ROJO};
    loser[2][30] = (pixel){' ', NEGRO, ROJO};
    loser[2][33] = (pixel){' ', NEGRO, ROJO};
    loser[2][34] = (pixel){' ', NEGRO, ROJO};
    loser[2][35] = (pixel){' ', NEGRO, ROJO};
    loser[2][38] = (pixel){' ', NEGRO, ROJO};
    loser[2][39] = (pixel){' ', NEGRO, ROJO};
    loser[2][40] = (pixel){' ', NEGRO, ROJO};
    
    loser[3][2] = (pixel){' ', NEGRO, ROJO};
    loser[3][6] = (pixel){' ', NEGRO, ROJO};
    loser[3][10] = (pixel){' ', NEGRO, ROJO};
    loser[3][12] = (pixel){' ', NEGRO, ROJO};
    loser[3][16] = (pixel){' ', NEGRO, ROJO};
    loser[3][20] = (pixel){' ', NEGRO, ROJO};
    loser[3][26] = (pixel){' ', NEGRO, ROJO};
    loser[3][30] = (pixel){' ', NEGRO, ROJO};
    loser[3][36] = (pixel){' ', NEGRO, ROJO};
    loser[3][38] = (pixel){' ', NEGRO, ROJO};
    
    loser[4][2] = (pixel){' ', NEGRO, ROJO};
    loser[4][7] = (pixel){' ', NEGRO, ROJO};
    loser[4][8] = (pixel){' ', NEGRO, ROJO};
    loser[4][9] = (pixel){' ', NEGRO, ROJO};
    loser[4][13] = (pixel){' ', NEGRO, ROJO};
    loser[4][14] = (pixel){' ', NEGRO, ROJO};
    loser[4][15] = (pixel){' ', NEGRO, ROJO};
    loser[4][20] = (pixel){' ', NEGRO, ROJO};
    loser[4][21] = (pixel){' ', NEGRO, ROJO};
    loser[4][22] = (pixel){' ', NEGRO, ROJO};
    loser[4][23] = (pixel){' ', NEGRO, ROJO};
    loser[4][24] = (pixel){' ', NEGRO, ROJO};
    loser[4][27] = (pixel){' ', NEGRO, ROJO};
    loser[4][28] = (pixel){' ', NEGRO, ROJO};
    loser[4][29] = (pixel){' ', NEGRO, ROJO};
    loser[4][32] = (pixel){' ', NEGRO, ROJO};
    loser[4][33] = (pixel){' ', NEGRO, ROJO};
    loser[4][34] = (pixel){' ', NEGRO, ROJO};
    loser[4][35] = (pixel){' ', NEGRO, ROJO};
    loser[4][38] = (pixel){' ', NEGRO, ROJO};
    loser[4][39] = (pixel){' ', NEGRO, ROJO};
    loser[4][40] = (pixel){' ', NEGRO, ROJO};
    loser[4][41] = (pixel){' ', NEGRO, ROJO};
    loser[4][42] = (pixel){' ', NEGRO, ROJO};
}

void creaWinner() {
    for (int i = 0; i < sizeYWinner; ++i) {
        for (int j = 0; j < sizeXWinner; ++j) {
            winner[i][j].ch = ' ';
            winner[i][j].fg = NEGRO;
            winner[i][j].bg = NEGRO;
        }
    }
    
    winner[0][0] = (pixel){' ', NEGRO, BLANCO};
    winner[0][4] = (pixel){' ', NEGRO, BLANCO};
    winner[0][6] = (pixel){' ', NEGRO, BLANCO};
    winner[0][7] = (pixel){' ', NEGRO, BLANCO};
    winner[0][8] = (pixel){' ', NEGRO, BLANCO};
    winner[0][9] = (pixel){' ', NEGRO, BLANCO};
    winner[0][10] = (pixel){' ', NEGRO, BLANCO};
    winner[0][12] = (pixel){' ', NEGRO, BLANCO};
    winner[0][16] = (pixel){' ', NEGRO, BLANCO};
    winner[0][18] = (pixel){' ', NEGRO, BLANCO};
    winner[0][22] = (pixel){' ', NEGRO, BLANCO};
    winner[0][24] = (pixel){' ', NEGRO, BLANCO};
    winner[0][25] = (pixel){' ', NEGRO, BLANCO};
    winner[0][26] = (pixel){' ', NEGRO, BLANCO};
    winner[0][27] = (pixel){' ', NEGRO, BLANCO};
    winner[0][28] = (pixel){' ', NEGRO, BLANCO};
    winner[0][30] = (pixel){' ', NEGRO, BLANCO};
    winner[0][31] = (pixel){' ', NEGRO, BLANCO};
    winner[0][32] = (pixel){' ', NEGRO, BLANCO};
    winner[0][33] = (pixel){' ', NEGRO, BLANCO};
    winner[0][37] = (pixel){' ', NEGRO, BLANCO};
    winner[0][38] = (pixel){' ', NEGRO, BLANCO};
    winner[0][39] = (pixel){' ', NEGRO, BLANCO};
    
    winner[1][0] = (pixel){' ', NEGRO, BLANCO};
    winner[1][4] = (pixel){' ', NEGRO, BLANCO};
    winner[1][8] = (pixel){' ', NEGRO, BLANCO};
    winner[1][12] = (pixel){' ', NEGRO, BLANCO};
    winner[1][13] = (pixel){' ', NEGRO, BLANCO};
    winner[1][16] = (pixel){' ', NEGRO, BLANCO};
    winner[1][18] = (pixel){' ', NEGRO, BLANCO};
    winner[1][19] = (pixel){' ', NEGRO, BLANCO};
    winner[1][22] = (pixel){' ', NEGRO, BLANCO};
    winner[1][24] = (pixel){' ', NEGRO, BLANCO};
    winner[1][30] = (pixel){' ', NEGRO, BLANCO};
    winner[1][34] = (pixel){' ', NEGRO, BLANCO};
    winner[1][37] = (pixel){' ', NEGRO, BLANCO};
    winner[1][38] = (pixel){' ', NEGRO, BLANCO};
    winner[1][39] = (pixel){' ', NEGRO, BLANCO};
    
    winner[2][0] = (pixel){' ', NEGRO, BLANCO};
    winner[2][2] = (pixel){' ', NEGRO, BLANCO};
    winner[2][4] = (pixel){' ', NEGRO, BLANCO};
    winner[2][8] = (pixel){' ', NEGRO, BLANCO};
    winner[2][12] = (pixel){' ', NEGRO, BLANCO};
    winner[2][14] = (pixel){' ', NEGRO, BLANCO};
    winner[2][16] = (pixel){' ', NEGRO, BLANCO};
    winner[2][18] = (pixel){' ', NEGRO, BLANCO};
    winner[2][20] = (pixel){' ', NEGRO, BLANCO};
    winner[2][22] = (pixel){' ', NEGRO, BLANCO};
    winner[2][24] = (pixel){' ', NEGRO, BLANCO};
    winner[2][25] = (pixel){' ', NEGRO, BLANCO};
    winner[2][26] = (pixel){' ', NEGRO, BLANCO};
    winner[2][30] = (pixel){' ', NEGRO, BLANCO};
    winner[2][31] = (pixel){' ', NEGRO, BLANCO};
    winner[2][32] = (pixel){' ', NEGRO, BLANCO};
    winner[2][33] = (pixel){' ', NEGRO, BLANCO};
    winner[2][37] = (pixel){' ', NEGRO, BLANCO};
    winner[2][38] = (pixel){' ', NEGRO, BLANCO};
    winner[2][39] = (pixel){' ', NEGRO, BLANCO};
    
    winner[3][0] = (pixel){' ', NEGRO, BLANCO};
    winner[3][1] = (pixel){' ', NEGRO, BLANCO};
    winner[3][2] = (pixel){' ', NEGRO, BLANCO};
    winner[3][3] = (pixel){' ', NEGRO, BLANCO};
    winner[3][4] = (pixel){' ', NEGRO, BLANCO};
    winner[3][8] = (pixel){' ', NEGRO, BLANCO};
    winner[3][12] = (pixel){' ', NEGRO, BLANCO};
    winner[3][15] = (pixel){' ', NEGRO, BLANCO};
    winner[3][16] = (pixel){' ', NEGRO, BLANCO};
    winner[3][18] = (pixel){' ', NEGRO, BLANCO};
    winner[3][21] = (pixel){' ', NEGRO, BLANCO};
    winner[3][22] = (pixel){' ', NEGRO, BLANCO};
    winner[3][24] = (pixel){' ', NEGRO, BLANCO};
    winner[3][30] = (pixel){' ', NEGRO, BLANCO};
    winner[3][32] = (pixel){' ', NEGRO, BLANCO};
    
    winner[4][0] = (pixel){' ', NEGRO, BLANCO};
    winner[4][4] = (pixel){' ', NEGRO, BLANCO};
    winner[4][6] = (pixel){' ', NEGRO, BLANCO};
    winner[4][7] = (pixel){' ', NEGRO, BLANCO};
    winner[4][8] = (pixel){' ', NEGRO, BLANCO};
    winner[4][9] = (pixel){' ', NEGRO, BLANCO};
    winner[4][10] = (pixel){' ', NEGRO, BLANCO};
    winner[4][12] = (pixel){' ', NEGRO, BLANCO};
    winner[4][16] = (pixel){' ', NEGRO, BLANCO};
    winner[4][18] = (pixel){' ', NEGRO, BLANCO};
    winner[4][22] = (pixel){' ', NEGRO, BLANCO};
    winner[4][24] = (pixel){' ', NEGRO, BLANCO};
    winner[4][25] = (pixel){' ', NEGRO, BLANCO};
    winner[4][26] = (pixel){' ', NEGRO, BLANCO};
    winner[4][27] = (pixel){' ', NEGRO, BLANCO};
    winner[4][28] = (pixel){' ', NEGRO, BLANCO};
    winner[4][30] = (pixel){' ', NEGRO, BLANCO};
    winner[4][33] = (pixel){' ', NEGRO, BLANCO};
    winner[4][37] = (pixel){' ', NEGRO, BLANCO};
    winner[4][38] = (pixel){' ', NEGRO, BLANCO};
    winner[4][39] = (pixel){' ', NEGRO, BLANCO};
}

void creaDragon() {
    for (int i = 0; i < sizeYDragon; ++i) {
        for (int j = 0; j < sizeXDragon; ++j) {
            dragon[i][j].ch = ' ';
            dragon[i][j].fg = NEGRO;
            dragon[i][j].bg = NEGRO;
        }
    }
         
    dragon[0][2] = (pixel){' ', NEGRO, ROJO};
    dragon[0][3] = (pixel){' ', NEGRO, ROJO};
    dragon[0][4] = (pixel){' ', NEGRO, ROJO};
    dragon[1][3] = (pixel){' ', NEGRO, ROJO};
    dragon[2][1] = (pixel){' ', NEGRO, ROJO};
    dragon[2][2] = (pixel){' ', NEGRO, ROJO};
    dragon[3][2] = (pixel){' ', NEGRO, ROJO};
    dragon[4][1] = (pixel){' ', NEGRO, ROJO};
    dragon[4][3] = (pixel){' ', NEGRO, ROJO};
    dragon[5][0] = (pixel){' ', NEGRO, ROJO};
    dragon[5][2] = (pixel){' ', NEGRO, ROJO};
    dragon[5][3] = (pixel){' ', NEGRO, ROJO};
    dragon[5][4] = (pixel){' ', NEGRO, ROJO};
    dragon[6][2] = (pixel){' ', NEGRO, ROJO};
    dragon[6][3] = (pixel){' ', NEGRO, ROJO};
    dragon[6][6] = (pixel){' ', NEGRO, ROJO};
    dragon[7][5] = (pixel){' ', NEGRO, ROJO};
    dragon[7][6] = (pixel){' ', NEGRO, ROJO};
    dragon[7][7] = (pixel){' ', NEGRO, ROJO};
    dragon[7][9] = (pixel){' ', NEGRO, ROJO};
    
    dragon[0][5] = (pixel){' ', NEGRO, VERDE};
    dragon[0][6] = (pixel){' ', NEGRO, VERDE};
    dragon[0][7] = (pixel){' ', NEGRO, VERDE};
    dragon[0][8] = (pixel){' ', NEGRO, VERDE};
    dragon[1][4] = (pixel){' ', NEGRO, VERDE};
    dragon[1][5] = (pixel){' ', NEGRO, VERDE};
    dragon[1][7] = (pixel){' ', NEGRO, VERDE};
    dragon[2][3] = (pixel){' ', NEGRO, VERDE};
    dragon[2][4] = (pixel){' ', NEGRO, VERDE};
    dragon[2][5] = (pixel){' ', NEGRO, VERDE};
    dragon[2][7] = (pixel){' ', NEGRO, VERDE};
    dragon[2][9] = (pixel){' ', NEGRO, VERDE};
    dragon[3][3] = (pixel){' ', NEGRO, VERDE};
    dragon[3][4] = (pixel){' ', NEGRO, VERDE};
    dragon[3][5] = (pixel){' ', NEGRO, VERDE};
    dragon[3][7] = (pixel){' ', NEGRO, VERDE};
    dragon[3][9] = (pixel){' ', NEGRO, VERDE};
    dragon[4][2] = (pixel){' ', NEGRO, VERDE};
    dragon[4][4] = (pixel){' ', NEGRO, VERDE};
    dragon[5][1] = (pixel){' ', NEGRO, VERDE};
    dragon[5][5] = (pixel){' ', NEGRO, VERDE};
    dragon[5][6] = (pixel){' ', NEGRO, VERDE};
    dragon[5][7] = (pixel){' ', NEGRO, VERDE};
    dragon[5][8] = (pixel){' ', NEGRO, VERDE};
    dragon[5][9] = (pixel){' ', NEGRO, VERDE};
    dragon[6][1] = (pixel){' ', NEGRO, VERDE};
    dragon[6][4] = (pixel){' ', NEGRO, VERDE};
    dragon[6][5] = (pixel){' ', NEGRO, VERDE};
    dragon[7][0] = (pixel){' ', NEGRO, VERDE};
    dragon[7][1] = (pixel){' ', NEGRO, VERDE};
    dragon[7][2] = (pixel){' ', NEGRO, VERDE};
    dragon[7][3] = (pixel){' ', NEGRO, VERDE};
    dragon[7][4] = (pixel){' ', NEGRO, VERDE};
    
    dragon[1][6] = (pixel){' ', NEGRO, BLANCO};
    dragon[1][8] = (pixel){' ', NEGRO, BLANCO};
    dragon[3][6] = (pixel){' ', NEGRO, BLANCO};
    dragon[3][8] = (pixel){' ', NEGRO, BLANCO};
    dragon[4][5] = (pixel){'V', BLANCO, NEGRO};
    dragon[4][6] = (pixel){'V', BLANCO, NEGRO};
    dragon[4][7] = (pixel){'V', BLANCO, NEGRO};
    dragon[4][8] = (pixel){'V', BLANCO, NEGRO};
    dragon[6][7] = (pixel){' ', NEGRO, BLANCO};
    dragon[6][8] = (pixel){' ', NEGRO, BLANCO};
    dragon[7][8] = (pixel){' ', NEGRO, BLANCO};
}

void creaEnemigo() {
    for (int i = 0; i < sizeYEnemigo; ++i) {
        for (int j = 0; j < sizeXEnemigo; ++j) {
            enemigo[i][j].ch = ' ';
            enemigo[i][j].fg = NEGRO;
            enemigo[i][j].bg = NEGRO;
        }
    }
    
    enemigo[0][2] = (pixel){' ', NEGRO, MARRON};
    enemigo[0][3] = (pixel){' ', NEGRO, MARRON};
    enemigo[0][4] = (pixel){' ', NEGRO, MARRON};
    enemigo[1][1] = (pixel){' ', NEGRO, MARRON};
    enemigo[1][5] = (pixel){' ', NEGRO, MARRON};
    enemigo[2][0] = (pixel){' ', NEGRO, MARRON};
    enemigo[2][3] = (pixel){' ', NEGRO, MARRON};
    enemigo[2][6] = (pixel){' ', NEGRO, MARRON};
    enemigo[3][0] = (pixel){' ', NEGRO, MARRON};
    enemigo[3][3] = (pixel){' ', NEGRO, MARRON};
    enemigo[3][6] = (pixel){' ', NEGRO, MARRON};
    enemigo[4][1] = (pixel){' ', NEGRO, MARRON};
    enemigo[4][2] = (pixel){' ', NEGRO, MARRON};
    enemigo[4][3] = (pixel){' ', NEGRO, MARRON};
    enemigo[4][4] = (pixel){' ', NEGRO, MARRON};
    enemigo[4][5] = (pixel){' ', NEGRO, MARRON};  
    
    enemigo[2][1] = (pixel){' ', NEGRO, BLANCO};
    enemigo[2][5] = (pixel){' ', NEGRO, BLANCO};  
    enemigo[3][1] = (pixel){' ', NEGRO, BLANCO};
    enemigo[3][2] = (pixel){' ', NEGRO, BLANCO}; 
    enemigo[3][4] = (pixel){' ', NEGRO, BLANCO};
    enemigo[3][5] = (pixel){' ', NEGRO, BLANCO}; 
    enemigo[5][1] = (pixel){' ', NEGRO, BLANCO};
    enemigo[5][2] = (pixel){' ', NEGRO, BLANCO}; 
    enemigo[5][3] = (pixel){' ', NEGRO, BLANCO};
    enemigo[5][4] = (pixel){' ', NEGRO, BLANCO}; 
    enemigo[5][5] = (pixel){' ', NEGRO, BLANCO};
    enemigo[6][3] = (pixel){' ', NEGRO, BLANCO};
    
    enemigo[0][0] = (pixel){' ', NEGRO, BLANCO};
    enemigo[0][1] = (pixel){' ', NEGRO, BLANCO};  
    enemigo[0][5] = (pixel){' ', NEGRO, BLANCO};
    enemigo[0][6] = (pixel){' ', NEGRO, BLANCO}; 
    enemigo[1][2] = (pixel){' ', NEGRO, BLANCO};
    enemigo[1][3] = (pixel){' ', NEGRO, BLANCO}; 
    enemigo[1][4] = (pixel){' ', NEGRO, BLANCO}; 
    
    enemigo[6][0] = (pixel){' ', NEGRO, ROJO};
    enemigo[6][1] = (pixel){' ', NEGRO, ROJO};
    enemigo[6][2] = (pixel){' ', NEGRO, ROJO};
    enemigo[6][4] = (pixel){' ', NEGRO, ROJO};   
    enemigo[6][5] = (pixel){' ', NEGRO, ROJO};
    enemigo[6][6] = (pixel){' ', NEGRO, ROJO};     
}

void creaDiamante(){
    for (int i = 0; i < sizeYDiamante; ++i) {
        for (int j = 0; j < sizeXDiamante; ++j) {
            diamante[i][j].ch = ' ';
            diamante[i][j].fg = NEGRO;
            diamante[i][j].bg = NEGRO;
        }
    }
    
    diamante[0][1] = (pixel){' ', NEGRO, CIAN};
    diamante[0][2] = (pixel){' ', NEGRO, CIAN};
    diamante[0][3] = (pixel){' ', NEGRO, CIAN};
    diamante[1][0] = (pixel){' ', NEGRO, CIAN};
    diamante[1][1] = (pixel){' ', NEGRO, BLANCO};
    diamante[1][2] = (pixel){' ', NEGRO, CIAN};
    diamante[1][3] = (pixel){' ', NEGRO, CIAN};
    diamante[1][4] = (pixel){' ', NEGRO, CIAN};
    diamante[2][1] = (pixel){' ', NEGRO, CIAN};
    diamante[2][2] = (pixel){' ', NEGRO, BLANCO};
    diamante[2][3] = (pixel){' ', NEGRO, CIAN};
    diamante[3][2] = (pixel){' ', NEGRO, CIAN};
}

int posicionValida(char ** map, int x, int y, int sizeX, int sizeY) {
  char buffer[1];
  buffer[0] = ' ';
  
  for(int i = y; (i < y + sizeY) && (i < ROWS); ++i) {
  	for(int j = x; (j < x + sizeX) && (j < COLS); ++j) {
  		if(map[i][j] == '0') return 0;
  	}
  }
  
  return 1;
}

int contadorEnemigos = 0;

void mueveEnemigos() {
 if(contadorEnemigos >= 5) {
  	contadorEnemigos = 0;
  	
  	if(enemigo1->dir == LEFT) enemigo1->dir = RIGHT;
  	else if(enemigo1->dir == RIGHT) enemigo1->dir = LEFT;
  	
  	if(enemigo2->dir == LEFT) enemigo2->dir = RIGHT;
  	else if(enemigo2->dir == RIGHT) enemigo2->dir = LEFT;
  }
  else {
  	++contadorEnemigos;
  	
  	if(enemigo1->dir == LEFT) --enemigo1->x;
  	else if(enemigo1->dir == RIGHT) ++enemigo1->x;
  	
  	if(enemigo2->dir == LEFT) --enemigo2->x;
  	else if(enemigo2->dir == RIGHT) ++enemigo2->x;
  }
}

int hayColisionEnemigo(int numEnemigo) {
  int px1 = player->x;
  int py1 = player->y;
  int px2 = px1 + sizeXDragon - 1;
  int py2 = py1 + sizeYDragon - 1;

  int ex1 = enemigo1->x;
  int ey1 = enemigo1->y;
  
  if(numEnemigo == 2) {
  	ex1 = enemigo2->x;
  	ey1 = enemigo2->y;
  }
  
  int ex2 = ex1 + sizeXEnemigo - 1;
  int ey2 = ey1 + sizeYEnemigo - 1;
  
  if (px2 < ex1 || px1 > ex2 || py2 < ey1 || py1 > ey2) return 0;
  return 1;
}

int hayColisionDiamante(int dx1, int dy1) {
  int px1 = player->x;
  int py1 = player->y;
  int px2 = px1 + sizeXDragon - 1;
  int py2 = py1 + sizeYDragon - 1;
  
  int dx2 = dx1 + sizeXDiamante - 1;
  int dy2 = dy1 + sizeYDiamante - 1;
  
  if (px2 < dx1 || px1 > dx2 || py2 < dy1 || py1 > dy2) return 0;
  return 1;
}

void gameThread() {  
  int xAntPlayer = player->x;
  int yAntPlayer = player->y;
  int xAntEnemigo1 = enemigo1->x;
  int yAntEnemigo1 = enemigo1->y;
  int xAntEnemigo2 = enemigo2->x;
  int yAntEnemigo2 = enemigo2->y;
  
  while(1) {	
  	semWait(semChanges);
  	//Pinta diamantes:
  	pintaPixel(5, ROWS - 8, sizeXDiamante, sizeYDiamante, diamante);
 	pintaPixel(70, ROWS - 8, sizeXDiamante, sizeYDiamante, diamante);
  	//Mueve y pinta los proyectiles:
  	moverProyectiles();
  	//Borra y pinta a los enemigos:
  	if(!enemigo1muerto || !enemigo2muerto) {
  		mueveEnemigos();
  	
  		if(!enemigo1muerto) {
  			gotoxy(xAntEnemigo1, yAntEnemigo1);
  			pintaTrozoMapa(xAntEnemigo1, yAntEnemigo1, sizeXEnemigo, sizeYEnemigo);
  			pintaPixel(enemigo1->x, enemigo1->y, sizeXEnemigo, sizeYEnemigo, enemigo);
  			xAntEnemigo1 = enemigo1->x;
  			yAntEnemigo1 = enemigo1->y;
  		}
  		
  		if(!enemigo2muerto) {
  			gotoxy(xAntEnemigo2, yAntEnemigo2);
  			pintaTrozoMapa(xAntEnemigo2, yAntEnemigo2, sizeXEnemigo, sizeYEnemigo);
  			pintaPixel(enemigo2->x, enemigo2->y, sizeXEnemigo, sizeYEnemigo, enemigo);
  			xAntEnemigo2 = enemigo2->x;
  			yAntEnemigo2 = enemigo2->y;
  		}
  	}	
	//Borra y pinta al player:
	gotoxy(xAntPlayer, yAntPlayer);
	
	pintaTrozoMapa(xAntPlayer, yAntPlayer, sizeXDragon, sizeYDragon);
	
	switch(player->state) {
	  case ST_UP: --(player->y); break;
	  case ST_DOWN: ++(player->y); break;
	  case ST_LEFT: --(player->x); break;
	  case ST_RIGHT: ++(player->x); break;
	  default: break;
	}
	
	if(!posicionValida(mapa, player->x, player->y, sizeXDragon, sizeYDragon)) {
		player->x = xAntPlayer;
		player->y = yAntPlayer;
	}
	
	if(player->dir == RIGHT) pintaPixel(player->x, player->y, sizeXDragon, sizeYDragon, dragon);
	else pintaDragonIzquierda(player->x, player->y);
	
	xAntPlayer = player->x;
  	yAntPlayer = player->y;
  	
  	player->state = ST_STOP;
  	
  	if(hayColisionEnemigo(1) && !enemigo1muerto || hayColisionEnemigo(2) && !enemigo2muerto) {
  		pintaPixel(17, 4, sizeXLoser, sizeYLoser, loser);
  		exit_thread();
  	}
  	else if(hayColisionDiamante(5, ROWS - 8) || hayColisionDiamante(70, ROWS - 8)) {
  		pintaPixel(20, 4, sizeXWinner, sizeYWinner, winner);
  		exit_thread();
  	}
  }
}

int __attribute__((__section__(".text.main"))) main(void) {
  /* Next line, tries to move value 0 to CR3 register. This register is a
   * privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  /*
  test_write();
  test_set_color();
  test_gotoxy();
  test_fork();
  test_block_unblock();
  test_read();
  test_combinado1();
  test_combinado2();
  test_thread();
  test_dyn_mem();
  test_dyn_mem2();
  test_sem();
  test_sem2();
  test_malloc_free();
  */
  
  // test_read2();
  // test_read3();
  // test_fork2();

  player = (character *)malloc(sizeof(character));
  enemigo1 = (character *)malloc(sizeof(character));
  enemigo2 = (character *)malloc(sizeof(character));
  
  mapa = (char **)malloc(ROWS * sizeof(char *));

  for (int i = 0; i < ROWS; ++i) mapa[i] = (char *)malloc(COLS * sizeof(char));
  
  dragon = (pixel**)malloc(sizeof(pixel*) * sizeYDragon);
  
  for (int i = 0; i < sizeYDragon; ++i) dragon[i] = (pixel*)malloc(sizeof(pixel) * sizeXDragon);
  
  enemigo = (pixel**)malloc(sizeof(pixel*) * sizeYEnemigo);
  
  for (int i = 0; i < sizeYEnemigo; ++i) enemigo[i] = (pixel*)malloc(sizeof(pixel) * sizeXEnemigo);
  
  diamante = (pixel**)malloc(sizeof(pixel*) * sizeYDiamante);
  
  for (int i = 0; i < sizeYDiamante; ++i) diamante[i] = (pixel*)malloc(sizeof(pixel) * sizeXDiamante);
  
  winner = (pixel**)malloc(sizeof(pixel*) * sizeYWinner);
  
  for (int i = 0; i < sizeYWinner; ++i) winner[i] = (pixel*)malloc(sizeof(pixel) * sizeXWinner);
  
  loser = (pixel**)malloc(sizeof(pixel*) * sizeYLoser);
  
  for (int i = 0; i < sizeYLoser; ++i) loser[i] = (pixel*)malloc(sizeof(pixel) * sizeXLoser);
  
  proyectiles = (proyectil *)malloc(sizeof(proyectil) * MAX_PROYECTILES);
  //Ponemos que no hay proyectiles
  for (int i = 0; i < MAX_PROYECTILES; ++i) proyectiles[i].activo = 0;
  
  creaMapa(mapa);
  pintaMapa(mapa);
  
  //Crea los pixeles para cada objeto y titulo:
  creaDiamante();
  creaWinner();
  creaLoser();
  creaDragon();
  creaEnemigo();
  
  player->x = COLS/2;
  player->y = ROWS - 9;
  player->dir = RIGHT;
  player->state = ST_STOP;
  
  enemigo1->x = 13;
  enemigo1->y = ROWS - 9;
  enemigo1->dir = RIGHT;
  
  enemigo2->x = 60;
  enemigo2->y = ROWS - 9;
  enemigo2->dir = LEFT;
  
  semChanges = semCreate(0);
  
  create_thread(&keyboardThread, &stackKeyboard[100], NULL);
  create_thread(&gameThread, &stackGame[1024], NULL);
  
  semSignal(semChanges);
  while (1) {
  	for(int i = 0; i < 10000000; ++i);
  	semSignal(semChanges);
  }
}
