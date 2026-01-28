
// Sequential Quicksort and Mergesort
// This file runs both sorting algorithms without threads.


#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int CSV_MODE = 0;

// This function helps measure time in milliseconds.
static double ms_between(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec)*1000.0 + (b.tv_nsec - a.tv_nsec)/1e6;
}

// Simple swap helper.
static inline void swap_int(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

// Makes a copy of an array.
static int *copy_array(const int *src, int n) {
    size_t bytes = (size_t)n * sizeof(int);
    if (bytes / sizeof(int) != (size_t)n) {
        fprintf(stderr, "size overflow\n");
        exit(1);
    }
    int *dst = (int *)malloc(bytes);
    if (!dst) {
        perror("malloc");
        exit(1);
    }
    memcpy(dst, src, sizeof(int)*n);
    return dst;
}

// Just checks if sorted correctly.
static int check_sorted(const int *a, int n) {
    for (int i = 1; i < n; i++) {
        if (a[i-1] > a[i]) return 0;
    }
    return 1;
}

// ------------------- Quicksort (no threads) ---------------------

// Picks last element as pivot and rearranges.
static int partition_simple(int *a, int L, int R) {
    int pivot = a[R];
    int i = L - 1;

    for (int j = L; j < R; j++) {
        if (a[j] <= pivot) {
            i++;
            swap_int(&a[i], &a[j]);
        }
    }
    swap_int(&a[i+1], &a[R]);
    return i+1;
}

// Normal recursive quicksort.
void quicksort_seq(int *a, int L, int R) {
    if (L < R) {
        int p = partition_simple(a, L, R);
        quicksort_seq(a, L, p-1);
        quicksort_seq(a, p+1, R);
    }
}

// ------------------- Mergesort (no threads) ---------------------

// Merges two sorted halves into one.
static void merge_simple(int *a, int L, int M, int R) {
    int size_left = M - L + 1;
    int size_right = R - M;

    int *Lbuf = (int *)malloc(sizeof(int)*size_left);
    int *Rbuf = (int *)malloc(sizeof(int)*size_right);
    if (!Lbuf || !Rbuf) {
        perror("malloc");
        free(Lbuf);
        free(Rbuf);
        exit(1);
    }

    for (int i = 0; i < size_left; i++) Lbuf[i] = a[L+i];
    for (int j = 0; j < size_right; j++) Rbuf[j] = a[M+1+j];

    int i = 0, j = 0, k = L;

    while (i < size_left && j < size_right) {
        if (Lbuf[i] <= Rbuf[j]) a[k++] = Lbuf[i++];
        else a[k++] = Rbuf[j++];
    }

    while (i < size_left) a[k++] = Lbuf[i++];
    while (j < size_right) a[k++] = Rbuf[j++];

    free(Lbuf);
    free(Rbuf);
}

// Normal recursive mergesort.
void mergesort_seq(int *a, int L, int R) {
    if (L < R) {
        int M = L + (R - L)/2;
        mergesort_seq(a, L, M);
        mergesort_seq(a, M+1, R);
        merge_simple(a, L, M, R);
    }
}

// Main function.

int main(int argc, char **argv) {

    // Simple usage check.
    if (argc < 2) {
        fprintf(stderr, "usage: %s <N> [--csv]\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    if (N <= 0) {
        fprintf(stderr, "N must be positive\n");
        return 1;
    }
    if (argc >= 3 && strcmp(argv[2], "--csv") == 0) CSV_MODE = 1;

    // Make original random array.
    size_t bytes = (size_t)N * sizeof(int);
    if (bytes / sizeof(int) != (size_t)N) {
        fprintf(stderr, "size overflow\n");
        return 1;
    }
    int *orig = (int *)malloc(bytes);
    if (!orig) {
        perror("malloc");
        return 1;
    }
    srand(42);
    for (int i = 0; i < N; i++) orig[i] = rand();

    // Make copies for each algorithm.
    int *A_qs = copy_array(orig, N);
    int *A_ms = copy_array(orig, N);

    struct timespec s, e;

    // Time sequential quicksort.
    clock_gettime(CLOCK_MONOTONIC, &s);
    quicksort_seq(A_qs, 0, N-1);
    clock_gettime(CLOCK_MONOTONIC, &e);
    if (CSV_MODE) printf("QS,SEQ,%d,1,%.3f\n", N, ms_between(s,e));
    else          printf("ALG=QS MODE=SEQ N=%d T=1 TIME_MS=%.3f\n", N, ms_between(s,e));

    // Time sequential mergesort.
    clock_gettime(CLOCK_MONOTONIC, &s);
    mergesort_seq(A_ms, 0, N-1);
    clock_gettime(CLOCK_MONOTONIC, &e);
    if (CSV_MODE) printf("MS,SEQ,%d,1,%.3f\n", N, ms_between(s,e));
    else          printf("ALG=MS MODE=SEQ N=%d T=1 TIME_MS=%.3f\n", N, ms_between(s,e));

    // Quick sorted check.
    if (!check_sorted(A_qs, N)) printf("WARNING: quicksort not sorted\n");
    if (!check_sorted(A_ms, N)) printf("WARNING: mergesort not sorted\n");

    free(orig); free(A_qs); free(A_ms);

    return 0;
}