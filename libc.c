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

char *heap_start = NULL;
char *heap_end = NULL;

block_header *next_block(block_header *block) {
  return (block_header *)((char *)block + block->size);
}

static block_header *prev_block(block_header *block) {
  block_header *block_footer =
      (block_header *)((char *)block - MALLOC_HEADER_SIZE);
  int prev_size = block_footer->size;
  return (block_header *)((char *)block - prev_size);
}

static void write_header_footer(block_header *block, int size, int is_free) {
  block->size = size;
  block->is_free = is_free;
  block_header *footer =
      (block_header *)((char *)block + size - MALLOC_HEADER_SIZE);
  footer->size = size;
  footer->is_free = is_free;
}

char *malloc(int num_bytes) {
  if (num_bytes <= 0)
    return (char *)-1;

  int total_size = num_bytes + 2 * MALLOC_HEADER_SIZE;

  if (!heap_start) {
    heap_start = dyn_mem(1);
    heap_end = heap_start + PAGE_SIZE;
    write_header_footer((block_header *)heap_start, PAGE_SIZE, 1);
  }

  block_header *current = (block_header *)heap_start;
  while ((char *)current < heap_end) {
    if (current->is_free && current->size >= total_size) {
      int rem = current->size - total_size;
      if (rem > 2 * MALLOC_HEADER_SIZE) {
        write_header_footer(current, total_size, 0);
        block_header *partition = next_block(current);
        write_header_footer(partition, rem, 1);
      } else {
        write_header_footer(current, current->size, 0);
      }
      return (char *)current + MALLOC_HEADER_SIZE;
    }
    current = next_block(current);
  }

  int pages_needed = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
  char *new_mem = dyn_mem(pages_needed);
  char *old_heap_end = heap_end;
  heap_end = new_mem + pages_needed * PAGE_SIZE;

  block_header *new_block = (block_header *)old_heap_end;
  write_header_footer(new_block, pages_needed * PAGE_SIZE, 0);
  return (char *)new_block + MALLOC_HEADER_SIZE;
}

int free(char *p) {
  if (!p || !heap_start)
    return -1;
  if (p < heap_start + MALLOC_HEADER_SIZE || p >= heap_end)
    return -1;

  block_header *block = (block_header *)(p - MALLOC_HEADER_SIZE);
  if (block->is_free)
    return -1;

  write_header_footer(block, block->size, 1);

  block_header *next = next_block(block);
  if ((char *)next < heap_end && next->is_free) {
    int new_size = block->size + next->size;
    write_header_footer(block, new_size, 1);
  }

  if ((char *)block > heap_start) {
    block_header *prev = prev_block(block);
    if (prev->is_free) {
      int new_size = block->size + prev->size;
      write_header_footer(prev, new_size, 1);
      block = prev;
    }
  }

  block_header *last = prev_block((block_header *)((char *)heap_end));
  if (last->is_free) {
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
