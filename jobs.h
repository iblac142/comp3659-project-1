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

const struct Job clear = {0};

int run_job(struct Job* job);
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

int check_for(char n);
/* Checks if a given symbol is whitespace, null, or a terminal symbol

n - the symbol to be checked

Returns:
  0 if the symbol is space, tab, or null (' ', '\t', '\0')
  1 if the symbol is |
  2 if the symbol is <
  3 if the symbol is >
  4 if the symbol is &
  5 if the symbol is new line ('\n')
  -1 if the symbol is anything else 
*/

int process_commands(struct Job* job);
/* Populates the commands of the supplied job structure until one of < > & or \n are encountered

job - the job structure to be populated

Returns:
  a positive value if run successful, equal to the position of the terminal symbol required
    relative to the start of the heap
  -2 if a command has too many arguments
  -3 if the pipeline has too many commands in it
  -50 if an exit command is detected
*/

int process_job(struct Job* job);
/* Populates the supplied job structure by reading from the heap.
First, populates the command structures via process_commands,
then populates any other relevant fields itself.

job - the job structure to be populated

Returns:
  1 if an exit command is detected
  0 if run successful
  -2 if a command has too many arguments 
  -3 if the pipeline has too many commands in it 
  -4 if a malformed command is detected
*/


int tokenize_line(char* buffer);
/* Tokenizes the contents of the supplied buffer onto the heap,
removing excess whitespace, adding command path prefixes,
and null terminating each token

buffer - the beginning of the buffer to be tokenized

Returns:
  0 if run successful
  -4 if a malformed command is detected

*/


int get_job(struct Job* job);
/* Prompts user, collects command line into a buffer, then
tokenizes the command line and fills in supplied job struct

job - the job structure to be populated
	
Returns:
  1 if an exit command is detected
	0 if run successful
	-1 if error due to too many characters
  -2 if error due to too many arguments
  -3 if error due to too many pipeline stages
  -4 if error due to malformed command 
*/

#endif
