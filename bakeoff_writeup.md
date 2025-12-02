# CIS 452 Project 2: Bake-Off 

## Project Overview

This project simulates a competitive bake-off in a shared kitchen using **threads**, **POSIX semaphores**, and **shared resources**. Each baker is represented as its own thread, and all bakers must complete five recipes: Cookies, Pancakes, Pizza Dough, Soft Pretzels, and Cinnamon Rolls. Since the bakers share the same kitchen, they must coordinate and compete for limited resources such as the pantry, refrigerator, mixers, bowls, spoons, and the oven.

Semaphores are used to enforce correct access to shared resources and prevent race conditions. Some resources must allow only one baker at a time (e.g., the pantry), while others allow multiple users (e.g., two mixers, three bowls). Each baker attempts to complete each recipe as quickly as possible while respecting these constraints.

Additionally, one randomly selected baker is chosen as the “**Ramsied**” baker. This baker must restart their current recipe once per execution, simulating being yelled at by Gordon Ramsay. This reset forces the baker to release resources and begin the recipe from the start, testing the robustness of the synchronization logic.

Colored console output is used to make each baker’s actions visually distinguishable and to show real-time concurrent activity.

---

## Implementation Summary

### 1. Initialization of Kitchen Resources

`init_kitchen()` sets up semaphores representing shared tools and spaces:

- **Mixer (2)**
- **Pantry (1)**
- **Refrigerator (2)**
- **Bowl (3)**
- **Spoon (5)**
- **Oven (1)**

A `print_lock` mutex is also initialized to prevent output overlap between threads.

---

### 2. Baker Threads

Each baker runs in its own thread via `pthread_create()` and executes the `baker_thread()` function.  
The baker:

1. Gets an ID (used to color code output).
2. Iterates through all five recipes.
3. Collects required ingredients via helper functions.
4. Mixes ingredients using bowl, spoon, and mixer semaphores.
5. Bakes the recipe using the oven semaphore.
6. Repeats until all recipes are completed.

All actions are printed using thread-safe locking.

---

### 3. Resource Access Functions

#### `get_from_pantry()`
- Uses `sem_wait(&pantry)` to ensure only one baker enters the pantry.
- Prints progress.
- Simulates access delay.
- Releases with `sem_post(&pantry)`.

#### `get_from_refrigerator()`
- Allows up to two bakers simultaneously.
- Prints action and simulates access delay.

---

### 4. Ramsied-Baker Mechanic

#### `check_if_ramsied()`
Determines if the current baker is the randomly chosen “Ramsied” baker.  
If so, and they have not already restarted:

- Prints a restart message.
- Signals that the recipe must restart.
- Forces the baker to re-attempt the recipe.

---

### 5. Mixing & Baking Stage

After gathering ingredients, the baker must acquire:

- **1 bowl**
- **1 spoon**
- **1 mixer**

Then the baker mixes ingredients, releases the resources, and must then acquire the **oven** to bake the recipe. After baking, the oven is released.

---

### 6. Program Flow in `main()`

- Initializes kitchen semaphores and mutex.
- Prompts user for number of bakers.
- Randomly selects the Ramsied baker.
- Creates one thread per baker.
- Waits for all threads to complete using `pthread_join()`.