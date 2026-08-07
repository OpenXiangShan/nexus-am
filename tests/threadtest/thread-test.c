#include <am.h>
#include <stdint.h>
#include <xs.h>
#include <thread.h>
#include <klib.h>


#define NCPUS 4
#define WORKERS 6

void* worker_thread(void* arg) {
    int id = (int)(intptr_t)arg;
    while(1) {
        printf("t%d", id);
    }
    return (void*)(intptr_t)(id * 2);
}

int main() {
    _mpe_setncpu('0' + NCPUS);
    thread_t t[WORKERS];
    void *retval[WORKERS];

    for(int i=0; i<WORKERS; i++) {
        thread_create(&t[i], NULL, worker_thread, (void*)(uintptr_t)(i));

    }
    for(int i=0; i<WORKERS; i++) {
        thread_join(t[i], &retval[i]);
    }

    printf("Thread returned: %ld, %ld\n", (intptr_t)t[1], (intptr_t)retval[1]);
    return 0;
}
