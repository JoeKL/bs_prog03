#define _POSIX_C_SOURCE 200809L //enable POSIX.1-2008 (and C99) functionalities

#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <setjmp.h>
#include <errno.h>

#define BUFFSIZE 512
#define FIFO_PATH "police"

// semaphore names for all 4 directions
const char *directions[] = {"/north", "/east", "/south", "/west"};

bool isBlocked = false;

pid_t pid;
sem_t *sem_src;
sem_t *sem_dest;

// the following two variables are used to check if the semaphore was already decremented by this process.
// its volatile and sig_atomic_t to encounter any unwanted behaviour if a signal is coming in the moment it changes.
volatile sig_atomic_t sem_src_decremented = 0;
volatile sig_atomic_t sem_dest_decremented = 0;

// direction with corresponding int
enum directions
{
    north, // 0
    east,  // 1
    south, // 2
    west   // 3
};

// converts the given string to corresponding int and fails if string isnt part of it
enum directions string_to_enum(const char *str)
{
    if (strcmp(str, "north") == 0)
        return north;
    if (strcmp(str, "east") == 0)
        return east;
    if (strcmp(str, "south") == 0)
        return south;
    if (strcmp(str, "west") == 0)
        return west;

    fprintf(stderr, "string_to_enum: Unknown Direction: %s\n", str);
    exit(EXIT_FAILURE);
}

