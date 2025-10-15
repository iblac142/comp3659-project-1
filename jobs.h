#ifndef JOBS_H
#define JOBS_H

#define MAX_ARGS 64  
#define MAX_PIPELINE_LEN 64

struct Command
{
  char *argv[MAX_ARGS+1];
  unsigned int argc;
};

struct Job
{
  struct Command pipeline[MAX_PIPELINE_LEN];
  unsigned int num_stages;
  char *outfile_path;		/* NULL for no output redirection */
  char *infile_path;		/* NULL for no input redirection */
  int background;			/* 0 for foreground, 1 for background */
}


#endif
