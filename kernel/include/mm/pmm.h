#ifndef MM_PMM_H
#define MM_PMM_H

#include <stdint.h>
#include <stddef.h>

typedef enum pmm_err_e {
    PMM_OK,
    PMM_ERR_INVALID,
} pmm_err_t;

extern pmm_err_t pmm_init(void);

#endif
