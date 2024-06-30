#pragma once

#ifdef __GNUC__
// GCC don't have a builtin min/max macro, so we need to define one for use
#define max(a, b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a, b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})
#elif defined(_MSC_VER)

// Mscvs has min/max macros built into stdlib
#include <stdlib.h>

#endif

#define MGPU_API_VERSION 2
