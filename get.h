#ifndef GET_H
#define GET_H

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
#endif