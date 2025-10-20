#include "jobs.h"
#include "get.h"
#include "myheap.h"
#include "mystring.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PIPE_READ_END  0
#define PIPE_WRITE_END 1

const int maxBuffer = 256;
const char *prompt = "$ ";
const char *lengthError = "Message exceeds max length of 256, please re-enter command with shorter length\n";
const char *forkError = "Error occurred while forking new process\n";
const char *execveError = "Error occurred while executing program\n";
const char *waitpidError = "Error occurred while waiting for program\n"; 
const char *inOpenError = "Error while opening file for input\n";
const char *outOpenError = "Error while opening file for output\n";
const char *pipeError = "Error while creating pipes\n";

const struct Job clear = {0};

/*
Helper function to run a command with optional waiting

command - pointer to Command structure containing the command to execute
infile - input file descriptor (0 for stdin)
outfile - output file descriptor (0 for stdout)
wait - 1 to wait for command completion, 0 for background execution

Returns:
    0 if execution successful
    -1 if error while forking
    -2 if error while waiting on new program
*/
static int run_command(struct Command* command, int infile, int outfile, int wait){
    pid_t pid;
    int status;

    //fork new proccess
    pid = fork();

    if (pid == -1) {
        write(1, forkError, 41);
        return -1;
    }

    if (pid == 0) {
        //in child
        if (infile != 0) {
            dup2(infile, 0);
            close(infile);
        }
        if (outfile != 0) {
            dup2(outfile, 1);
            close(outfile);
        }
        execve(command->argv[0], command->argv, NULL);
        write(1, execveError, 39);
        _exit(2);
    }

    if (pid != 0) {
        //in parent
        if (wait == 1) {
            pid = waitpid(pid, &status, 0);
            if (pid == -1) {
                write(1, waitpidError, 41);
                return -2;
            }
        }
        
        return 0;
    }
}

/*
Helper function to run a command without forking (used in pipeline children)

command - pointer to Command structure containing the command to execute
infile - input file descriptor (0 for stdin)
outfile - output file descriptor (0 for stdout)

Returns:
    void (exits child process via execve or _exit)
*/
static void run_command_no_fork(struct Command* command, int infile, int outfile) {
    if (infile != 0) {
        dup2(infile, 0);
        close(infile);
    }
    if (outfile != 0) {
        dup2(outfile, 1);
        close(outfile);
    }
    execve(command->argv[0], command->argv, NULL);
    write(1, execveError, 39);
    _exit(2);
}

