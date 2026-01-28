// Parallel Quicksort and Mergesort using pthreads
// This file runs both algorithms with threads.
// I also run the sequential versions here for comparison.
// Comments are simple so beginners can follow easily.

#define _XOPEN_SOURCE 700
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// This struct is what each thread will receive.
typedef struct {
    int *arr;
    int left;
    int right;
    int depth;  // how deep in recursion we are
} args_t;

// This controls how many threads can be made.
static int MAX_DEPTH = 0;

// If --csv is passed, we print CSV lines.
static int CSV_MODE = 0;

// Time helper.
static double ms_between(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec)*1000.0 + (b.tv_nsec - a.tv_nsec)/1e6;
}

// Simple swap.
static inline void swap_int(int *a, int *b) {
    int t=*a;
    *a=*b;
    *b=t;
}

// Make a copy of array.
static int *copy_array(const int *src, int n) {
    int *dst = malloc(sizeof(int)*n);
    memcpy(dst, src, sizeof(int)*n);
    return dst;
}

// Check if sorted.
static int check_sorted(const int *a, int n) {
    for (int i=1;i<n;i++)
        if (a[i-1] > a[i]) return 0;
    return 1;
}

// Sequential Quicksort 
static int partition_simple(int *a,int L,int R) {
    int pivot = a[R];
    int i = L - 1;

    for (int j=L; j<R; j++) {
        if (a[j] <= pivot) {
            i++;
            swap_int(&a[i], &a[j]);
        }
    }

    swap_int(&a[i+1], &a[R]);
    return i+1;
}

void quicksort_seq(int *a, int L, int R) {
    if (L < R) {
        int p = partition_simple(a, L, R);
        quicksort_seq(a, L, p-1);
        quicksort_seq(a, p+1, R);
    }
}

//  Sequential Mergesort 
static void merge_simple(int *a, int L, int M, int R) {
    int sizeL = M-L+1;
    int sizeR = R-M;

    int *Lbuf = malloc(sizeof(int)*sizeL);
    int *Rbuf = malloc(sizeof(int)*sizeR);

    for (int i=0;i<sizeL;i++) Lbuf[i] = a[L+i];
    for (int j=0;j<sizeR;j++) Rbuf[j] = a[M+1+j];

    int i=0, j=0, k=L;

    while (i<sizeL && j<sizeR) {
        if (Lbuf[i] <= Rbuf[j]) a[k++] = Lbuf[i++];
        else a[k++] = Rbuf[j++];
    }

    while (i<sizeL) a[k++] = Lbuf[i++];
    while (j<sizeR) a[k++] = Rbuf[j++];

    free(Lbuf);
    free(Rbuf);
}

void mergesort_seq(int *a, int L, int R) {
    if (L < R) {
        int M = L + (R-L)/2;
        mergesort_seq(a, L, M);
        mergesort_seq(a, M+1, R);
        merge_simple(a, L, M, R);
    }
}

//  Parallel Quicksort 
static void *qs_worker(void *arg) {
    args_t *x = (args_t*)arg;
    int L = x->left;
    int R = x->right;
    int d = x->depth;

    if (L < R) {
        int p = partition_simple(x->arr, L, R);

        // Only make new threads if we are not too deep.
        if (d < MAX_DEPTH) {
            pthread_t t1, t2;

            args_t left_args = { x->arr, L, p-1, d+1 };
            args_t right_args = { x->arr, p+1, R, d+1 };

            // Make 2 threads: one for left, one for right.
            pthread_create(&t1, NULL, qs_worker, &left_args);
            pthread_create(&t2, NULL, qs_worker, &right_args);

            // Wait for them.
            pthread_join(t1, NULL);
            pthread_join(t2, NULL);

        } else {
            // If too deep, just run normal (sequential).
            quicksort_seq(x->arr, L, p-1);
            quicksort_seq(x->arr, p+1, R);
        }
    }

    return NULL;
}

void quicksort_parallel(int *a, int n) {
    args_t root = { a, 0, n-1, 0 };
    qs_worker(&root);
}

