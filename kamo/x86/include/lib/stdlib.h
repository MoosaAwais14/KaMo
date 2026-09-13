#ifndef KAMO_X86_INCLUDE_LIB_STDLIB_H
#define KAMO_X86_INCLUDE_LIB_STDLIB_H

#include <stdint.h>

static inline int atoi(const char *s) {
    int sign = 1;
    int result = 0;

    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }

    return result * sign;
}

static inline long atol(const char *s) {
    long sign = 1;
    long result = 0;

    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }

    return result * sign;
}

static inline char *itoa(int value, char *str, int base) {
    char *ptr = str;
    char *ptr1 = str;
    char tmp;
    int negative = 0;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    if (value < 0 && base == 10) {
        negative = 1;
        value = -value;
    }

    while (value != 0) {
        int rem = value % base;
        *ptr++ = (rem < 10)
            ? ('0' + rem)
            : ('a' + (rem - 10));

        value /= base;
    }

    if (negative)
        *ptr++ = '-';

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp = *ptr;
        *ptr = *ptr1;
        *ptr1 = tmp;

        ptr--;
        ptr1++;
    }

    return str;
}

static inline char *utoa(unsigned int value, char *str, int base) {
    char *ptr = str;
    char *ptr1 = str;
    char tmp;

    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }

    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    while (value != 0) {
        unsigned int rem = value % (unsigned int)base;

        *ptr++ = (rem < 10)
            ? ('0' + rem)
            : ('a' + (rem - 10));

        value /= (unsigned int)base;
    }

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp = *ptr;
        *ptr = *ptr1;
        *ptr1 = tmp;

        ptr--;
        ptr1++;
    }

    return str;
}

#endif
