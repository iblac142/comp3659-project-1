#include "mystring.h"
#include "jobs.h"
#include "myheap.h"
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[]) {
    int exitRequested = 0;
    struct Job currentJob;



    if (get_job(&currentJob) == 1) {
        exitRequested = 1;
    }

    while (!exitRequested) {
        run_job(&currentJob);

        if (get_job(&currentJob) == 1) {
            exitRequested = 1;
        }
    }

    return 0;
}
