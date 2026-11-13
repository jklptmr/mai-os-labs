#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

typedef struct {
    double re;
    double im;
} complex_t;

typedef struct {
    int start_row;
    int end_row;
    int n;
    complex_t *A;
    complex_t *B;
    complex_t *C;
} thread_data_t;

double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

void fill_matrix(complex_t *M, int n) {
    for (int i = 0; i < n * n; i++) {
        M[i].re = (rand() % 200 - 100) / 10.0;
        M[i].im = (rand() % 200 - 100) / 10.0;
    }
}

void multiply_seq(int n, complex_t *A, complex_t *B, complex_t *C) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum_re = 0.0;
            double sum_im = 0.0;
            for (int k = 0; k < n; k++) {
                complex_t a = A[i * n + k];
                complex_t b = B[k * n + j];
                sum_re += a.re * b.re - a.im * b.im;
                sum_im += a.re * b.im + a.im * b.re;
            }
            C[i * n + j].re = sum_re;
            C[i * n + j].im = sum_im;
        }
    }
}

void *multiply_worker(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int n = data->n;

    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < n; j++) {
            double sum_re = 0.0;
            double sum_im = 0.0;
            for (int k = 0; k < n; k++) {
                complex_t a = data->A[i * n + k];
                complex_t b = data->B[k * n + j];
                sum_re += a.re * b.re - a.im * b.im;
                sum_im += a.re * b.im + a.im * b.re;
            }
            data->C[i * n + j].re = sum_re;
            data->C[i * n + j].im = sum_im;
        }
    }
    return NULL;
}

void multiply_par(int n, int num_threads, complex_t *A, complex_t *B, complex_t *C) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_data_t *args = malloc(num_threads * sizeof(thread_data_t));

    for (int i = 0; i < num_threads; i++) {
        args[i].start_row = i * n / num_threads;
        args[i].end_row = (i + 1) * n / num_threads;
        args[i].n = n;
        args[i].A = A;
        args[i].B = B;
        args[i].C = C;
        pthread_create(&threads[i], NULL, multiply_worker, &args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
}

int main(int argc, char *argv[]) {
    int n = 1024;
    int num_threads = 4;

    for (int i = 1; i < argc - 1; i++) {
        if (argv[i][0] == '-' && argv[i][1] == 't' && argv[i][2] == '\0') {
            sscanf(argv[i + 1], "%d", &num_threads);
        }
        if (argv[i][0] == '-' && argv[i][1] == 'n' && argv[i][2] == '\0') {
            sscanf(argv[i + 1], "%d", &n);
        }
    }

    if (num_threads <= 0 || n <= 0) {
        printf("Usage: %s [-t threads] [-n matrix_size]\n", argv[0]);
        return 1;
    }

    srand(42);

    complex_t *A = malloc(n * n * sizeof(complex_t));
    complex_t *B = malloc(n * n * sizeof(complex_t));
    complex_t *C = malloc(n * n * sizeof(complex_t));

    fill_matrix(A, n);
    fill_matrix(B, n);

    double t1 = get_time_ms();
    multiply_seq(n, A, B, C);
    double t2 = get_time_ms();
    double seq_time = t2 - t1;

    t1 = get_time_ms();
    multiply_par(n, num_threads, A, B, C);
    t2 = get_time_ms();
    double par_time = t2 - t1;

    double speedup = seq_time / par_time;
    double efficiency = speedup / num_threads;

    printf("Matrix: %dx%d | Threads: %d | Seq: %.2f ms | Par: %.2f ms | S: %.4f | E: %.4f\n",
           n, n, num_threads, seq_time, par_time, speedup, efficiency);

    free(A);
    free(B);
    free(C);
    return 0;
}
