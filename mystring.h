#ifndef MY_STRING_H
#define MY_STRING_H

/* Checks to see if two null-terminated strings are the same

s1 - One string to check
s2 - Another string to check

Returns
  0 if the strings are the same
  -1 if the strings are not the same
*/
int mystrcmp(const char *s1, const char *s2);

/* Copies the contents of one string to another

dest - where to copy src to
src - the beginning to the string to copy to dst

No return values
*/
void *mystrcpy(char *dest, const char *src);

#endif
