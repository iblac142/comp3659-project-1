#ifndef GETJOB_H
#define GETJOB_H

#include "jobs.h"

/* Prompts user, collects command line into a buffer, then
tokenizes the command line and fills in supplied job struct

job - the job structure to be populated
	
Returns:
  1 if an exit command is detected
  0 if run successful
  1 if error due to too many characters
  -2 if error due to too many arguments
  -3 if error due to too many pipeline stages
  -4 if error due to malformed command 
*/
int get_job(struct Job* job);

#endif