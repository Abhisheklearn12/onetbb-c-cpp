
#ifndef TBB_WRAPPER_H
#define TBB_WRAPPER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*c_task_fn)(size_t idx, void *ctx);
typedef long long (*c_reduce_fn)(size_t idx, void *ctx);

/*
 * Run parallel_for from 0..n-1 calling fn(idx, ctx) for each index.
 * grain_size: recommended chunk size for work-stealing; 0 means library default.
 * max_threads: if >0 request that many threads; if <=0 use TBB default.
 * Returns 0 on success, non-zero on error.
 */
int tbb_run_parallel_for(size_t n, c_task_fn fn, void *ctx, size_t grain_size, int max_threads);

/*
 * Parallel reduction: sums values produced by fn(idx, ctx) for idx in [0,n)
 * Result placed in out_sum (must be non-null).
 * Returns 0 on success, non-zero on error.
 */
int tbb_parallel_sum(size_t n, c_reduce_fn fn, void *ctx, size_t grain_size, int max_threads, long long *out_sum);

/*
 * Run an array of independent tasks (fn_array[i], ctx_array[i]) in a task group and wait.
 * fn_array: array of function pointers (cast to void*). Each function must be: void f(void* ctx)
 * ctx_array: array of contexts for each function.
 * count: number of tasks
 * max_threads: requested degree of parallelism (>0 to set)
 *
 * Returns 0 on success, non-zero on error.
 */
int tbb_run_task_group(const void **fn_array, const void **ctx_array, size_t count, int max_threads);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // TBB_WRAPPER_H