//prints the current isBlocked status into fifo 
void messageIsBlocked()
{
    int fd = open(FIFO_PATH, O_WRONLY); // open fifo in write-only
    if (fd != -1){ // if open successfull
        if (isBlocked)
        {
            char message[] = "0"; // 0 == Blocked    
            if (write(fd, message, strlen(message) + 1) == -1) {
                perror("Error writing 'Blocked' status");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            char message[] = "1"; // 1 == Free
            if (write(fd, message, strlen(message) + 1) == -1) {
                perror("Error writing 'Free' status");        
                exit(EXIT_FAILURE);
            }
        }
        if (close(fd) != 0) { //close fifo with handling
            perror("close fifo failed");
            exit(EXIT_FAILURE);
        }
    } else { // handle error if not opened sucessfull
        perror("open fifo failed");
        exit(EXIT_FAILURE);
    }
}

// handles incoming signal and exits the program
void signalHandler(int signum)
{

    if (signum == SIGINT) { // if sigint -> Kill program
        printf("SIGINT signal received. Exiting...\n");
        
        // increment semaphores if they were decremented before.
        if (sem_src_decremented) {
            if (sem_post(sem_src) != 0) {
                perror("sem_post failed");
                exit(EXIT_FAILURE);
            }
        }
        if (sem_dest_decremented) {
            if (sem_post(sem_dest) != 0) {
                perror("sem_post failed");
                exit(EXIT_FAILURE);
            }
        }

        // close them 
        if (sem_close(sem_src) != 0) {
            perror("sem_close failed");
            exit(EXIT_FAILURE);
        }
        if (sem_close(sem_dest) != 0) {
            perror("sem_close failed");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }
    else if (signum == SIGUSR1) // else handle as status-message in Fifo
    {
        messageIsBlocked();
    }
    else if (signum == SIGALRM)
    {
        
        printf("Kaefer %i: Ich habe keine Gedult mehr zu warten. Ich kehre um!\n", pid);
    }
}

int main(int argc, char **argv)
{

    // declare a sigaction structure to specify how a signal should be handled.
    struct sigaction sa;  

    // init the sigaction structure to zero, so that all fields are set to default values.
    memset(&sa, 0, sizeof(sa));  

    // 'signalHandler' as the handler function for the signal.
    sa.sa_handler = signalHandler;  

    // flag to ensure system calls are automatically restarted if interrupted by this signal.
    // sa.sa_flags = SA_RESTART;  // this allows the interruption of sem_wait

    // Register the signal handler for SIGINT (Interrupt from keyboard, usually Ctrl+C).
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting handler for SIGINT");
        exit(EXIT_FAILURE);
    }

    // Register signal handler to write program status in FIFO for SIGUSR1.
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Error setting handler for SIGUSR1");
        exit(EXIT_FAILURE);
    }

    // Register the signal handler for SIGALRM
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("Error setting handler for SIGALRM");
        exit(EXIT_FAILURE);
    }

    // get pid
    pid = getpid();

    // set random seed to pid
    srand(pid);

    // if not exactly 3 arguments (progname, source, destination), return failure
    if (argc != 3)
    {
        perror("Bitte genau zwei Argumente angeben\n");
        exit(EXIT_FAILURE);
    }

    // convert src and dest
    enum directions car_src = string_to_enum(argv[1]);
    enum directions car_dest = string_to_enum(argv[2]);

    // if equal src and dest, return failure
    if (car_src == car_dest)
    {
        perror("Quelle und Ziel dürfen nicht gleich sein\n");
        exit(EXIT_FAILURE);
    }

    // open sem_src, return failure if fails
    sem_src = sem_open(directions[car_src], O_CREAT, 0666, 1);
    if (sem_src == SEM_FAILED)
    {
        perror("sem_open/sem_src");
        exit(EXIT_FAILURE);
    }

    // open sem_dest, return failure if fails
    sem_dest = sem_open(directions[car_dest], O_CREAT, 0666, 1);
    if (sem_dest == SEM_FAILED)
    {
        perror("sem_open/sem_dest");
        exit(EXIT_FAILURE);
    }

    while (1)
    {

        // Kaefer comes from source direction
        printf("Kaefer %i: ich komme von %s.\n", pid, argv[1]);

        // waits until source is free
        isBlocked = true;
        if (sem_wait(sem_src) != 0) {
            perror("sem_wait failed");
            exit(EXIT_FAILURE);
        }
        isBlocked = false;
        sem_src_decremented = 1;

        // when source direction is not blocked
        printf("Kaefer %i: ich stehe nun bei %s.\n", pid, argv[1]);

        // sleep some time
        usleep(1000000 - 100 * (rand() % 10));

        printf("Kaefer %i: ist %s frei?\n", pid, argv[2]);

        // waits until destination is free
        isBlocked = true;
        if (sem_wait(sem_dest) != 0) {
            if (errno == EINTR) {
                // sem_wait interrupted by SIGALRM-Signal

                // inc sem_src
                if (sem_post(sem_src) != 0) {
                    perror("sem_post failed");
                    exit(EXIT_FAILURE);
                }
                sem_src_decremented = 0;

                // unblock
                isBlocked = false;

                continue; // return to start of while loop
            } else {
                // Anderer Fehler
                perror("sem_wait(sem_dest) failed");
                exit(EXIT_FAILURE);
            }
        }
        isBlocked = false;
        sem_dest_decremented = 1;

        // if destination is free, drive
        printf("Kaefer %i: Es ist frei!\n", pid);
        printf("Kaefer %i: Ich fahre los nach %s.\n", pid, argv[2]);

        //sleep fixed amount of time (1 sec)
        sleep(1);

        // open up the source again, since its free now
        if (sem_post(sem_src) != 0) {
            perror("sem_post failed");
            exit(EXIT_FAILURE);
        }
        sem_src_decremented = 0;


        printf("Kaefer %i: ich bin angekommen in %s.\n", pid, argv[2]);

        // open up the destination again, since car has left the intersection
        if (sem_post(sem_dest) != 0) {
            perror("sem_post failed");
            exit(EXIT_FAILURE);
        }
        sem_dest_decremented = 0;
    }

    // close semaphores should ever be reached, just for good measure
    if (sem_close(sem_src) != 0) {
        perror("sem_close failed");
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_dest) != 0) {
        perror("sem_close failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}