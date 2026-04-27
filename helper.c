#include "helper.h"

void safe_print(const char *fmt, ...) {
    va_list args;
    pthread_mutex_lock(&print_lock);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
    pthread_mutex_unlock(&print_lock);
}