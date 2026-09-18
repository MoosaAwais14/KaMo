#include <asm/i8259.h>

#include <asm/io.h>

static inline void io_wait(void);

static int legacy_init(void* __unused__)
{
  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();

  outb(PIC1_DATA, PIC_MASTER_OFFSET);
  io_wait();
  outb(PIC2_DATA, PIC_SLAVE_OFFSET);
  io_wait();

  outb(PIC1_DATA, 1 << CASCADE_IRQ);
  io_wait();
  outb(PIC2_DATA, CASCADE_IRQ);
  io_wait();

  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  outb(PIC1_DATA, 0xFF);
  outb(PIC2_DATA, 0xFF);

  return 0;
}

static void legacy_shutdown(void)
{
  outb(PIC1_DATA, 0xFF);
  outb(PIC2_DATA, 0xFF);
}

static int legacy_configure(uint32_t irq, irq_flags_t flags)
{
  return 0;
}

static int legacy_mask(uint32_t irq)
{
  uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
  uint8_t value = inb(port) | (1 << (irq % 8));
  outb(port, value);
  return 0;
}

static int legacy_unmask(uint32_t irq)
{
  uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
  uint8_t value = inb(port) & ~(1 << (irq % 8));
  outb(port, value);
  return 0;
}

static void legacy_eoi(uint32_t irq)
{
  if (irq >= 8)
  {
    outb(PIC2_COMMAND, PIC_EOI);
  }

  outb(PIC1_COMMAND, PIC_EOI);
}

static inline void io_wait(void)
{
  outb(0x80, 0);
}

const irq_chip_t legacy_pic = {
  .name = "i8259",

  .init = legacy_init,
  .shutdown = legacy_shutdown,

  .configure = legacy_configure,

  .mask = legacy_mask,
  .unmask = legacy_unmask,

  .eoi = legacy_eoi
};
