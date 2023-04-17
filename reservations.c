#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int seat_count;                                     // Total number of seats
int broker_count;                                   // Total number of brokers
int *seat_taken;                                    // Array of seats
int transaction_count;                              // Number of transactions per broker

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;   // Mutex lock for thread sync

int seat_taken_count = 0;                           // Number of seats that are taken

// Reserve a seat
int reserve_seat(int n) {
    pthread_mutex_lock(&lock);                      // Lock the mutex to ensure exclusive access

    int return_value = -1;

    if (!seat_taken[n]) {                           // If the seat is not taken
        seat_taken[n] = 1;                          // Mark the seat as taken
        ++seat_taken_count;                         // Increment the seat taken count
        return_value = 0;                           // Return 0 for a successful reservation
    }

    pthread_mutex_unlock(&lock);                    // Unlock the mutex

    return return_value;
}

// Free a seat
int free_seat(int n) {
    pthread_mutex_lock(&lock);                      // Lock the mutex to ensure exclusive access

    int return_value = -1;

    if (seat_taken[n]) {                            // If the seat is taken
        seat_taken[n] = 0;                          // Mark the seat as free
        --seat_taken_count;                         // Decrement the seat taken count
        return_value = 0;                           // Return 0 to indicate successful freeing of the seat
    }

    pthread_mutex_unlock(&lock);                    // Unlock the mutex

    return return_value;
}

// Check if a seat is free
int is_free(int n) {
    pthread_mutex_lock(&lock);                      // Lock the mutex to ensure exclusive access

    int return_value = seat_taken[n] ? 0 : 1;       // Return 0 if seat is taken, 1 if seat is free

    pthread_mutex_unlock(&lock);                    // Unlock the mutex

    return return_value;
}

// Verify if the seat count matches the seat taken count
int verify_seat_count(void) {
    pthread_mutex_lock(&lock);                      // Lock the mutex to ensure exclusive access

    int count = 0;

    for (int i = 0; i < seat_count; i++)
        count += seat_taken[i];                     // Count the number of seats that are taken

    int return_value = (count == seat_taken_count); // Compare the count with seat_taken_count

    pthread_mutex_unlock(&lock);                    // Unlock the mutex

    return return_value;
}

// Executed by each broker thread
void *seat_broker(void *arg)
{
    int *id = arg;

    for (int i = 0; i < transaction_count; i++) {
        int seat = rand() % seat_count;
        if (rand() & 1) {
            reserve_seat(seat);
        } else {
            free_seat(seat);
        }

        if (!verify_seat_count()) {
            printf("Broker %d: the seat count seems to be off! " \
                   "I quit!\n", *id);
            return NULL;
        }
    }

    printf("Broker %d: That all seemed to work very well.\n", *id);

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "usage: reservations seat_count broker_count xaction_count\n");
        exit(1);
    }

    seat_count = atoi(argv[1]);
    broker_count = atoi(argv[2]);
    transaction_count = atoi(argv[3]);

    seat_taken = calloc(seat_count, sizeof *seat_taken);

    pthread_t *thread = calloc(broker_count, sizeof *thread);

    int *thread_id = calloc(broker_count, sizeof *thread_id);

    srand(time(NULL) + getpid());
    
    for (int i = 0; i < broker_count; i++) {
        thread_id[i] = i;
        pthread_create(thread + i, NULL, seat_broker, thread_id + i);
    }

    for (int i = 0; i < broker_count; i++)
        pthread_join(thread[i], NULL);
}
