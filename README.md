# comp3659-project-1
 COMP 3659 Operating Systems Fall 2025

mysh is a custom shell program for Linux.

Inputs to the command line are to be formatted in the following manner:

```command1 arg1 ... argn | command2 arg1 | ... < infile > outfile &```

Where:
* commands are valid commands and arg are their respective arguments
* infile is where to redirect the initial input
* outfile is where to redirect the final output
* if & is present, the job will be run in the background

Requirements: 
* all < > & must be placed after every |
* there must be at most one instance each of < > &
* a token may not be left blank (e.g. `ls | > out.txt`)
* the command line must not exceed 256 characters
* each command must not exceed 64 arguments, including the inital command
* the pipeline must not exceed 64 commands, excluding the infile and outfile

Allowances:
* Any amount of whitespace greater than a single character is ignored
* < > & may be listed in any order
* additional tokens after < > are ignored (e.g. `ls > file location name &` runs as `ls > file &`)
