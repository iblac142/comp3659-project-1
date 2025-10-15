#include "jobs.h"
#include "myheap.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

const int maxBuffer = 256;
const char *prompt = "$ ";
const char *lengthError = "Messege exceeds max length of 256, please re enter command with shorter length\n";
const char *forkError = "Error occured while forking new proccess\n";
const char *execveError = "Error occured while executing program\n";
const char *waitpidError = "Error occured while waiting for program\n"; 

int run_command(struct Command* command){
    pid_t pid;
    int status;

    //fork new proccess
    pid = fork();

    if (pid == -1) {
        write(1, forkError, 42);
        return -1;
    }

    if (pid == 0) {
        //in child
        execve(command->argv[0], command->argv, NULL);
        write(1, execveError, 39);
        return -2;
    }
    if (pid != 0) {
        //in parent
        pid = waitpid(pid, &status, 0);
        if (pid == -1) {
            write(1, waitpidError, 41);
            return -3;
        }
        return 0;
    }
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