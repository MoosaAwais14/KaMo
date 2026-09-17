#include <interrupt.h>

#include <irq.h>

#include <lib/memory.h>

static interrupt_desc_t interrupt_descs[INTERRUPT_VECTOR_COUNT] = { 0 };

int interrupt_init(void)
{
  memset(&interrupt_descs, 0, sizeof(interrupt_descs));
  
  for (uint32_t vector = 0; vector < INTERRUPT_VECTOR_COUNT; vector++) {
    interrupt_desc_t* desc = &interrupt_descs[vector]; 

    desc->vector = vector;
    desc->type = INTERRUPT_NONE;
    desc->handler = NULL;
    desc->arg = NULL;
  }

  return 0;
}

int interrupt_set_type(uint32_t vector, interrupt_type_t type)
{
  if(vector >= INTERRUPT_VECTOR_COUNT)
    return -1;

  interrupt_descs[vector].type = type;

  return 0;
}

int interrupt_register(uint32_t vector, interrupt_handler_t handler, void* arg)
{
  if(vector >= INTERRUPT_VECTOR_COUNT)
    return -1;

  interrupt_desc_t* desc = &interrupt_descs[vector]; 

  desc->handler = handler;
  desc->arg = arg;

  return 0;
}

int interrupt_unregister(uint32_t vector)
{
  if(vector >= INTERRUPT_VECTOR_COUNT)
    return -1;

  interrupt_desc_t* desc = &interrupt_descs[vector]; 

  desc->handler = NULL;
  desc->arg = NULL;

  return 0;
}

void interrupt_dispatch(interrupt_context_t* context)
{
  
}
