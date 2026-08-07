#include <am.h>
#include <klib.h>
#include <thread.h>
#include <xsextra.h>
#include <xs.h>

#define NUM_THREADS 4
#define ITERATIONS 10
#define HART_COVERAGE_TIMEOUT_US 1000U
#define HART_COVERAGE_BACKOFF 64

#ifndef THREAD_DEMO_NCPU
#define THREAD_DEMO_NCPU 2
#endif

#ifndef THREAD_DEMO_VERBOSE
#define THREAD_DEMO_VERBOSE 0
#endif

#if THREAD_DEMO_NCPU < 1 || THREAD_DEMO_NCPU > MAX_CPU
#error "THREAD_DEMO_NCPU must be in [1, MAX_CPU]"
#endif

#if THREAD_DEMO_NCPU < NUM_THREADS
#define HART_COVERAGE_WORKERS THREAD_DEMO_NCPU
#else
#define HART_COVERAGE_WORKERS NUM_THREADS
#endif

#if THREAD_DEMO_VERBOSE
#define demo_log_m(...) atomic_printf(__VA_ARGS__)
#define demo_log(...) s_atomic_printf(__VA_ARGS__)
#else
#define demo_log_m(...) ((void)0)
#define demo_log(...) ((void)0)
#endif

// Shared counters
volatile long unsafe_counter = 0;
volatile long safe_counter = 0;

/* The first workers occupy a hart without yielding until all configured harts arrive. */
static volatile intptr_t hart_coverage_lock;
static volatile intptr_t worker_hart_mask;
static volatile intptr_t worker_hart_count;
static volatile intptr_t hart_coverage_timed_out;

// Mutex for protecting shared data
thread_mutex_t counter_mutex;

// Barrier for synchronization
thread_barrier_t start_barrier;
thread_barrier_t unsafe_read_barrier;
thread_barrier_t unsafe_write_barrier;

#define DEMO_WORKER_FAILURE ((void *)(intptr_t)-1)

static void lock_hart_coverage(void) {
    while (_atomic_xchg(&hart_coverage_lock, 1) != 0) {
        asm volatile("nop");
    }
}

static void unlock_hart_coverage(void) {
    _atomic_xchg(&hart_coverage_lock, 0);
}

/*
 * No worker yields while this gate is closed. Reaching the required count
 * therefore requires one worker to be resident on each configured hart.
 */
static int wait_for_hart_coverage(int thread_id) {
    if (thread_id >= HART_COVERAGE_WORKERS) {
        return THREAD_SUCCESS;
    }

    int cpu = _cpu();
    if (cpu < 0 || cpu >= _ncpu()) {
        return THREAD_ERROR;
    }

    lock_hart_coverage();
    intptr_t cpu_bit = (intptr_t)1 << cpu;
    if ((worker_hart_mask & cpu_bit) == 0) {
        worker_hart_mask |= cpu_bit;
        _atomic_add(&worker_hart_count, 1);
    }
    unlock_hart_coverage();

    uint32_t deadline = uptime() + HART_COVERAGE_TIMEOUT_US;
    while (worker_hart_count != HART_COVERAGE_WORKERS) {
        if ((int32_t)(uptime() - deadline) >= 0) {
            _atomic_xchg(&hart_coverage_timed_out, 1);
            return THREAD_TIMEOUT;
        }
        for (int i = 0; i < HART_COVERAGE_BACKOFF; i++) {
            asm volatile("nop");
        }
    }
    return THREAD_SUCCESS;
}

void *worker_thread(void *arg) {
    int thread_id = (int)(intptr_t)arg;

    demo_log("Thread %d started on CPU %d\n", thread_id, _cpu());

    int hart_coverage_result = wait_for_hart_coverage(thread_id);

    // Wait for all threads to start
    if (thread_barrier_wait(&start_barrier) != THREAD_SUCCESS) {
        return DEMO_WORKER_FAILURE;
    }
    if (hart_coverage_result != THREAD_SUCCESS) {
        return DEMO_WORKER_FAILURE;
    }

    demo_log("Thread %d: starting work\n", thread_id);

    /*
     * Force every worker to read the same value before any writes it back.
     * This exposes the non-atomic load/modify/store loss deterministically;
     * the hart-coverage gate above separately proves simultaneous multi-hart
     * worker residency.
     */
    long observed = unsafe_counter;
    if (thread_barrier_wait(&unsafe_read_barrier) != THREAD_SUCCESS) {
        return DEMO_WORKER_FAILURE;
    }
    unsafe_counter = observed + 1;
    if (thread_barrier_wait(&unsafe_write_barrier) != THREAD_SUCCESS) {
        return DEMO_WORKER_FAILURE;
    }

    // Test safe counter with mutex
    for (int i = 0; i < ITERATIONS; i++) {
        thread_mutex_lock(&counter_mutex);
        safe_counter++;
        thread_mutex_unlock(&counter_mutex);
    }

    demo_log("Thread %d: completed work\n", thread_id);

    return (void *)(intptr_t)thread_id;
}

