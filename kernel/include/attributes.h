#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#define __always_inline   inline __attribute__((__always_inline__))

#define __noreturn        __attribute__((__noreturn__))

#define __aligned(x)      __attribute__((__aligned__(x)))

#define __packed          __attribute__((__packed__))

#endif
