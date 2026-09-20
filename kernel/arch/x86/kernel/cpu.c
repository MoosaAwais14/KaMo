#include <asm/cpu_arch.h>

#include <sync/spinlock.h>
#include <lib/memory.h>

static uint32_t arch_cpu_id_default(void);
static uint32_t arch_cpu_count_default(void);

static arch_cpu_t arch_cpus[CPU_MAX] = { 0 };

static arch_cpu_current_id_fn_t current_id_provider = arch_cpu_id_default;
static arch_cpu_cpu_count_fn_t cpu_count_provider = arch_cpu_count_default;
static raw_spinlock_t provider_lock = RAW_SPINLOCK_UNLOCKED;

int arch_percpu_early_init(cpu_t *cpu)
{
  if (!cpu || cpu->arch_priv)
    return -1;

  if (cpu->id >= CPU_MAX)
    return -1;

  arch_cpu_t* arch = &arch_cpus[cpu->id];

  memset(arch, 0, sizeof(arch_cpu_t));

  if(gdt_cpu_init(&arch->gdt))
    return 1;

  if(gdt_cpu_load_kernel(&arch->gdt))
    return 2;

  if(idt_cpu_init(&arch->idt))
    return 3;

  cpu->arch_priv = arch;

  return 0;
}

int arch_percpu_init(cpu_t* cpu)
{
  if (!cpu || !cpu->arch_priv)
    return -1;

  arch_cpu_t* arch = cpu->arch_priv;

  if(idt_cpu_load(&arch->idt))
    return 1;

  return 0;
}

void arch_cpu_set_current_id_provider(arch_cpu_current_id_fn_t provider)
{
  unsigned long flags = raw_spin_lock_irqsave(&provider_lock);
  current_id_provider = provider ? provider : arch_cpu_id_default;
  raw_spin_unlock_irqrestore(&provider_lock, flags);
}

uint32_t arch_cpu_current_id(void)
{   
  return current_id_provider();
}

void arch_cpu_set_cpu_count_provider(arch_cpu_cpu_count_fn_t provider)
{
  unsigned long flags = raw_spin_lock_irqsave(&provider_lock);
  cpu_count_provider = provider ? provider : arch_cpu_count_default;
  raw_spin_unlock_irqrestore(&provider_lock, flags);
}

uint32_t arch_cpu_cpu_count(void)
{
  return cpu_count_provider();
}

static uint32_t arch_cpu_id_default(void)
{
  return 0;
}

static uint32_t arch_cpu_count_default(void)
{
  return 1;
}
