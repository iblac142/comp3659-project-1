#include "mystring.h"
#include <string.h>       /* TO DO: initial cheat! Remove this line and all library dependency evetually */


unsigned int mystrlen(const char *s)
{
  return strlen(s);     /* TO DO: replace string library wrappers with non-library code */
}


int mystrcmp(const char *s1, const char *s2)
{
  while (*s1 != '\0' && (*s1 == *s2)) {
    s1 += 1;
    s2 += 1;
  }
  if (*s1 == *s2) {
    return 0;
  } else {
    return -1;
  }
}


void *mystrcpy(char *dest, const char *src)
{
  int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i += 1;
    }
}
