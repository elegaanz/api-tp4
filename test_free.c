#include "common.h"
#include "mem.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define NB_TESTS 10

void p(void * addr, size_t available, int is_free) {
    printf("zone %p : %lu dispo (%d)\n", addr, available, is_free);
}

int main(int argc, char *argv[]) {
    printf("==== TEST free =====\n");
    for (int i = 0; i < NB_TESTS; i++) {
        debug("Initializing memory\n");
        mem_init(get_memory_adr(), get_memory_size());
        void *ptr = alloc_max(get_memory_size());
        mem_free(ptr);
    }

    for (int i = 0; i < NB_TESTS; i++) {
        debug("\nInitializing memory\n");
        mem_init(get_memory_adr(), get_memory_size());
        mem_show(p);
        debug("Allocating %d bytes\n", 100 * i);
        void *ptr = mem_alloc(100 * i);
        mem_free(ptr);
        mem_show(p);
    }

    printf("memoire avant p1\n");
    mem_show(p);
    void *ptr1 = mem_alloc(64);
    printf("memoire apres p1\n");
    mem_show(p);
    void *ptr2 = mem_alloc(64);
    printf("memoire apres p2\n");
    mem_show(p);
    void *ptr3 = mem_alloc(64);
    printf("memoire apres p3\n");
    mem_show(p);
    mem_free(ptr3);
    mem_free(ptr2);
    mem_free(ptr1);

    // TEST OK
    return 0;
}
