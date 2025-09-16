
// tbb_wrapper.cpp
#include "tbb_wrapper.h"

#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/task_group.h>
#include <tbb/global_control.h>

#include <cstddef>
#include <cstdio>
#include <exception>
#include <new>
#include <cstdint>
#include <type_traits>

extern "C" {

/* Helper to create a tbb::global_control that limits number of threads when requested.
   If max_threads <= 0 we don't create a global_control (use TBB default). */
static std::unique_ptr<tbb::global_control> make_global_control_if_needed(int max_threads) {
    if (max_threads > 0) {
        try {
            return std::make_unique<tbb::global_control>(tbb::global_control::max_allowed_parallelism, max_threads);
        } catch (...) {
            return nullptr;
        }
    }
    return nullptr;
}

int tbb_run_parallel_for(size_t n, c_task_fn fn, void *ctx, size_t grain_size, int max_threads) {
    if (!fn) return 2; // bad arg
    // guard against huge n -> but tbb handles size_t ranges well
    try {
        auto global_ctrl = make_global_control_if_needed(max_threads);
        if (max_threads > 0 && !global_ctrl) return 3; // failed to create control

        if (grain_size == 0) {
            // use default blocked_range without specifying grain size
            tbb::parallel_for(tbb::blocked_range<size_t>(0, n),
                [&](const tbb::blocked_range<size_t>& r){
                    for (size_t i = r.begin(); i < r.end(); ++i) fn(i, ctx);
                });
        } else {
            tbb::parallel_for(tbb::blocked_range<size_t>(0, n, grain_size),
                [&](const tbb::blocked_range<size_t>& r){
                    for (size_t i = r.begin(); i < r.end(); ++i) fn(i, ctx);
                });
        }
        return 0;
    } catch (const std::bad_alloc&) {
        return 5;
    } catch (const std::exception&) {
        return 4;
    } catch (...) {
        return 6;
    }
}

int tbb_parallel_sum(size_t n, c_reduce_fn fn, void *ctx, size_t grain_size, int max_threads, long long *out_sum) {
    if (!fn || !out_sum) return 2;
    try {
        auto global_ctrl = make_global_control_if_needed(max_threads);
        if (max_threads > 0 && !global_ctrl) return 3;

        if (grain_size == 0) {
            long long total = tbb::parallel_reduce(
                tbb::blocked_range<size_t>(0, n),
                (long long)0,
                [&](const tbb::blocked_range<size_t>& r, long long init) -> long long {
                    long long acc = init;
                    for (size_t i = r.begin(); i < r.end(); ++i) acc += fn(i, ctx);
                    return acc;
                },
                [](long long a, long long b) -> long long { return a + b; }
            );
            *out_sum = total;
        } else {
            long long total = tbb::parallel_reduce(
                tbb::blocked_range<size_t>(0, n, grain_size),
                (long long)0,
                [&](const tbb::blocked_range<size_t>& r, long long init) -> long long {
                    long long acc = init;
                    for (size_t i = r.begin(); i < r.end(); ++i) acc += fn(i, ctx);
                    return acc;
                },
                [](long long a, long long b) -> long long { return a + b; }
            );
            *out_sum = total;
        }
        return 0;
    } catch (const std::bad_alloc&) {
        return 5;
    } catch (const std::exception&) {
        return 4;
    } catch (...) {
        return 6;
    }
}

/* Run array of independent tasks.
   Each fn is expected to be a pointer to a function with signature: void f(void* ctx)
   We accept arrays of (const void*) and cast them back.
*/
int tbb_run_task_group(const void **fn_array, const void **ctx_array, size_t count, int max_threads) {
    if (count == 0) return 0;
    if (!fn_array) return 2;
    try {
        auto global_ctrl = make_global_control_if_needed(max_threads);
        if (max_threads > 0 && !global_ctrl) return 3;

        tbb::task_group tg;
        for (size_t i = 0; i < count; ++i) {
            using raw_fn_t = void (*)(void*);
            raw_fn_t f = reinterpret_cast<raw_fn_t>(const_cast<void*>(fn_array[i]));
            void *c = const_cast<void*>(ctx_array ? ctx_array[i] : nullptr);
            if (!f) continue; // skip null
            tg.run([f, c]() {
                try {
                    f(c);
                } catch (...) {
                    // swallow exceptions from user function to avoid unwinding into TBB
                    // Could record error state if desired; for now, we keep it simple.
                }
            });
        }
        tg.wait();
        return 0;
    } catch (const std::bad_alloc&) {
        return 5;
    } catch (const std::exception&) {
        return 4;
    } catch (...) {
        return 6;
    }
}

} // extern "C"
