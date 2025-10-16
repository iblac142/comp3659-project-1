#include "jobs.h"
#include "myheap.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h> //remove when done testing

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

int run_job(struct Job* job) {
    int in = 0;
    int out = 0;
    int numberOfPipes = job->num_stages - 1;
    int pipes[numberOfPipes][2];
    pid_t pids[job->num_stages];
    int status;

    if (job->num_stages == 1) {
        if (job->infile_path != NULL) {
            in = open(job->infile_path, O_RDONLY);
            if (in == -1) {
                write(1, inOpenError, 35);
                return -1;
            }
        }
        if (job->outfile_path != NULL) {
            out = open(job->outfile_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (out == -1) {
                write(1, outOpenError, 36);
                return -2;
            }
        }
        run_command(&job->pipeline[0], in, out, 1);

        if (in != 0) close(in);
        if (out != 0) close(out);
    }
    else { //multi stage pipeline
        //create pipes
        for (int i = 0; i < numberOfPipes; i++) {
            if (pipe(pipes[i]) == -1) {
                write(1, pipeError, 27);
                //close already created pipes
                for (int j = 0; j < i; j++) {
                    close(pipes[j][PIPE_READ_END]);
                    close(pipes[j][PIPE_WRITE_END]);
                }
                return -3;
            }
        }
        
        //loop through each command and run (cant wait for child commands until whole job is running)
        for (int i = 0; i < job->num_stages; i++) {
            pids[i] = fork();

            if (pids[i] == 0) { //in child

                if (i == 0) { //first command in pipeline
                    if (job->infile_path != NULL) {
                        in = open(job->infile_path, O_RDONLY);
                        if (in == -1) {
                            write(1, inOpenError, 35);
                            _exit(1);
                        }
                    }

                    //out is first pipe write
                    out = pipes[0][PIPE_WRITE_END];

                    //close all other pipes
                    close(pipes[0][PIPE_READ_END]);
                    
                    for (int j = 1; j < numberOfPipes; j++) {
                        close(pipes[j][PIPE_READ_END]);
                        close(pipes[j][PIPE_WRITE_END]);
                    }

                    run_command_no_fork(&job->pipeline[i], in, out);
                }
                else if (i == job->num_stages - 1) { //last command in pipeline
                    if (job->outfile_path != NULL) {
                        out = open(job->outfile_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                        if (out == -1) {
                            write(1, outOpenError, 36);
                            _exit(2);
                        }
                    }
                    //in is last pipe read
                    in = pipes[numberOfPipes - 1][PIPE_READ_END];
                    
                    //close all other pipes
                    close(pipes[numberOfPipes - 1][PIPE_WRITE_END]);
                    
                    for (int j = 0; j < numberOfPipes - 1; j++) {
                        close(pipes[j][PIPE_READ_END]);
                        close(pipes[j][PIPE_WRITE_END]);
                    }
                    
                    run_command_no_fork(&job->pipeline[i], in, out);
                }
                else { //all commands between first and last
                    //in-between commands both i/o pipes
                    in = pipes[i - 1][PIPE_READ_END];
                    out = pipes[i][PIPE_WRITE_END];

                    //close all other pipes
                    for (int j = 0; j < i - 1; j++) {
                        close(pipes[j][PIPE_READ_END]);
                        close(pipes[j][PIPE_WRITE_END]);
                    }

                    close(pipes[i - 1][PIPE_WRITE_END]);
                    close(pipes[i][PIPE_READ_END]);

                    for (int j = i + 1; j < numberOfPipes; j++) {
                        close(pipes[j][PIPE_READ_END]);
                        close(pipes[j][PIPE_WRITE_END]);
                    }

                    run_command_no_fork(&job->pipeline[i], in, out);
                }
            }
            else if (pids[i] < 0) { // fork failed in parent
                write(1, forkError, 41);
                // close all pipes
                for (int j = 0; j < numberOfPipes; j++) {
                    close(pipes[j][PIPE_READ_END]);
                    close(pipes[j][PIPE_WRITE_END]);
                }
                // wait for already started children to avoid zombies
                for (int k = 0; k < i; k++) {
                    waitpid(pids[k], &status, 0);
                }
                return -1;
            }
            else {
                //in parent (shell)
                if (i > 0) { //close pipe with both ends already in use
                    close(pipes[i - 1][PIPE_READ_END]);
                    close(pipes[i - 1][PIPE_WRITE_END]);
                }
            }

        }

        for (int i = 0; i < job->num_stages; i++) {
            pids[i] = waitpid(pids[i], &status, 0);
            if (pids[i] == -1) {
                write(1, waitpidError, 41);
                return -4;
            }
        }
    }
    return 0;
}

int run_command(struct Command* command, int infile, int outfile, int wait){
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
                return -3;
            }
        }
        
        return 0;
    }
}

int run_command_no_fork(struct Command* command, int infile, int outfile) {
    // This function runs a command without forking (used in pipeline children)
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

int get_command(struct Command* command) {
	char buffer[maxBuffer];
    int readLength;

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
        for (int i = 0; i < readLength; i++) {
            buffer[i] = 0;
        }
        //return
        return -1;
    }

    // clear the heap
    clear_heap();

    // when a non-whitespace character is reached after any number
    // of whitespace characters, place a pointer to that character
    // into command's argv.
    int i = 0;
    int newToken = 0;
    int tokenCount = 0;
    char tokenizedLine = alloc(512);

	while (buffer[i] != '\n') {
        if ((buffer[i] != ' ')) {
            if (newToken == 0) {
                command->argv[tokenCount] = tokenizedLine;
                newToken = 1;
                tokenCount += 1;
            }
            tokenizedLine = buffer[i];
            tokenizedLine += 1;
        }
        if ((buffer[i] == ' ') && newToken == 1) {
            newToken = 0;
            tokenizedLine = NULL;
            tokenizedLine += 1;
        }
        if (tokenCount >= MAX_ARGS - 1) {
            return -2;
        }
        i += 1;
    }

    // set argc of the command
    command->argc = tokenCount;

    //clear buffer
    for (int i = 0; i < readLength; i++) {
        buffer[i] = 0;
    }
    return 0;
}