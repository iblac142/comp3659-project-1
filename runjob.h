#ifndef RUNJOB_H
#define RUNJOB_H

#include "jobs.h"

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

/*
Checks for any zombie children of current proccess and cleans them up, will loop untill
there are no more zombies

No arguments or return values
*/
void check_for_zombies();

#endif