#include <stdio.h> 
#include <semaphore.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

const char *directions[] = {"/north", "/east", "/south", "/west"};

void initialize_semaphores(sem_t *semaphores[]) {
    for (int i = 0; i < 4; i++) {
        semaphores[i] = sem_open(directions[i], O_CREAT, 0644, 1);
        if (semaphores[i] == SEM_FAILED) {
            perror("sem_open Fehler");
            exit(EXIT_FAILURE);
        }
    }
}

void close_semaphores(sem_t *semaphores[]) {
    for (int i = 0; i < 4; i++) {
        if (sem_close(semaphores[i]) != 0) {
            perror("sem_close Fehler");
            exit(EXIT_FAILURE);
        }
    }
}

void  unlink_semaphores() {
    for (int i = 0; i < 4; i++) {
        sem_unlink(directions[i]);
    }
}


int main(){
    sem_t *semaphores[4];
    unlink_semaphores();
    initialize_semaphores(semaphores);
    close_semaphores(semaphores);
    return 0;
}