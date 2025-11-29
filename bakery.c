#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#define num_recipes 5

#define RESET "\033[0m"

#define COOKIES 0
#define PANCAKES 1
#define DOUGH 2
#define PRETZELS 3
#define CINN_ROLLS 4

sem_t mixer, pantry, refrigerator, bowl, spoon, oven;

const char* colors[]= {"\033[1;31m", "\033[1;32m", "\033[1;34m", "\033[1;35m", "\033[1;36m"};

char* recipes[] = {
    "Cookies",
    "Pancakes",
    "Pizza Dough",
    "Soft Pretzels",
    "Cinnamon Rolls"
};

int ramsied_baker;
pthread_mutex_t print_lock;

//Initialize semaphores for kitchen resources
void init_kitchen() {
    sem_init(&mixer, 0, 2);
    sem_init(&pantry, 0, 1);
    sem_init(&refrigerator, 0, 2);
    sem_init(&bowl, 0, 3);
    sem_init(&spoon, 0, 5);
    sem_init(&oven, 0, 1);
}

//Function prototypes
void get_from_pantry(const char* color, int id, const char* item);
void get_from_refrigerator(const char* color, int id, const char* item);
int check_if_ramsied(int id, int* restarted, const char* color);

void* baker_thread(void* arg){
    //Give each baker an id and separate color
    int id = (intptr_t)arg;  
    const char* color = colors[id % 5];
    
    //determine which recipe to start
    for (int recipe = 0; recipe < num_recipes; recipe++) {
        int restarted = 0;

    //If ramsied, thread will restart here
    restart_recipe:
        pthread_mutex_lock(&print_lock);
        printf("%sBaker %d is starting recipe: %s.%s\n", color, id, recipes[recipe], RESET);
        pthread_mutex_unlock(&print_lock);

        switch(recipe) {
            case COOKIES:
                get_from_pantry(color, id, "Flour");
                if (check_if_ramsied(id, &restarted, color)) goto restart_recipe;
                get_from_pantry(color, id, "Sugar");
                get_from_pantry(color, id, "Sugar");
                get_from_refrigerator(color, id, "Milk");
                get_from_refrigerator(color, id, "Butter");
                break;

            case PANCAKES:
                get_from_pantry(color, id, "Flour");
                get_from_pantry(color, id, "Sugar");
                get_from_pantry(color, id, "Baking Soda");
                get_from_refrigerator(color, id, "Egg");
                get_from_refrigerator(color, id, "Milk");
                get_from_refrigerator(color, id, "Butter");
                get_from_pantry(color, id, "Salt");
                break;

            case DOUGH:
                get_from_pantry(color, id, "Yeast");
                get_from_pantry(color, id, "Sugar");
                get_from_pantry(color, id, "Salt");
                break;

            case PRETZELS:
                get_from_pantry(color, id, "Flour");
                get_from_pantry(color, id, "Sugar");
                get_from_pantry(color, id, "Yeast");
                get_from_pantry(color, id, "Salt");
                get_from_pantry(color, id, "Baking Soda");
                get_from_refrigerator(color, id, "Egg");
                break;

            case CINN_ROLLS:
                get_from_pantry(color, id, "Flour");
                get_from_pantry(color, id, "Sugar");
                get_from_pantry(color, id, "Salt");
                get_from_refrigerator(color, id, "Butter");
                get_from_refrigerator(color, id, "Egg");
                get_from_pantry(color, id, "Cinnamon");
                break;
        }

        if (check_if_ramsied(id, &restarted, color)) goto restart_recipe;

        //Mixing portion to prepare ingredients for the "oven"
        sem_wait(&bowl);
        sem_wait(&spoon);
        sem_wait(&mixer);

        pthread_mutex_lock(&print_lock);
        printf("%sBaker %d is mixing the ingredients for %s.%s\n", color, id, recipes[recipe], RESET);
        pthread_mutex_unlock(&print_lock);
        usleep(300000);

        sem_post(&mixer);
        sem_post(&spoon);
        sem_post(&bowl);

        //Cooking portion
        sem_wait(&oven);
        pthread_mutex_lock(&print_lock);
        printf("%sBaker %d is baking %s in the oven.%s\n", color, id, recipes[recipe], RESET);
        pthread_mutex_unlock(&print_lock);
        sleep(1);
        sem_post(&oven);

        //Once finished cooking the recipe
        pthread_mutex_lock(&print_lock);
        printf("%sBaker %d has finished recipe: %s.%s\n", color, id, recipes[recipe], RESET);
        pthread_mutex_unlock(&print_lock);
    }

    //Once a baker finishes all of the recipes
    pthread_mutex_lock(&print_lock);
    printf("%sBaker %d has completed all recipes!%s\n", color, id, RESET);
    pthread_mutex_unlock(&print_lock);
    return NULL;
}


//get an item from the pantry
void get_from_pantry(const char* color, int id, const char* item) {
    sem_wait(&pantry);
    pthread_mutex_lock(&print_lock);
    printf("%sBaker %d is getting %s from the pantry.%s\n", color, id, item, RESET);
    pthread_mutex_unlock(&print_lock);
    usleep(200000);
    sem_post(&pantry);
}

//get an item from the refridgerator
void get_from_refrigerator(const char* color, int id, const char* item) {
    sem_wait(&refrigerator);
    pthread_mutex_lock(&print_lock);
    printf("%sBaker %d is getting %s from the refrigerator.%s\n", color, id, item, RESET);
    pthread_mutex_unlock(&print_lock);
    usleep(200000);
    sem_post(&refrigerator);
}

//Check each baker to see if they are the ramsied baker
int check_if_ramsied(int id, int* restarted, const char* color) {
    if (id == ramsied_baker && !*restarted) {
        pthread_mutex_lock(&print_lock);
        printf("%sBaker %d has been ramsied! Restarting recipe.%s\n", color, id, RESET);
        pthread_mutex_unlock(&print_lock);
        *restarted = 1;
        return 1;
    }
    return 0;
}



int main() {
    //ensure each run is randomized
    srand(time(NULL));

    //set up the kitchen and ingredients
    init_kitchen();
    pthread_mutex_init(&print_lock, NULL);

    //determine number of bakers
    int n;
    printf("Enter number of bakers: ");
    scanf("%d", &n);

    ramsied_baker = rand() % n;

    pthread_t threads[n];
    int ids[n];

    //create thread of each baker, start the baker_thread function for each
    for (int i = 0; i < n; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, baker_thread, (void*)(intptr_t)i);
    }

    //loop through baker threads and wait for them to finish, ensures the main thread waits for all bakers to complete
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}