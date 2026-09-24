#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stddef.h>

#define INT16_TO_LE_BYTES(x)      \
    (uint8_t)((x) & 0xFF),        \
    (uint8_t)(((x) >> 8) & 0xFF)

#define INT16_TO_BE_BYTES(x)      \
    (uint8_t)(((x) >> 8) & 0xFF), \
    (uint8_t)((x) & 0xFF)


#define INT32_TO_LE_BYTES(x)      \
    (uint8_t)((x) & 0xFF),        \
    (uint8_t)(((x) >> 8) & 0xFF), \
    (uint8_t)(((x) >> 16) & 0xFF),\
    (uint8_t)(((x) >> 24) & 0xFF)

#define INT32_TO_BE_BYTES(x)      \
    (uint8_t)(((x) >> 24) & 0xFF),\
    (uint8_t)(((x) >> 16) & 0xFF),\
    (uint8_t)(((x) >> 8) & 0xFF), \
    (uint8_t)((x) & 0xFF)


#define INT64_TO_LE_BYTES(x)       \
    (uint8_t)((x) & 0xFF),         \
    (uint8_t)(((x) >> 8) & 0xFF),  \
    (uint8_t)(((x) >> 16) & 0xFF), \
    (uint8_t)(((x) >> 24) & 0xFF), \
    (uint8_t)(((x) >> 32) & 0xFF), \
    (uint8_t)(((x) >> 40) & 0xFF), \
    (uint8_t)(((x) >> 48) & 0xFF), \
    (uint8_t)(((x) >> 56) & 0xFF)

#define INT64_TO_BE_BYTES(x)       \
    (uint8_t)(((x) >> 56) & 0xFF), \
    (uint8_t)(((x) >> 48) & 0xFF), \
    (uint8_t)(((x) >> 40) & 0xFF), \
    (uint8_t)(((x) >> 32) & 0xFF), \
    (uint8_t)(((x) >> 24) & 0xFF), \
    (uint8_t)(((x) >> 16) & 0xFF), \
    (uint8_t)(((x) >> 8) & 0xFF),  \
    (uint8_t)((x) & 0xFF)

#endif
