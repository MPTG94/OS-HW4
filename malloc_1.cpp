#include <iostream>
#include <unistd.h>

void *smalloc(size_t size) {
    if (size == 0 || size > 100000000) {
        return NULL;
    }
    void *res = sbrk(size);
    if (res == (void *) (-1)) {
        return NULL;
    }
    return res;
}