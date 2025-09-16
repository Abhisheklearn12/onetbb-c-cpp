
// main.c
#include "tbb_wrapper.h"
#include <stdio.h>
#include <stdlib.h>

void print_task(size_t idx, void *ctx) {
    (void)ctx;
    printf("print_task: idx=%zu\n", idx);
}

long long produce_value(size_t idx, void *ctx) {
    (void)ctx;
    // produce value = idx * 2
    return (long long)idx * 2;
}

/* function pointer type for task group tasks: void f(void* ctx) */
void simple_group_fn1(void *ctx) {
    const char *s = (const char*)ctx;
    printf("group1: %s\n", s ? s : "(null)");
}
void simple_group_fn2(void *ctx) {
    int v = *(int*)ctx;
    printf("group2: value=%d\n", v);
}

int main(void) {
    printf("Parallel for (10 items) with grain_size=2:\n");
    if (tbb_run_parallel_for(10, print_task, NULL, 2, 0) != 0) {
        fprintf(stderr, "parallel_for failed\n");
        return 1;
    }

    printf("Parallel sum (0..999):\n");
    long long sum = 0;
    if (tbb_parallel_sum(1000, produce_value, NULL, 16, 0, &sum) != 0) {
        fprintf(stderr, "parallel_sum failed\n");
        return 1;
    }
    printf("sum = %lld (expected %lld)\n", sum, (long long)(999ULL * 1000ULL)); // produce_value gives 2*idx so expected = 2*sum0..999

    printf("Task group demo:\n");
    const void *fns[2];
    const void *ctxs[2];
    fns[0] = (const void*)simple_group_fn1;
    ctxs[0] = (const void*)"hello from group";
    fns[1] = (const void*)simple_group_fn2;
    int v = 42;
    ctxs[1] = (const void*)&v;

    if (tbb_run_task_group(fns, ctxs, 2, 0) != 0) {
        fprintf(stderr, "task_group failed\n");
        return 1;
    }

    printf("Done\n");
    return 0;
}
