#include "getjob.h"
#include "myheap.h"
#include "mystring.h"
#include <unistd.h>

const int maxBuffer = 256;
const char *prompt = "$ ";
const char *lengthError = "Message exceeds max length of 256, please re-enter command with shorter length\n";
const char *argCountError = "Error while processing command: a command has too many arguments\n";
const char *pipeCountError = "Error while processing command: too many commands in pipeline\n";
const char *malCommandError = "Error while processing command: malformed input\n";

const char *cmdPath = "/usr/bin/";
const char *cmdExit = "/usr/bin/exit";

const struct Job clear = {0};

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
static int check_for(char n) {
    if (n == ' ' || n == '\t' || n == '\0') return 0;
    if (n == '|') return 1;
    if (n == '<') return 2;
    if (n == '>') return 3;
    if (n == '&') return 4;
    if (n == '\n') return 5;
    return -1;
}

/* Populates the commands of the supplied job structure until one of < > & or \n are encountered

job - the job structure to be populated

Returns:
    a positive value if run successful, equal to the position of the first 
        terminal symbol encountered, relative to the start of the heap
    -2 if a command has too many arguments
    -3 if the pipeline has too many commands in it
    -50 if an exit command is detected
*/
static int process_commands(struct Job* job) {
    char* heapStart = heap_start();
    int numArgs = 0;
    unsigned int numCommands = 0;
    int newToken = 0;
    int i = 0;
    // Check for exit command
    if (mystrcmp(heapStart, cmdExit) == 0) {
        return -50;
    }
    // Continue until first < or > or & or end of file
    while (check_for(heapStart[i]) < 2) {
        // Continue until | (i.e. end of command)
        while (check_for(heapStart[i]) < 1) {
            // Continue until null (i.e. end of token)
            while (check_for(heapStart[i]) < 0) {
                if (newToken == 0) {
                    // Set command argument
                    job->pipeline[numCommands].argv[numArgs] = &heapStart[i];
                    newToken = 1;
                }
                i += 1;
            }
            // Set up for next token to be added
            newToken = 0;
            numArgs += 1;

			// Detect too many arguments
            if (numArgs > MAX_ARGS) {
				write(1, argCountError, 65);
                return -2;
            }
			
			// Move i forward if it's not < > & \n
            if (check_for(heapStart[i]) < 2) {
                i += 1;
            }
        }
        // Set argc of pipeline and set up for next command
        job->pipeline[numCommands].argc = numArgs;
        newToken = 0;
        numArgs = 0;
        numCommands += 1;

		// Detect too many commands
        if (numCommands > MAX_PIPELINE_LEN) {
			write(1, pipeCountError, 62);
            return -3;
        }

		// Move i forward if it's not < > & \n
        if (check_for(heapStart[i]) < 2) {
            i += 1;
        }
    }
    job->num_stages = numCommands;
    return i;
}

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
static int process_job(struct Job* job) {
    char *heapPos = heap_start();
    int status;
    int setIn = 0;
    int setOut = 0;
    int setBack = 0;

    // Set default values first
    job->infile_path = NULL;
    job->outfile_path = NULL;
    job->background = 0;
    
    status = process_commands(job);
    if (status == -50) {
        return 1;
    } else if (status < 0) {
        return status;
    } else {
        heapPos += status;
    }
    while (check_for(*heapPos) < 5) {
        switch (check_for(*heapPos)) {
            // No more | should occur after the first < > &
            case 1:
				write(1, malCommandError, 48);
                return -4;
                break;
            // infile <
            // each field should only have one token each maximum
            case 2:
                if (setIn == 0) {
                    job->infile_path = heapPos + 1;
                    setIn = 1;
                } else {
					write(1, malCommandError, 48);
                    return -4;
                }
                break;
            // outfile >
            case 3:
                if (setOut == 0) {
                    job->outfile_path = heapPos + 1;
                    setOut = 1;
                } else {
					write(1, malCommandError, 48);
                    return -4;
                }
                break;
            // background &
            case 4:
                if (setBack == 0) {
                    job->background = 1;
                    setBack = 1;
                } else {
					write(1, malCommandError, 48);
                    return -4;
                }
                break;
            }
        heapPos += 1;
    }
    return 0;
}

/* Tokenizes the contents of the supplied buffer onto the heap,
removing excess whitespace, adding command path prefixes,
and null terminating each token

buffer - the beginning of the buffer to be tokenized

Returns:
    0 if run successful
    -4 if a malformed command is detected

*/
static int tokenize_line(char* buffer) {
    int i = 0;
    int newToken = 0;
    int startOfCommand = 0;
    char* n;
    while (check_for(buffer[i]) < 5) {
        // Non-whitespace, non-terminal characters written to heap normally
        if ((check_for(buffer[i]) < 0)) {
            // Mark the start of a new token if previous characters were special cases
            if (newToken < 1) {
                newToken = 1;
                // If this is the first argument of a command, prepend "/src/usr/"
                if (startOfCommand == 0) {
                    n = alloc(9);
                    mystrcpy(n, cmdPath);
                    startOfCommand = 1;
                }
            }
            n = alloc(1);
            n[0] = buffer[i];
        }
        else if (check_for(buffer[i]) > -1) {
            if (newToken == 1) {
                // null terminate the token
                n = alloc(1);
                n[0] = '\0';
                newToken = 0;
            }
            
            if (check_for(buffer[i]) > 0) {
                // If a terminal character has been reached without any token having
                 // been recorded (e.g. ||), the command is malformed
                if (startOfCommand == 0) {
					write(1, malCommandError, 48);
                    return -4;
                } else {
                    // If the symbol is | then this is a new command
                    if (check_for(buffer[i]) == 1) {
                        startOfCommand = 0;
                    }
                    // If the symbol is terminal, add it to the heap for later processing
                    if (check_for(buffer[i]) > 0) {
                        n = alloc(1);
                        n[0] = buffer[i];
                    }
                }
            }
        }
        i += 1;
    }

    // if there is not a final token and the command doesn't
    // end with &, the command is malformed
    if (newToken == 1) {
        // null terminate the final token if present
        n = alloc(1);
        n[0] = '\0';
        newToken = 0;
    } else {
        if (check_for(n[0]) != 0 && check_for(n[0]) != 4) {
			write(1, malCommandError, 48);
            return -4;
        }
    }
    // add a newline to the heap so final processing knows when to stop 
    n = alloc(1);
    n[0] = '\n';
    return 0;
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



