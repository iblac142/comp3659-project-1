#include "mystring.h"
#include "jobs.h"
#include "myheap.h"
#include <unistd.h>
#include <string.h>

const char *prompt = "$ ";
const char *lengthError = "Messege exceeds max length of 256, please re enter command with shorter length\n";
const char *forkError = "Error occured while forking new proccess\n";
const char *execvpError = "Error occured while executing program\n";
const char *waitpidError = "Error occured while waiting for program\n"; 
const int maxBuffer = 256;

int run_command(struct Command* command){
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
    int pid;
    int status;

    //fork new proccess
    pid = fork();

    if (pid == -1) {
        write(1, forkError, 42);
        return -1;
    }

    if (pid == 0) {
        //in child
        execvp(command->argv[0], command->argv);
        write(1, execvpError, 39);
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
	/* Prompts user, collects command line into a buffer,
	
	clears the heap, then tokenizes the command line and
	
	fills in supplied command struct with the data
	
    Returns:
		0 if run successful
		-1 if error due to too many characters
        -2 if error due to too many arguments
	*/
	
	int readLength;
    int newToken = 0;
    int tokenCount = 0;
	char *buffer = alloc(256);
	
	//prompt and read input
	write(1, prompt, 2);
	readLength = read(0, buffer, maxBuffer);
	
	//check length
	if (readLength >= maxBuffer) {
		//display error message
		write(1, lengthError, 80);
		//discard rest of input not read by first read
		char discard;
		while (read(0, &discard, 1) == 1 && discard != '\n') {
		}
        return -1;
	}

	// when a non-whitespace character is reached after any number
    // of whitespace characters, place a pointer to that character
    // into command's argv.
	for (int i = 0; i < readLength; i += 1) {
        if ((buffer[i] != ' ') && newToken == 0) {
            command->argv[tokenCount] = buffer[i];
            newToken = 1;
            tokenCount += 1;
        }
        if ((buffer[i] == ' ') && newToken == 1) {
            newToken = 0;
        }
        if (tokenCount >= MAX_ARGS - 1) {
            return -2;
        }
    }

    // set argc of the command
    command->argc = tokenCount;

	//clear buffer
	free_all();

    return 0;
}

int main(int argc, char const *argv[]) {
    char buffer[maxBuffer];
    int readLength;
    int exit = 1;

    //loop untill user types "exit"
    while (exit != 0) {

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
            //read agian
            readLength = read(0, buffer, maxBuffer);
        }
        
        //check if input was "exit"
        exit = strcmp(buffer, "exit\n"); //TO-DO implemnt own string compare

        //echo input back
        write(1, buffer, readLength);

        //clear buffer
        for (int i = 0; i < readLength; i++) {
            buffer[i] = 0;
        }
    }
    return 0;
}