void *recursive_mutex_test(void *arg) {
    thread_mutex_t recursive_mutex;
    thread_mutexattr_t attr;

    // Create recursive mutex
    thread_mutexattr_init(&attr);
    thread_mutexattr_settype(&attr, THREAD_MUTEX_RECURSIVE);
    thread_mutex_init(&recursive_mutex, &attr);

    // Test recursive locking
    thread_mutex_lock(&recursive_mutex);
    thread_mutex_lock(&recursive_mutex);  // Should work with recursive mutex

    demo_log("Recursive mutex test: acquired lock twice\n");

    thread_mutex_unlock(&recursive_mutex);
    thread_mutex_unlock(&recursive_mutex);

    thread_mutex_destroy(&recursive_mutex);
    thread_mutexattr_destroy(&attr);

    return NULL;
}

int main() {
    _mpe_setncpu('0' + THREAD_DEMO_NCPU);
    demo_log_m("====== AM Thread Library Demo ======\n");
    demo_log_m("Number of CPUs: %d\n", _ncpu());
    demo_log_m("Creating %d threads\n", NUM_THREADS);
    demo_log_m("Iterations per thread: %d\n", ITERATIONS);
    demo_log_m("Expected total: %d\n", NUM_THREADS * ITERATIONS);
    demo_log_m("\n");

    // Initialize synchronization primitives
    thread_mutex_init(&counter_mutex, NULL);
    if (thread_barrier_init(&start_barrier, NUM_THREADS + 1) != THREAD_SUCCESS ||
        thread_barrier_init(&unsafe_read_barrier, NUM_THREADS) != THREAD_SUCCESS ||
        thread_barrier_init(&unsafe_write_barrier, NUM_THREADS) != THREAD_SUCCESS) {
        return 1;
    }

    thread_t threads[NUM_THREADS];
    thread_attr_t attr;

    // Initialize thread attributes
    thread_attr_init(&attr);
    thread_attr_setstacksize(&attr, 8192);  // 8KB stack

    // Create worker threads
    for (int i = 0; i < NUM_THREADS; i++) {
        int result = thread_create(&threads[i], &attr, worker_thread, (void *)(intptr_t)i);
        if (result != THREAD_SUCCESS) {
            demo_log("Failed to create thread %d: error %d\n", i, result);
            return 1;
        } else {
            demo_log("Created thread %d with ID %lu\n", i, threads[i]);
        }

    }

    // Create recursive mutex test thread
    thread_t recursive_thread;
    if (thread_create(&recursive_thread, &attr, recursive_mutex_test, NULL) !=
        THREAD_SUCCESS) {
        return 1;
    }

    demo_log("Main thread: waiting for all threads to start...\n");
    if (thread_barrier_wait(&start_barrier) != THREAD_SUCCESS) {
        return 1;
    }
    demo_log("Main thread: all threads started, waiting for completion...\n");

    // Wait for recursive mutex test thread
    void *retval;
    if (thread_join(recursive_thread, &retval) != THREAD_SUCCESS || retval != NULL) {
        return 1;
    }
    demo_log("Recursive mutex test completed\n");

    // Wait for all worker threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        if (thread_join(threads[i], &retval) != THREAD_SUCCESS ||
            retval != (void *)(intptr_t)i) {
            return 1;
        }
        demo_log("Thread %d joined, returned %ld\n", i, (intptr_t)retval);
    }

    // Print results
    demo_log("\n====== Test Results ======\n");
    intptr_t expected_hart_mask = ((intptr_t)1 << HART_COVERAGE_WORKERS) - 1;
    long expected = NUM_THREADS * ITERATIONS;
    long unsafe_expected = 1;

    demo_log("Worker hart mask: 0x%lx\n", (unsigned long)worker_hart_mask);
    demo_log("Unsafe counter (forced race): %ld\n", unsafe_counter);
    demo_log("Safe counter (with mutex): %ld\n", safe_counter);
    demo_log("Worker hart mask: 0x%lx\n", (unsigned long)worker_hart_mask);
    demo_log("Unsafe counter (forced race): %ld\n", unsafe_counter);
    demo_log("Safe counter (with mutex): %ld\n", safe_counter);
    demo_log("Expected safe value: %ld\n", expected);

    if (hart_coverage_timed_out || worker_hart_mask != expected_hart_mask ||
        unsafe_counter != unsafe_expected) {
        return 1;
    }

    if (safe_counter == expected) {
        demo_log(" Mutex synchronization works correctly!\n");
        demo_log("  Safe counter matches expected value\n");
    } else {
        demo_log(" Mutex synchronization failed\n");
        demo_log("   Safe counter is %ld, expected %ld\n", safe_counter, expected);
        return 1;
    }

    // Cleanup
    thread_mutex_destroy(&counter_mutex);
    thread_barrier_destroy(&start_barrier);
    thread_barrier_destroy(&unsafe_read_barrier);
    thread_barrier_destroy(&unsafe_write_barrier);
    thread_attr_destroy(&attr);

    demo_log("\nThread library demo completed successfully!\n");
    _putc('P');

    return 0;
}
