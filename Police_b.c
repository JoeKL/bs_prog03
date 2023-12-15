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

    //file descriptor
    int fd;
    char buffer[BUFFSIZE];

    int kaefer_count = argc - 1; // in our case its 4
    pid_t kaefer_pids[kaefer_count];

    //copy each argv > 0 and check if argument is integer
    for (int i = 0; i < kaefer_count; i++) {
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
        perror("open fifo failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // send SIGUSR1 to kaefer to get blockedstatus as response in fifo
        for (int i = 0; i < kaefer_count; i++) {
            // com-channel to get status of kaefer
            if (kill(kaefer_pids[i], SIGUSR1) == -1) {
                perror("Error sending SIGUSR1 to process");
                exit(EXIT_FAILURE);
            }
        }

        //sleep fixed amount of time (5 sec)
        sleep(5);
    
        // count zeros in buffer
        int blockedCount = 0;
        for (int i = 0; i < kaefer_count; i++) {
            if (buffer[i] == 0) {
                blockedCount++; 
            }
        }

        // check if deadlock
        // if  blockedCount == kaefer_count, all kaefer are waiting => deadlock
        if (blockedCount == kaefer_count) {
            printf("DEADLOCK\n");
        }
    }

    if (close(fd) != 0) {
        perror("close fifo failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}