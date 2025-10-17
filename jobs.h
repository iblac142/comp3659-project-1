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
};

/*
Runs given job with support for multi-stage pipelines, I/O redirection, and background jobs

job - pointer to Job structure containing job to execute

Return:
    0 if successful

    -1 through -4 for single-stage pipelines
    -1 if error while forking (from run_command)
    -2 if error while waiting for program (from run_command, foreground jobs only)
    -3 if error opening input file
    -4 if error opening output file

    -5 through -7 for multi-stage pipelines
    -5 if error while creating pipes
    -6 if error while executing pipelines (a fork failed)
    -7 if error while waiting for children proccesses (only in foreground execution)

*/
int run_job(struct Job* job);

/* Prompts user, collects command line into a buffer,
	
then tokenizes the command line and fills in supplied
    
command struct with the data
	
Returns:
	0 if run successful
	-1 if error due to too many characters
  -2 if error due to too many arguments
*/
int get_command(struct Command* command);


#endif
