#ifndef JOBS_H
#define JOBS_H

#define MAX_ARGS 64  

struct Command
{
  char *argv[MAX_ARGS+1];
  unsigned int argc;
};

#endif