//  Parallel Mergesort 
static void *ms_worker(void *arg) {
    args_t *x = (args_t*)arg;
    int L = x->left;
    int R = x->right;
    int d = x->depth;

    if (L < R) {
        int M = L + (R-L)/2;

        if (d < MAX_DEPTH) {
            pthread_t t1, t2;

            args_t left_args  = { x->arr, L, M, d+1 };
            args_t right_args = { x->arr, M+1, R, d+1 };

            pthread_create(&t1, NULL, ms_worker, &left_args);
            pthread_create(&t2, NULL, ms_worker, &right_args);

            pthread_join(t1, NULL);
            pthread_join(t2, NULL);

        } else {
            mergesort_seq(x->arr, L, M);
            mergesort_seq(x->arr, M+1, R);
        }

        merge_simple(x->arr, L, M, R);
    }

    return NULL;
}

void mergesort_parallel(int *a, int n) {
    args_t root = { a, 0, n-1, 0 };
    ms_worker(&root);
}

// Main
int main(int argc, char **argv) {

    if (argc < 3) {
        printf("usage: %s <N> <max_threads> [--csv]\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int T = atoi(argv[2]);

    if (argc == 4 && strcmp(argv[3], "--csv") == 0)
        CSV_MODE = 1;

    // Convert max threads to max depth (~ log2).
    int d = 0;
    while ((1 << d) < T) d++;
    MAX_DEPTH = d;

    // Make random data.
    int *orig = malloc(sizeof(int)*N);
    srand(42);
    for (int i=0;i<N;i++) orig[i] = rand();

    // Copies so each algorithm gets the same input.
    int *A_qs_seq = copy_array(orig, N);
    int *A_qs_par = copy_array(orig, N);

    int *A_ms_seq = copy_array(orig, N);
    int *A_ms_par = copy_array(orig, N);

    struct timespec s, e;

    // --- QS SEQ ---
    clock_gettime(CLOCK_MONOTONIC, &s);
    quicksort_seq(A_qs_seq, 0, N-1);
    clock_gettime(CLOCK_MONOTONIC, &e);

    if (CSV_MODE) printf("QS,SEQ,%d,1,%.3f\n", N, ms_between(s,e));
    else printf("ALG=QS MODE=SEQ N=%d T=1 TIME_MS=%.3f\n", N, ms_between(s,e));

    // --- QS PAR ---
    clock_gettime(CLOCK_MONOTONIC, &s);
    quicksort_parallel(A_qs_par, N);
    clock_gettime(CLOCK_MONOTONIC, &e);

    if (CSV_MODE) printf("QS,PAR,%d,%d,%.3f\n", N, T, ms_between(s,e));
    else printf("ALG=QS MODE=PAR N=%d T=%d TIME_MS=%.3f\n", N, T, ms_between(s,e));

    // --- MS SEQ ---
    clock_gettime(CLOCK_MONOTONIC, &s);
    mergesort_seq(A_ms_seq, 0, N-1);
    clock_gettime(CLOCK_MONOTONIC, &e);

    if (CSV_MODE) printf("MS,SEQ,%d,1,%.3f\n", N, ms_between(s,e));
    else printf("ALG=MS MODE=SEQ N=%d T=1 TIME_MS=%.3f\n", N, ms_between(s,e));

    // --- MS PAR ---
    clock_gettime(CLOCK_MONOTONIC, &s);
    mergesort_parallel(A_ms_par, N);
    clock_gettime(CLOCK_MONOTONOTIC, &e);

    if (CSV_MODE) printf("MS,PAR,%d,%d,%.3f\n", N, T, ms_between(s,e));
    else printf("ALG=MS MODE=PAR N=%d T=%d TIME_MS=%.3f\n", N, T, ms_between(s,e));

    // Check if sorted.
    if (!check_sorted(A_qs_seq,N)) printf("WARNING: QS SEQ not sorted!\n");
    if (!check_sorted(A_qs_par,N)) printf("WARNING: QS PAR not sorted!\n");
    if (!check_sorted(A_ms_seq,N)) printf("WARNING: MS SEQ not sorted!\n");
    if (!check_sorted(A_ms_par,N)) printf("WARNING: MS PAR not sorted!\n");

    free(orig);
    free(A_qs_seq); free(A_qs_par);
    free(A_ms_seq); free(A_ms_par);

    return 0;
}