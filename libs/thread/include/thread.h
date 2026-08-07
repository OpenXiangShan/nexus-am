#ifndef __THREAD_H__
#define __THREAD_H__

#include <am.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Thread handle type
typedef uintptr_t thread_t;

// Thread attributes
typedef struct {
    size_t stack_size;     // Stack size in bytes
    int priority;          // Thread priority (implementation defined)
    int detach_state;      // Joinable or detached
} thread_attr_t;

// Mutex type
typedef struct {
    volatile intptr_t lock;
    int type;
    thread_t owner;
} thread_mutex_t;

// Mutex attributes
typedef struct {
    int type;              // PTHREAD_MUTEX_NORMAL, PTHREAD_MUTEX_RECURSIVE, etc.
} thread_mutexattr_t;

// Condition variable
typedef struct {
    volatile intptr_t wait_count;
    volatile intptr_t wake_count;
    volatile intptr_t lock;
    thread_mutex_t *mutex;
} thread_cond_t;

// Condition variable attributes
typedef struct {
    // Reserved for future use
} thread_condattr_t;

// Once control
typedef struct {
    volatile intptr_t done;
    volatile intptr_t in_progress;
} thread_once_t;

// Thread-specific data key
typedef uintptr_t thread_key_t;

/* Absolute microsecond timestamp in the same clock domain as uptime(). */
typedef uint32_t thread_time_t;

// Thread creation and management
int thread_create(thread_t *thread, const thread_attr_t *attr,
                  void *(*start_routine)(void *), void *arg);
int thread_join(thread_t thread, void **retval);
int thread_detach(thread_t thread);
void thread_exit(void *retval);
thread_t thread_self(void);
int thread_equal(thread_t t1, thread_t t2);
int thread_yield(void);

// Thread attributes
int thread_attr_init(thread_attr_t *attr);
int thread_attr_destroy(thread_attr_t *attr);
int thread_attr_setstacksize(thread_attr_t *attr, size_t stacksize);
int thread_attr_getstacksize(const thread_attr_t *attr, size_t *stacksize);
int thread_attr_setdetachstate(thread_attr_t *attr, int detachstate);
int thread_attr_getdetachstate(const thread_attr_t *attr, int *detachstate);

// Mutex operations
int thread_mutex_init(thread_mutex_t *mutex, const thread_mutexattr_t *attr);
int thread_mutex_destroy(thread_mutex_t *mutex);
int thread_mutex_lock(thread_mutex_t *mutex);
int thread_mutex_trylock(thread_mutex_t *mutex);
int thread_mutex_unlock(thread_mutex_t *mutex);

// Mutex attributes
int thread_mutexattr_init(thread_mutexattr_t *attr);
int thread_mutexattr_destroy(thread_mutexattr_t *attr);
int thread_mutexattr_settype(thread_mutexattr_t *attr, int type);
int thread_mutexattr_gettype(const thread_mutexattr_t *attr, int *type);

// Condition variable operations
int thread_cond_init(thread_cond_t *cond, const thread_condattr_t *attr);
int thread_cond_destroy(thread_cond_t *cond);
int thread_cond_wait(thread_cond_t *cond, thread_mutex_t *mutex);
int thread_cond_timedwait(thread_cond_t *cond, thread_mutex_t *mutex,
                         const thread_time_t *abstime);
int thread_cond_signal(thread_cond_t *cond);
int thread_cond_broadcast(thread_cond_t *cond);

// Once initialization
int thread_once(thread_once_t *once_control, void (*init_routine)(void));

// Thread-specific data
int thread_key_create(thread_key_t *key, void (*destructor)(void *));
int thread_key_delete(thread_key_t key);
void *thread_getspecific(thread_key_t key);
int thread_setspecific(thread_key_t key, const void *value);

// Barrier (if supported)
typedef struct {
    volatile intptr_t lock;
    volatile intptr_t count;
    volatile intptr_t generation;
    unsigned required;
} thread_barrier_t;

int thread_barrier_init(thread_barrier_t *barrier, unsigned count);
int thread_barrier_destroy(thread_barrier_t *barrier);
int thread_barrier_wait(thread_barrier_t *barrier);

// Error codes
#define THREAD_SUCCESS 0
#define THREAD_ERROR -1
#define THREAD_BUSY -2
#define THREAD_TIMEOUT -3
#define THREAD_NOMEM -4
#define THREAD_INVAL -5
#define THREAD_PERM -6

// Detach state constants
#define THREAD_CREATE_JOINABLE 0
#define THREAD_CREATE_DETACHED 1

// Mutex type constants
#define THREAD_MUTEX_NORMAL 0
#define THREAD_MUTEX_RECURSIVE 1
#define THREAD_MUTEX_ERRORCHECK 2

// Default values
#define THREAD_STACK_MIN (4096)  // 4KB minimum stack
#define THREAD_STACK_DEFAULT (16384) // 16KB default stack

#ifdef __cplusplus
}
#endif

#endif // __THREAD_H__
