#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#define BUFFSIZE 512
#define FIFO_PATH "police"

int main(int argc, char const *argv[])
{

    if (argc != 5) {
        perror("Incorrect number of arguments!\n");
        printf("required 5, recieved %i\n", argc);
        exit(EXIT_FAILURE);
    }


    int fd;
    char buffer[BUFFSIZE];

    int kaefer_count = argc - 1;
    pid_t kaefer_pids[kaefer_count];

    //copy each argv > 0 and check if argument is integer
    for (int i = 0; i < 4; i++) {
        char *endptr;
        kaefer_pids[i] = strtol(argv[i + 1], &endptr, 10);
        
        // Check if conversion was successful
        if (*endptr != '\0') {
            perror("Argument is not an integer!\n");
            exit(EXIT_FAILURE);
        }
    }


    // open fifo
    fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK); // Open the FIFO for reading only, without blocking the caller if no data is available.
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }



    while (1) {
        // send SIGUSR1 to kaefer
        for (int i = 0; i <= kaefer_count; i++) {
            // com-channel to get status of kaefer
            kill(kaefer_pids[i], SIGUSR1);
        }

        sleep(5);

        // count zeros in buffer
        int blockedCount = 0;
        for (int i = 0; i < kaefer_count; i++) {
            if (buffer[i] == 0) {
                blockedCount++;  // 
            }
        }

        // check if deadlock
        // if  blockedCount == kaefer_count, all kaefer are waiting => deadlock
        if (blockedCount == kaefer_count) {
            printf("DEADLOCK\n");
        }


    }

    close(fd);
    return 0;
}

