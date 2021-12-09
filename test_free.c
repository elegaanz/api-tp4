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
        debug("Allocating %d bytes\n", 100 * i);
        void *ptr = mem_alloc(100 * i);
        mem_free(ptr);
    }

    int *ptr1 = mem_alloc(64);
    int *ptr2 = mem_alloc(64);
    int *ptr3 = mem_alloc(64);
    mem_free(ptr3);
    mem_free(ptr2);
    mem_free(ptr1);

    ptr1 = mem_alloc(64);
    *ptr1 = 42;
    ptr2 = mem_alloc(64);
    *ptr2 = 86;
    ptr3 = mem_alloc(64);
    *ptr3 = 651;
    mem_free(ptr2);
    mem_free(ptr1);
    ptr2 = mem_alloc(64);
    printf("%d %d %d\n", *ptr1, *ptr2, *ptr3);
    mem_free(ptr3);
    // TEST OK
    return 0;
}
