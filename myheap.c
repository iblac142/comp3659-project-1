#include "myheap.h"

#define HEAP_SIZE 4096     /* adjust as necessary */

static char heap[HEAP_SIZE];
static char *freep = heap;


char *alloc(unsigned int size)
{
  freep += size;
  return freep -= size;
}


void free_all()
{
  freep = heap;
  return;
}

char *heap_start() {
  return heap;
}
