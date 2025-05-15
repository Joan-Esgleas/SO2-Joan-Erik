/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>


int write(int fd, char *buffer, int size);

int gotoxy(int x, int y);

int set_color(int fg, int bg);

int read(char* b, int maxchars);

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();

void exit();

void block();

int unblock(int pid);

void perror();

#endif  /* __LIBC_H__ */
