#include <kernel/cpu.h>

#include <stddef.h>

#include <lib/memory.h>

#include <asm/cpu_arch.h>

static cpu_t cpus[CPU_MAX] = { 0 };

cpu_t* cpu_current(void)
{
  uint32_t id = arch_cpu_current_id();
  return (id < CPU_MAX) ? &cpus[id] : NULL;
}

cpu_t* cpu_get(uint32_t id)
{
  return (id < CPU_MAX) ? &cpus[id] : NULL;
}

int cpu_early_init(uint32_t id)
{
  if (id >= CPU_MAX)
    return 1;

  cpu_t *cpu = &cpus[id];

  if (cpu->arch_priv)
    return 1;

  cpu->id = id;
  cpu->online = 0;
  cpu->preempt_count = 0;

  return arch_percpu_early_init(cpu);
}

int cpu_init(uint32_t id)
{
  if (id >= CPU_MAX)
    return 1;

  cpu_t *cpu = &cpus[id];

  if(!cpu->arch_priv || cpu->id != id)
    return 2;

  if(cpu->online)
    return 3;
  
  if(arch_percpu_init(cpu))
    return 4;

  cpu->online = 1;

  return 0;
}
