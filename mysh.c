#include "mystring.h"
#include "jobs.h"
#include "myheap.h"
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[]) {
    /*
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
        */
    return 0;
}
