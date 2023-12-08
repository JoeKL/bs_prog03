#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

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

// handles incoming signal and exits the program
void signalHandler()
{
    printf("SIGINT signal received. Exiting...\n");
    // Perform any cleanup operations here
    // sem_post(sem_src);
    // sem_post(sem_dest);

    // sem_close(sem_src);
    // sem_close(sem_dest);
    exit(0);
}

int main(int argc, char **argv)
{

    // start signal handler
    signal(SIGINT, signalHandler);

    // get pid
    pid_t pid = getpid();

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
    sem_t *sem_src = sem_open(directions[car_src], O_CREAT, 0666, 1);
    if (sem_src == SEM_FAILED)
    {
        perror("sem_open/sem_src");
        exit(EXIT_FAILURE);
    }

    // open sem_dest, return failure if fails
    sem_t *sem_dest = sem_open(directions[car_dest], O_CREAT, 0666, 1);
    if (sem_dest == SEM_FAILED)
    {
        perror("sem_open/sem_dest");
        exit(EXIT_FAILURE);
    }

    // Kaefer comes from source direction
    printf("Kaefer %i: ich komme von %s.\n", pid, argv[1]);

    sem_wait(sem_src);

    // when source direction is not blocked

    printf("Kaefer %i: ich stehe nun bei %s.\n", pid, argv[1]);

    // usleep(1000000 - 100 * (rand() % 10));

    sleep(3);
    printf("Kaefer %i: ist %s frei?\n", pid, argv[2]);

    //waits if destination is free

    sem_wait(sem_dest);

    // if free, drive
    printf("Kaefer %i: Es ist frei!\n", pid);
    printf("Kaefer %i: Ich fahre los nach %s.\n", pid, argv[2]);

    sleep(1);

    // open the source again, since its free now
    sem_post(sem_src);

    printf("Kaefer %i: ich bin angekommen in %s.\n", pid, argv[2]);

    //open the destination again, since car has left the intersection
    sem_post(sem_dest);

    // close semaphores
    sem_close(sem_src);
    sem_close(sem_dest);
    return 0;
}