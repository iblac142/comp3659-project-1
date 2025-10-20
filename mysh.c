#include "mystring.h"
#include "jobs.h"
#include "myheap.h"
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[]) {
    int exitRequested = 0;
    int status = 0;
    struct Job currentJob;


    status = get_job(&currentJob);
    if (status == 1) {
        exitRequested = 1;
    }

    while (!exitRequested) {
        if (status == 0) {
            run_job(&currentJob);
        }
        status = get_job(&currentJob);
        if (status == 1) {
            exitRequested = 1;
        }
    }

    return 0;
}
