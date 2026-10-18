#pragma once

#include <cstdio>
#include <cstdlib>

// Always evaluates cond, including when NDEBUG is set.
#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__,        \
                         __LINE__, #cond);                                     \
            std::exit(EXIT_FAILURE);                                           \
        }                                                                      \
    } while (0)
