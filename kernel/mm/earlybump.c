#include <mm/earlybump.h>

#include <compiler/attributes.h>
#include <lib/sizes.h>

#include <lib/memory.h>

struct earlybump_s {
  uint8_t reserved[EARLYBUMP_RESERVED_SIZE];

  uint8_t* brk;
  uint8_t* brk_limit;

  uint8_t disabled;
} __aligned(KiB(4));

static struct earlybump_s earlybump = {
  .disabled = 0,
};

void earlybump_init(void)
{
  if(earlybump.disabled)
  {
    return;
  }

  earlybump.brk = earlybump.reserved;
  earlybump.brk_limit = earlybump.reserved + sizeof(earlybump.reserved);
}

void earlybump_disable(void)
{
  earlybump.disabled = 1;
}

void* earlybump_alloc(size_t size, size_t aligned)
{
  if(earlybump.disabled)
  {
    return NULL;
  }

  uint8_t* target = (uint8_t*)ALIGN_UP((uintptr_t)earlybump.brk, aligned);
  uint8_t* target_end = (target + size);

  if(target_end > earlybump.brk_limit)
  {
    return NULL;
  }
  
  earlybump.brk = (uint8_t*)target_end;
  return (void*)target;
}
