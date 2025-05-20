/*
 * libc.c
 */

#include "include/libc.h"
#include <libc.h>

#include <types.h>

#include <errno.h>

int errno = 0;

void itoa(int a, char *b) {
  int i, i1;
  char c;

  if (a == 0) {
    b[0] = '0';
    b[1] = 0;
    return;
  }

  i = 0;
  while (a > 0) {
    b[i] = (a % 10) + '0';
    a = a / 10;
    i++;
  }

  for (i1 = 0; i1 < i / 2; i1++) {
    c = b[i1];
    b[i1] = b[i - i1 - 1];
    b[i - i1 - 1] = c;
  }
  b[i] = 0;
}

int strlen(char *a) {
  int i;

  i = 0;

  while (a[i] != 0)
    i++;

  return i;
}

void perror() {
  char buff[3];
  switch (errno) {
  case ENOSYS:
    write(1, "Syscall not implemented\n", 24);
    break;
  case EFAULT:
    write(1, "Bad address\n", 12);
    break;
  case EINVAL:
    write(1, "Invalid argument\n", 17);
    break;
  default:
    itoa(errno, buff);
    write(1, "Message for error ", 18);
    write(1, buff, strlen(buff));
    write(1, " not found\n", 11);
    break;
  }
}

char *heap_start = 0;
char *heap_end = 0;

block_header *next_block(block_header *block) {
  return (block_header *)((char *)block + block->size);
}

char *malloc(int num_bytes) {
  if (num_bytes <= 0)
    return (char *)-1;

  int total_size = num_bytes + MALLOC_HEADER_SIZE;
  if (total_size % MALLOC_HEADER_SIZE != 0) {
    total_size += MALLOC_HEADER_SIZE - (total_size % 8);
  }

  if (!heap_start) {
    heap_start = dyn_mem(1);
    heap_end = heap_start + PAGE_SIZE;
    block_header *initial = (block_header *)heap_start;
    initial->size = PAGE_SIZE;
    initial->is_free = 1;
  }

  block_header *current = (block_header *)heap_start;
  while ((char *)current < heap_end) {
    if (current->is_free && current->size >= total_size) {
      if (current->size >= total_size +  MALLOC_HEADER_SIZE + 8) {
        block_header *new_block =
            (block_header *)((char *)current + total_size);
        new_block->size = current->size - total_size;
        new_block->is_free = 1;
        current->size = total_size;
      }
      current->is_free = 0;
      return (char *)current + MALLOC_HEADER_SIZE;
    }
    current = next_block(current);
  }

  int pages_needed = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
  char *new_mem = dyn_mem(pages_needed);
  char *old_heap_end = heap_end;
  heap_end = new_mem + pages_needed * PAGE_SIZE;

  block_header *new_block = (block_header *)old_heap_end;
  new_block->size = pages_needed * PAGE_SIZE;
  new_block->is_free = 0;

  return (char *)new_block + MALLOC_HEADER_SIZE;
}

int free(char *p) {
  block_header *block = (block_header *)(p - MALLOC_HEADER_SIZE);
  block->is_free = 1;

  block_header *current = (block_header *)heap_start;
  while ((char *)current < heap_end) {
    block_header *next = next_block(current);
    if ((char *)next >= heap_end)
      break;

    if (current->is_free && next->is_free) {
      current->size += next->size;
      continue;
    }
    current = next;
  }

  current = (block_header *)heap_start;
  block_header *last = NULL;
  while ((char *)current < heap_end) {
    last = current;
    current = next_block(current);
  }

  if (last && last->is_free) {
    char *block_end = (char *)last + last->size;
    int free_bytes = heap_end - (char *)last;

    int free_pages = free_bytes / PAGE_SIZE;
    if (free_pages > 0 && block_end == heap_end) {
      heap_end -= free_pages * PAGE_SIZE;
      last->size -= free_pages * PAGE_SIZE;
      dyn_mem(-(free_pages));
    }
  }
  return 0;
}
