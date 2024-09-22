#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

long long result = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int start;
    int end;
    int mod;
} ThreadArgs;

void* factorial_part(void* args) {
    ThreadArgs* data = (ThreadArgs*)args;
    long long partial_result = 1;
    
    for (int i = data->start; i <= data->end; i++) {
        partial_result = (partial_result * i) % data->mod;
    }

    pthread_mutex_lock(&mutex);
    result = (result * partial_result) % data->mod;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s k pnum mod\n", argv[0]);
        return 1;
    }

    int k = atoi(argv[1]);
    int pnum = atoi(argv[2]);
    int mod = atoi(argv[3]);

    pthread_t threads[pnum];
    ThreadArgs args[pnum];

    int range = k / pnum;
    for (int i = 0; i < pnum; i++) {
        args[i].start = i * range + 1;
        args[i].end = (i == pnum - 1) ? k : (i + 1) * range;
        args[i].mod = mod;

        if (pthread_create(&threads[i], NULL, factorial_part, &args[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Factorial %d! mod %d = %lld\n", k, mod, result);
    pthread_mutex_destroy(&mutex);

    return 0;
}
