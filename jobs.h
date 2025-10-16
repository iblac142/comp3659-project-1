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

int run_job(struct Job* job);

/*
Runs given command as new program through fork then exevcp call

command - command to run contained in Command structure

Returns:
  0 if run successful
  -1 if error while forking
  -2 if error while calling exevcp (calling program needs to terminate 
      it is a duplicate proccess)
  -3 if error while waiting on new program
*/
int run_command(struct Command* command, int infile, int outfile, int wait);

int run_command_no_fork(struct Command* command, int infile, int outfile);

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
