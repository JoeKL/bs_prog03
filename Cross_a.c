#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

// https://www.youtube.com/watch?v=ukM_zzrIeXs

const char *directions[] = {"/north", "/east", "/south", "/west"};

enum directions
{
    north, // 0
    east,  // 1
    south, // 2
    west   // 3
};

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

    signal(SIGINT, signalHandler);

    pid_t pid = getpid();
    srand(pid);

    if (argc != 3)
    {
        perror("Bitte genau zwei Argumente angeben\n");
        exit(EXIT_FAILURE);
    }

    enum directions car_src = string_to_enum(argv[1]);
    enum directions car_dest = string_to_enum(argv[2]);

    if (car_src == car_dest)
    {
        perror("Quelle und Ziel dürfen nicht gleich sein\n");
        exit(EXIT_FAILURE);
    }

    sem_t *sem_src = sem_open(directions[car_src], O_CREAT, 0666, 1);
    if (sem_src == SEM_FAILED)
    {
        perror("sem_open/sem_src");
        exit(EXIT_FAILURE);
    }

    sem_t *sem_dest = sem_open(directions[car_dest], O_CREAT, 0666, 1);
    if (sem_dest == SEM_FAILED)
    {
        perror("sem_open/sem_dest");
        exit(EXIT_FAILURE);
    }

    printf("Kaefer %i: ich komme von %s.\n", pid, argv[1]);

    sem_wait(sem_src);

    printf("Kaefer %i: ich stehe nun bei %s.\n", pid, argv[1]);
    // usleep(1000000 - 100 * (rand() % 10));
    sleep(3);
    printf("Kaefer %i: ist %s frei?\n", pid, argv[2]);

    sem_wait(sem_dest);

    printf("Kaefer %i: Es ist frei!\n", pid);
    printf("Kaefer %i: Ich fahre los nach %s.\n", pid, argv[2]);

    sleep(1);

    sem_post(sem_src);

    printf("Kaefer %i: ich bin angekommen in %s.\n", pid, argv[2]);

    sem_post(sem_dest);

    sem_close(sem_src);
    sem_close(sem_dest);
    return 0;
}