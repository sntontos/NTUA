#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

// Function to compare integers in descending order
typedef struct {
    int diff;
    int index;
} Worker;

int compare_workers_desc(const void *a, const void *b) {
    const Worker *w1 = (const Worker *)a;
    const Worker *w2 = (const Worker *)b;
    return w2->diff - w1->diff;  // Descending order
}

bool load_data(const char *filename, int **M_ptr, int **K_ptr, int *m_ptr, int *k_ptr, int *count_ptr) {
    size_t len = strlen(filename);
    if (len < 4 || strcmp(filename + len - 4, ".txt") != 0) {
        fprintf(stderr, "Error: File must have a .txt extension\n");
        return false;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return false;
    }

    int m, k;
    if (fscanf(file, "%d %d", &m, &k) != 2 || m < 0 || k < 0) {
        fprintf(stderr, "Invalid first line. Expected two non-negative integers.\n");
        fclose(file);
        return false;
    }

    int capacity = m + k;
    int *M = malloc(capacity * sizeof(int));
    int *K = malloc(capacity * sizeof(int));
    if (!M || !K) {
        perror("Memory allocation failed");
        fclose(file);
        free(M);
        free(K);
        return false;
    }

    int a, b, count = 0;
    while (fscanf(file, "%d %d", &a, &b) == 2) {
        if (count >= capacity) break;
        M[count] = a;
        K[count] = b;
        count++;
    }

    fclose(file);
    *M_ptr = M;
    *K_ptr = K;
    *m_ptr = m;
    *k_ptr = k;
    *count_ptr = count;
    return true;
}

int find_max(int *M, int *K, int m, int k) //M and K are arrays of integers and are sorted in descending order
{   
    int best_sum = 0;
    int i = 0;
    Worker *workers = malloc((m + k) * sizeof(Worker));
    if (workers == NULL) {
        perror("Memory allocation failed");
        return -1;
    }
    for (i = 0; i < m + k; i++) {
        workers[i].diff = M[i] - K[i];
        workers[i].index = i;
    }
    qsort(workers, m + k, sizeof(Worker), compare_workers_desc);

    for(int i = 0; i < m + k; i++)
    {
        if (i < m)
        {
            best_sum += M[workers[i].index];
        }
        else
        {
            best_sum += K[workers[i].index];
        }
    }
    free(workers);

    return best_sum;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        write(STDERR_FILENO, "Error: Missing or too many arguments.\n", 38);
        write(STDERR_FILENO, "Example: ./OS_first.c 5\n", 23);
        exit(0);
    }

    char *filename = argv[1];
    int *M, *K, m, k, count;
    if (!load_data(filename, &M, &K, &m, &k, &count)) {
        return 1;
    }

    // // Sort M and K in descending order
    // qsort(M, count, sizeof(int), compare_desc);
    // qsort(K, count, sizeof(int), compare_desc);


    printf("Best sum: %d\n", find_max(M, K, m, k));

    free(M);
    free(K);
    
    return 0;
}
