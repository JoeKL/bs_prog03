#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <setjmp.h>

#define BUFFSIZE 512
#define FIFO_PATH "police"

bool isBlocked = false;
static sigjmp_buf jmpEnv;

pid_t pid;
sem_t *sem_src;
sem_t *sem_dest;


// semaphores for all 4 directions
const char *directions[] = {"/north", "/east", "/south", "/west"};

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

    fprintf(stderr, "Ungültige Richtung: %s\n", str);
    exit(1);
}

void messageIsBlocked()
{

    int fd = open(FIFO_PATH, O_WRONLY); // Fifo in Write only öffnen
    if (fd != -1)
    { // wenn geöffnet werden konnte
        if (isBlocked)
        {
            char message[] = "0"; // 0 == Blocked
            write(fd, message, strlen(message) + 1);
        }
        else
        {
            char message[] = "1"; // 1 == Free
            write(fd, message, strlen(message) + 1);
        }
        close(fd);
    }
    else
    {
        perror("Konnte FIFO nicht öffnen.");
        exit(EXIT_FAILURE);
    }
}

// handles incoming signal and exits the program
void signalHandler(int signum)
{

    if (signum == SIGINT)
    { // if sigint -> Kill program
        printf("SIGINT signal received. Exiting...\n");
        exit(0);
    }
    else if (signum == SIGUSR1) // else handle as status-message in Fifo
    {
        messageIsBlocked();
    }
    else if (signum == SIGALRM)
    {
        printf("Kaefer %i: Ich habe keine Gedult mehr zu warten . Ich kehre um!\n", pid);
        sem_post(sem_src);
        sem_post(sem_dest);
        siglongjmp(jmpEnv, 1); // Springt zum Punkt zurück, der mit sigsetjmp gesetzt wurde
    }
}

int main(int argc, char **argv)
{

    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = signalHandler;
    sa.sa_flags = SA_RESTART; // dont interrupt syscalls

    sigaction(SIGINT, &sa, NULL);  // Kill Signal to stop programm
    sigaction(SIGUSR1, &sa, NULL); // Kill signal to write programm status in Fifo
    sigaction(SIGALRM, &sa, NULL); // Kill signal to restart prog

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
        sigsetjmp(jmpEnv, 1);
        // Kaefer comes from source direction
        printf("Kaefer %i: ich komme von %s.\n", pid, argv[1]);
        isBlocked = true;
        sem_wait(sem_src);
        isBlocked = false;
        // when source direction is not blocked

        printf("Kaefer %i: ich stehe nun bei %s.\n", pid, argv[1]);

        // usleep(1000000 - 100 * (rand() % 10));
        sleep(3);

        printf("Kaefer %i: ist %s frei?\n", pid, argv[2]);

        // waits if destination isnt free
        isBlocked = true;
        sem_wait(sem_dest);
        isBlocked = false;

        // if free, drive
        printf("Kaefer %i: Es ist frei!\n", pid);
        printf("Kaefer %i: Ich fahre los nach %s.\n", pid, argv[2]);

        sleep(1);

        // open the source again, since its free now
        sem_post(sem_src);

        printf("Kaefer %i: ich bin angekommen in %s.\n", pid, argv[2]);

        // open the destination again, since car has left the intersection
        sem_post(sem_dest);
    }

    // close semaphores
    sem_close(sem_src);
    sem_close(sem_dest);
    return 0;
}