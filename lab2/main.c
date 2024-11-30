#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ARRAY_LENGTH 10
#define MAX_ARRAYS 10

int K;
int max_threads;
int arrays[MAX_ARRAYS][ARRAY_LENGTH];
int result[ARRAY_LENGTH] = {0};
int active_threads = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;

void initialize_arrays() {
    srand(time(NULL));
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < ARRAY_LENGTH; j++) {
            arrays[i][j] = rand() % 10;
        }
    }
}

void print_arrays() {
    printf("Input arrays:\n");
    for (int i = 0; i < K; i++) {
        printf("Array %d: ", i + 1);
        for (int j = 0; j < ARRAY_LENGTH; j++) {
            printf("%d ", arrays[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_result() {
    printf("Result array:\n");
    for (int i = 0; i < ARRAY_LENGTH; i++) {
        printf("%d ", result[i]);
    }
    printf("\n\n");
}

void* sum_partial_arrays(void* arg) {
    int thread_id = *((int*)arg);
    free(arg);

    int start = (ARRAY_LENGTH / max_threads) * thread_id;
    int end = (thread_id == max_threads - 1) ? ARRAY_LENGTH : start + (ARRAY_LENGTH / max_threads);

    for (int i = start; i < end; i++) {
        for (int j = 0; j < K; j++) {
            result[i] += arrays[j][i];
        }
    }

    pthread_mutex_lock(&mutex);
    active_threads--;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void parallel_sum() {
    pthread_t threads[max_threads];

    for (int i = 0; i < max_threads; i++) {
        pthread_mutex_lock(&mutex);

        while (active_threads >= max_threads) {
            pthread_cond_wait(&cond, &mutex);
        }

        active_threads++;
        pthread_mutex_unlock(&mutex);

        int* thread_id = malloc(sizeof(int));
        *thread_id = i;
        if (pthread_create(&threads[i], NULL, sum_partial_arrays, thread_id) != 0) {
            perror("Failed to create thread");
            exit(1);
        }
    }

    for (int i = 0; i < max_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <max_threads> <num_arrays>\n", argv[0]);
        return 1;
    }

    max_threads = atoi(argv[1]);
    K = atoi(argv[2]);

    if (K > MAX_ARRAYS || K <= 0) {
        printf("Error: The number of arrays should be between 1 and %d\n", MAX_ARRAYS);
        return 1;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    initialize_arrays();

    print_arrays();

    clock_t start_time = clock();
    parallel_sum();
    clock_t end_time = clock();

    print_result();

    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", time_taken);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
