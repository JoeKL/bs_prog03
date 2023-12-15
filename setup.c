#include <stdio.h> 
#include <semaphore.h> 
#include <stdlib.h>
#include <fcntl.h>

#define SEM_COUNT 4

//the following 4 semaphore names are used
const char *directions[] = {"/north", "/east", "/south", "/west"};


// initializes sempahores, handles errors if something goes wrong
void initialize_semaphores(sem_t *semaphores[]) {
    for (int i = 0; i < SEM_COUNT; i++) {
        semaphores[i] = sem_open(directions[i], O_CREAT, 0644, 1);
        if (semaphores[i] == SEM_FAILED) {
            perror("sem_open error");
            exit(EXIT_FAILURE);
        }
    }
}

// closes sempahores, handles errors if something goes wrong
void close_semaphores(sem_t *semaphores[]) {
    for (int i = 0; i < SEM_COUNT; i++) {
        if (sem_close(semaphores[i]) != 0) {
            perror("sem_close error");
            exit(EXIT_FAILURE);
        }
    }
}

// deletes sempahores, doesnt handle errors if something goes wrong
void  unlink_semaphores() {
    for (int i = 0; i < SEM_COUNT; i++) {
        sem_unlink(directions[i]);
        // sem_unlink error ignored: "Dabei sollten Sie Fehler ignorieren, damit das Programm auch läuft, wenn es noch keine Semaphoren gibt."
        // if (sem_unlink(directions[i]) != 0) {
        //     perror("sem_unlink error");
        //     exit(EXIT_FAILURE);
        // }
    }
}


int main(){
    sem_t *semaphores[SEM_COUNT];
    unlink_semaphores();
    initialize_semaphores(semaphores);
    close_semaphores(semaphores);
    return 0;
}