#include "jobs.h"
#include "getjob.h"
#include "runjob.h"

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
        
        check_for_zombies();

        status = get_job(&currentJob);
        if (status == 1) {
            exitRequested = 1;
        }
    }

    return 0;
}
