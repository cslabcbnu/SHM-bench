#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <numa.h>
#include <sched.h>

char *shm;
long long shm_size;

void *operations(void *arg) {
    int thread_id = *(int *)arg;
    int num_nodes = numa_num_configured_nodes();
    int node_id = thread_id % num_nodes;


    struct bitmask *cpus = numa_allocate_cpumask();
    if (numa_node_to_cpus(node_id, cpus) == 0) {
        for (int cpu = 0; cpu < cpus->size; cpu++) {
            if (numa_bitmask_isbitset(cpus, cpu)) {
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(cpu, &cpuset);
                pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
                break;
            }
        }
    }
    numa_free_cpumask(cpus);


    numa_run_on_node(node_id);

    for (long long i = 0; i < shm_size; i++) {
        shm[i] = i;
    }

    for (long long i = 0; i < shm_size - 1; i++) {
        shm[i] = shm[i + 1];
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s --size <size_in_mb> --threads <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int pid = getpid();
    char cmd[256];

    int size_in_mb = atoi(argv[2]);
    int num_threads = atoi(argv[4]);

    shm_size = size_in_mb * 1024LL * 1024LL;
    printf("Shared memory size: %d MB (%lld bytes)\n", size_in_mb, shm_size);

    shm = calloc(shm_size, 1);
    if (shm == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }


    printf("\n[Before workload]\n");
    snprintf(cmd, sizeof(cmd), "numastat -p %d", pid);
    system(cmd);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    int *thread_ids = malloc(sizeof(int) * num_threads);

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, operations, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double exec_time = (end_time.tv_sec - start_time.tv_sec) +
                       (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("\nTotal execution time: %.3f seconds\n", exec_time);


    printf("\n[After workload]\n");
    system(cmd);


    free(threads);
    free(thread_ids);
    free(shm);

    return 0;
}
