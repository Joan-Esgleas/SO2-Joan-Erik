/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

#define PAGE_SIZE 0x1000
#define MALLOC_HEADER_SIZE 8
#define NULL 0

typedef struct block_header {
    int size;
    int is_free;
} block_header;

int write(int fd, char *buffer, int size);

int gotoxy(int x, int y);

int set_color(int fg, int bg);

int read(char* b, int maxchars);

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();

int create_thread(void (*function)(void* arg), void* stack, void* parameter);

void exit();
void exit_thread();
int wait_thread(int pid);

char* dyn_mem(int num_pages);

char* malloc(int num_bytes);
int free(char* p);

void block();

int unblock(int pid);

int semCreate(int initial_value);
int semWait(int semid);
int semSignal(int semid);
int semDestroy(int semid);

void perror();

#endif  /* __LIBC_H__ */
