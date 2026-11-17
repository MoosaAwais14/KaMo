#ifndef MM_IOREMAP_H
#define MM_IOREMAP_H

#include <stdint.h>
#include <stddef.h>
#include <mm/mmu_types.h>
#include <kernel/address.h>

extern mmu_err_t ioremap(phys_addr_t paddr, size_t size, mmu_flags_t flags, void** out_addr);
extern mmu_err_t iounmap(void* addr);

#endif
