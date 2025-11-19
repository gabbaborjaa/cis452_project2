#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define num_recipes 5

#define RESET "\033[0m"

sem_t mixer, pantry, refrigerator, bowl, spoon, oven;

const char* colors[]= {"\033[1;31m", "\033[1;32m", "\033[1;34m", "\033[1;35m", "\033[1;36m"};


void init_kitchen() {
    sem_init(&mixer, 0, 2);
    sem_init(&pantry, 0, 1);
    sem_init(&refrigerator, 0, 2);
    sem_init(&bowl, 0, 3);
    sem_init(&spoon, 0, 5);
    sem_init(&oven, 0, 1);
}

void* baker_thread(void* arg){
    int id = *(int*)arg;
    const char* color = colors[id % 5];
    for (int r = 0; r < num_recipes; r++){
        printf("");

        sem_wait(&pantry);
        printf("%sBaker %d is in the pantry.%s\n", color, id, RESET);
        sleep(1);
        sem_post(&pantry);

        sem_wait(&refrigerator);
        printf("%sBaker %d is in the refrigerator.%s\n", color, id, RESET);
        sleep(1);
        sem_post(&refrigerator);

        sem_wait(&bowl);
        sem_wait(&spoon);
        sem_wait(&mixer);

        printf("%sBaker %d is mixing ingredeients");


    }


    return NULL;
}