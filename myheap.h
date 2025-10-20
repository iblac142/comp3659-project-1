#ifndef MY_HEAP_H
#define MY_HEAP_H

/* Allocates the given amount of space to the heap
and returns the pointer to the beginning of that space.

size - the number of chars (bytes) to allocate

Returns
  The pointer which points to the start of the allocated space
*/
char *alloc(unsigned int size);

/* Allocates the given amount of space to the heap
and returns the pointer to the beginning of that space.

Takes no arguments
No return values
*/
void free_all();

/* Returns a pointer to the first item in the heap

Takes no arguments

Returns:
    a pointer to the first item in the heap
*/
char *heap_start();

#endif
