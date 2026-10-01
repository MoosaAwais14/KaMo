#include <mm/frame_alloc.h>

#include <asm/frame_alloc.h>
#include <mm/memblock.h>

#include <mm/earlybump.h>

struct frame_alloc_s {
  section_t* section_list;
};

static const section_t* frame_alloc_add_section_range(range_t range);
static frame_alloc_err_t frame_alloc_add_free_block(section_t* section, size_t order, uint64_t pfn);

static struct frame_alloc_s frame_alloc = { 0 };

frame_alloc_err_t frame_alloc_init(void)
{
  memblock_iter_t mem_it = { NULL };
  memblock_iter_t res_it = { NULL };

  range_t memory;
  range_t reserved;

  memblock_err_t memory_err = memblock_memory_first(&mem_it, &memory);

  if (memory_err != MEMBLOCK_OK)
    return FRAME_ALLOC_INVALID;

  memblock_err_t reserved_err = memblock_reserved_first(&res_it, &reserved);

  do
  {
    phys_addr_t cursor = ALIGN_UP(memory.start, FRAME_SIZE);
    phys_addr_t end = ALIGN_DOWN(memory.end, FRAME_SIZE) + FRAME_SIZE - 1;

    if (cursor > end)
      goto next_memory_range;

    while (reserved_err == MEMBLOCK_OK && reserved.end < cursor)
    {
      reserved_err = memblock_reserved_next(&res_it, &reserved);
    }

    while (reserved_err == MEMBLOCK_OK && reserved.start <= end)
    {
      if (reserved.start > cursor)
      {
        phys_addr_t free_end = ALIGN_DOWN(reserved.start, FRAME_SIZE) - 1;

        if (free_end >= cursor)
        {
          if (!frame_alloc_add_section_range((range_t) {
            .start = cursor,
            .end   = free_end
          }))
            return FRAME_ALLOC_INVALID;
        }
      }

      if (reserved.end == UINTPTR_MAX)
      {
        cursor = end + 1;
        break;
      }

      phys_addr_t next = ALIGN_UP(reserved.end + 1, FRAME_SIZE);

      if (next > cursor)
        cursor = next;

      if (cursor > end)
        break;

      reserved_err = memblock_reserved_next(&res_it, &reserved);
    }

    if (cursor <= end)
    {
      if (!frame_alloc_add_section_range((range_t) {
        .start = cursor,
        .end   = end
      }))
        return FRAME_ALLOC_INVALID;
    }

  next_memory_range:
    ;
  }
  while (memblock_memory_next(&mem_it, &memory) == MEMBLOCK_OK);

  section_t* previous = NULL;
  for (section_t* section = frame_alloc.section_list; section; section = section->next)
  {
    if (range_len(&section->range) == 0 || 
      (section->range.start & (FRAME_SIZE - 1)) != 0 ||
      (section->range.end & (FRAME_SIZE - 1)) != FRAME_SIZE - 1 ||
      (previous && range_overlaps(&previous->range, &section->range)) ||
      section->nr_frames != 0 ||
      section->bitmap.array != NULL ||
      section->orders != NULL)
      return FRAME_ALLOC_INVALID;

    previous = section;
  }

  for (section_t* section = frame_alloc.section_list; section; section = section->next)
  {
    
  }

  return FRAME_ALLOC_OK;
}

static const section_t* frame_alloc_add_section_range(range_t range)
{
  section_t** prev = &frame_alloc.section_list;
  while (*prev)
  {
    prev = &(*prev)->next;
  }

  if (range.start > range.end ||
    (range.start & (FRAME_SIZE - 1)) != 0 || 
    (range.end & (FRAME_SIZE - 1)) != FRAME_SIZE - 1)
    return NULL;

  section_t* section = earlybump_alloc(sizeof(*section), __alignof__(section_t));
  if (!section)
    return NULL;

  section->range = range;
  section->base_pfn = 0;
  section->nr_frames = 0;
  section->nr_orders = 0;
  section->orders = NULL;
  section->next = NULL;
  section->rlock = RAW_SPINLOCK_UNLOCKED;

  bitmap_init(&section->bitmap);
  *prev = section;

  return section;
}