/*
Helper function to handle single-stage job execution with file I/O redirection

job - pointer to Job structure containing the command and I/O redirection info

Returns:
    0 if execution successful
    -1 if error while forking (from run_command)
    -2 if error while waiting for program (from run_command, foreground jobs only)
    -3 if error opening input file
    -4 if error opening output file
*/
static int run_single_stage_job(struct Job* job) {
    int in = 0;
    int out = 0;
    int should_wait = 1;
    
    if (job->infile_path != NULL) {
        in = open(job->infile_path, O_RDONLY);
        if (in == -1) {
            write(1, inOpenError, 35);
            return -3;
        }
    }
    if (job->outfile_path != NULL) {
        out = open(job->outfile_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (out == -1) {
            write(1, outOpenError, 36);
            if (in != 0) close(in);
            return -4;
        }
    }
    
    if (job->background) {
        should_wait = 0;
    }
    int result = run_command(&job->pipeline[0], in, out, should_wait);

    if (in != 0) close(in);
    if (out != 0) close(out);
    
    return result;
}

/*
Helper function to create pipes for pipeline communication

numberOfPipes - number of pipes to create (num_stages - 1)
pipes - 2D array to store pipe file descriptors

Returns:
    0 if all pipes created successfully
    -1 if error creating pipes (closes any already created pipes)
*/
static int create_pipes(int numberOfPipes, int pipes[][2]) {
    for (int i = 0; i < numberOfPipes; i++) {
        if (pipe(pipes[i]) == -1) {
            write(1, pipeError, 27);
            // Close already created pipes
            for (int j = 0; j < i; j++) {
                close(pipes[j][PIPE_READ_END]);
                close(pipes[j][PIPE_WRITE_END]);
            }
            return -1;
        }
    }
    return 0;
}

/*
Helper function to close all pipes in a pipeline

numberOfPipes - number of pipes to close
pipes - 2D array containing pipe file descriptors

Returns:
    void
*/
static void close_all_pipes(int numberOfPipes, int pipes[][2]) {
    for (int j = 0; j < numberOfPipes; j++) {
        close(pipes[j][PIPE_READ_END]);
        close(pipes[j][PIPE_WRITE_END]);
    }
}

/*
Helper function to setup and run first command in pipeline (runs in child process)

job - pointer to Job structure containing command and I/O redirection info
pipes - 2D array containing pipe file descriptors
numberOfPipes - number of pipes in the pipeline

Returns:
    void (child proccess becomes first command during function)
*/
static void setup_first_command(struct Job* job, int pipes[][2], int numberOfPipes) {
    int in = 0;
    int out = 0;
    
    if (job->infile_path != NULL) {
        in = open(job->infile_path, O_RDONLY);
        if (in == -1) {
            write(1, inOpenError, 35);
            _exit(1);
        }
    }

    // out is first pipe write
    out = pipes[0][PIPE_WRITE_END];

    // close all other pipes
    close(pipes[0][PIPE_READ_END]);
    for (int j = 1; j < numberOfPipes; j++) {
        close(pipes[j][PIPE_READ_END]);
        close(pipes[j][PIPE_WRITE_END]);
    }

    run_command_no_fork(&job->pipeline[0], in, out);
}

/*
Helper function to setup and run last command in pipeline (runs in child process)

job - pointer to Job structure containing command and I/O redirection info
pipes - 2D array containing pipe file descriptors
numberOfPipes - number of pipes in the pipeline
stage_index - index of the command in the pipeline

Returns:
    void (child proccess becomes last command during function)
*/
static void setup_last_command(struct Job* job, int pipes[][2], int numberOfPipes, int stage_index) {
    int in = 0;
    int out = 0;
    
    if (job->outfile_path != NULL) {
        out = open(job->outfile_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (out == -1) {
            write(1, outOpenError, 36);
            _exit(2);
        }
    }

    // in is last pipe read
    in = pipes[numberOfPipes - 1][PIPE_READ_END];
    
    // close all other pipes
    close(pipes[numberOfPipes - 1][PIPE_WRITE_END]);
    for (int j = 0; j < numberOfPipes - 1; j++) {
        close(pipes[j][PIPE_READ_END]);
        close(pipes[j][PIPE_WRITE_END]);
    }
    
    run_command_no_fork(&job->pipeline[stage_index], in, out);
}

/*
Helper function to setup and run middle commands in pipeline (runs in child process)

job - pointer to Job structure containing command and I/O redirection info
pipes - 2D array containing pipe file descriptors
numberOfPipes - number of pipes in the pipeline
stage_index - index of the command in the pipeline

Returns:
    void (child proccess becomes specified middle command during function)
*/
static void setup_middle_command(struct Job* job, int pipes[][2], int numberOfPipes, int stage_index) {
    //in and out are both pipes
    int in = pipes[stage_index - 1][PIPE_READ_END];
    int out = pipes[stage_index][PIPE_WRITE_END];

    // close all other pipes
    for (int j = 0; j < stage_index - 1; j++) {
        close(pipes[j][PIPE_READ_END]);
        close(pipes[j][PIPE_WRITE_END]);
    }
    close(pipes[stage_index - 1][PIPE_WRITE_END]);
    close(pipes[stage_index][PIPE_READ_END]);
    for (int j = stage_index + 1; j < numberOfPipes; j++) {
        close(pipes[j][PIPE_READ_END]);
        close(pipes[j][PIPE_WRITE_END]);
    }

    run_command_no_fork(&job->pipeline[stage_index], in, out);
}

/*
Helper function to execute pipeline stages by forking child processes

job - pointer to Job structure containing commands and I/O redirection info
pipes - 2D array containing pipe file descriptors
numberOfPipes - number of pipes in the pipeline
pids - array to store child process IDs

Returns:
    0 if all stages forked successfully
    -1 if a fork failed (cleans up pipes and waits for already started children)
*/
static int execute_pipeline(struct Job* job, int pipes[][2], int numberOfPipes, pid_t pids[]) {
    // Loop through each command and run it(can't wait for child commands until whole job is running)
    for (int i = 0; i < job->num_stages; i++) {
        pids[i] = fork();

        if (pids[i] == 0) { // in child
            if (i == 0) { // first command in pipeline
                setup_first_command(job, pipes, numberOfPipes);
            }
            else if (i == job->num_stages - 1) { // last command in pipeline
                setup_last_command(job, pipes, numberOfPipes, i);
            }
            else { // all commands between first and last
                setup_middle_command(job, pipes, numberOfPipes, i);
            }
        }

        else if (pids[i] < 0) { // fork failed in parent
            write(1, forkError, 41);
            // close all pipes
            close_all_pipes(numberOfPipes, pipes);
            // wait for already started children to avoid zombies
            for (int k = 0; k < i; k++) {
                int status;
                waitpid(pids[k], &status, 0);
            }
            return -1;
        }
    }

    close_all_pipes(numberOfPipes, pipes);
    
    return 0;
}

/*
Helper function to wait for all child processes to complete

pids - array of child process IDs
num_stages - number of child processes to wait for

Returns:
    0 if all children handled successfully
    -1 if waitpid error occurred
*/
static int wait_for_children(pid_t pids[], int num_stages) {
    int status;

    for (int i = 0; i < num_stages; i++) {
        pid_t result = waitpid(pids[i], &status, 0);
        if (result == -1) {
            write(1, waitpidError, 41);
            return -1;
        }
    }
    return 0;
}

int run_job(struct Job* job) {
    if (job->num_stages == 1) {
        return run_single_stage_job(job);
    }
    
    // Multi-stage pipeline
    int numberOfPipes = job->num_stages - 1;
    int pipes[numberOfPipes][2];
    pid_t pids[job->num_stages];
    
    // Create pipes
    if (create_pipes(numberOfPipes, pipes) != 0) {
        return -5;
    }
    
    // Execute pipeline
    int result = execute_pipeline(job, pipes, numberOfPipes, pids);
    if (result != 0) {
        return -6;
    }
    
    // Wait for all children (only if not background job)
    if (!job->background) {
        result = wait_for_children(pids, job->num_stages);
        if (result != 0) {
            return -7;
        }
    }  
    
    return 0;
}

void check_for_zombies() {
    int status;

    while (waitpid(-1, &status, WNOHANG) > 0) {} 
}

int get_job(struct Job* job) {
	char buffer[maxBuffer];
    int readLength;
    int status;

    //prompt and read input
    write(1, prompt, 2);
    readLength = read(0, buffer, maxBuffer);
    
    //check length
    while (readLength >= maxBuffer) {
        //display error message and reprompt
        write(1, lengthError, 80);
        write(1, prompt, 2);
        //discard rest of input not read by first read
        char discard;
        while (read(0, &discard, 1) == 1 && discard != '\n') {
        }
        //clear buffer
        for (int i = 0; i < maxBuffer; i++) {
            buffer[i] = 0;
        }
        //return
        return -1;
    }

    // clear the heap
    free_all();

	// clear the previous job
    *job = clear;

    // tokenize the entire command line for simple parsing
    status = tokenize_line(buffer);

	//clear buffer as everything is now in the heap
    for (int i = 0; i < maxBuffer; i++) {
        buffer[i] = 0;
    }

    if (status < 0) {
        return status;
    }
    

    return process_job(job);
}
