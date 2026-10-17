#include "psx/bus.h"

#include <stddef.h>
#include <string.h>

#define IRQ_STATUS 0x1f801070u
#define IRQ_MASK   0x1f801074u
#define TIMER_BASE 0x1f801100u
#define DMA_BASE   0x1f801080u
#define DPCR_ADDR  0x1f8010f0u
#define DICR_ADDR  0x1f8010f4u
#define GPU_GP0    0x1f801810u
#define GPU_GP1    0x1f801814u
#define DMA_IRQ_BIT(channel) (1u << (24u + (channel)))
#define DMA_IRQ_ENABLE(channel) (1u << (16u + (channel)))

void psx_bus_init(psx_bus_t *bus, psx_memory_t *memory)
{
    if (bus == NULL) return;
    memset(bus, 0, sizeof(*bus));
    bus->memory = memory;
    bus->gpu_gp1_status = 0x14802000u;
}

void psx_bus_gpu_gp0_write(psx_bus_t *bus, uint32_t value)
{
    uint32_t command;
    if (bus == NULL) return;
    command = value >> 24;
    bus->gpu_gp0_write_count++;
    /* GP0 drawing-environment commands (E1-E6). Payload interpretation is partial. */
    switch (command) {
    case 0xe1u: bus->gpu_draw_mode = value & 0x00ffffffu; break;
    case 0xe2u: bus->gpu_texture_window = value & 0x00ffffffu; break;
    case 0xe3u: bus->gpu_draw_top_left = value & 0x000fffffu; break;
    case 0xe4u: bus->gpu_draw_bottom_right = value & 0x000fffffu; break;
    case 0xe5u: bus->gpu_draw_offset = value & 0x003fffffu; break;
    case 0xe6u: bus->gpu_mask_setting = value & 3u; break;
    default: break; /* Primitive/FIFO execution is not implemented yet. */
    }
}

static bool bus_word_only_mmio(uint32_t address)
{
    return address == GPU_GP0 || address == GPU_GP1 ||
           address == IRQ_STATUS || address == IRQ_MASK ||
           (address >= TIMER_BASE && address < TIMER_BASE + 0x30u) ||
           (address >= DMA_BASE && address < DMA_BASE + 0x70u) ||
           address == DPCR_ADDR || address == DICR_ADDR;
}

bool psx_bus_read8(psx_bus_t *bus, uint32_t address, uint8_t *value)
{
    if (bus == NULL || value == NULL || bus_word_only_mmio(address)) return false;
    return bus->memory != NULL && psx_memory_read8(bus->memory, address, value);
}

bool psx_bus_read16(psx_bus_t *bus, uint32_t address, uint16_t *value)
{
    if (bus == NULL || value == NULL || (address & 1u) != 0u ||
        bus_word_only_mmio(address)) return false;
    return bus->memory != NULL && psx_memory_read16(bus->memory, address, value);
}

bool psx_bus_write8(psx_bus_t *bus, uint32_t address, uint8_t value)
{
    if (bus == NULL || bus_word_only_mmio(address)) return false;
    return bus->memory != NULL && psx_memory_write8(bus->memory, address, value);
}

bool psx_bus_write16(psx_bus_t *bus, uint32_t address, uint16_t value)
{
    if (bus == NULL || (address & 1u) != 0u || bus_word_only_mmio(address)) return false;
    return bus->memory != NULL && psx_memory_write16(bus->memory, address, value);
}

bool psx_bus_read32(psx_bus_t *bus, uint32_t address, uint32_t *value)
{
    uint32_t offset, channel, reg;
    if (bus == NULL || value == NULL) return false;
    if (address == GPU_GP1) { *value = bus->gpu_gp1_status; return true; }
    if (address == IRQ_STATUS) { *value = bus->irq_status; return true; }
    if (address == IRQ_MASK) { *value = bus->irq_mask; return true; }
    if (address >= TIMER_BASE && address < TIMER_BASE + 0x30u) {
        offset = address - TIMER_BASE;
        channel = offset / 0x10u;
        reg = (offset % 0x10u) / 4u;
        if (channel >= PSX_BUS_TIMER_COUNT || (offset & 3u)) return false;
        *value = reg == 0u ? bus->timers[channel].count :
                 reg == 1u ? bus->timers[channel].mode :
                 reg == 2u ? bus->timers[channel].target : 0u;
        return reg <= 2u;
    }
    if (address >= DMA_BASE && address < DMA_BASE + 0x70u) {
        offset = address - DMA_BASE;
        channel = offset / 0x10u;
        reg = (offset % 0x10u) / 4u;
        if (channel >= PSX_BUS_DMA_CHANNELS || reg > 2u || (offset & 3u)) return false;
        *value = reg == 0u ? bus->dma[channel].madr :
                 reg == 1u ? bus->dma[channel].bcr : bus->dma[channel].chcr;
        return true;
    }
    if (address == DPCR_ADDR) { *value = bus->dma_control; return true; }
    if (address == DICR_ADDR) { *value = bus->dma_interrupt; return true; }
    return bus->memory != NULL && psx_memory_read32(bus->memory, address, value);
}

bool psx_bus_write32(psx_bus_t *bus, uint32_t address, uint32_t value)
{
    uint32_t offset, channel, reg;
    if (bus == NULL) return false;
    /* I_STAT is write-one-to-clear: only bits written as 1 are acknowledged. */
    if (address == GPU_GP0) { psx_bus_gpu_gp0_write(bus, value); return true; }
    if (address == GPU_GP1) {
        uint32_t command = value >> 24;
        bus->gpu_gp1_write_count++;
        switch (command) {
        case 0x00u: /* Reset GPU command state. */
            bus->gpu_draw_mode = 0u;
            bus->gpu_texture_window = 0u;
            bus->gpu_draw_top_left = 0u;
            bus->gpu_draw_bottom_right = 0u;
            bus->gpu_draw_offset = 0u;
            bus->gpu_mask_setting = 0u;
            bus->gpu_gp1_status = 0x14802000u;
            break;
        case 0x01u: /* Reset command buffer (FIFO is not emulated yet). */
            break;
        case 0x02u: /* Acknowledge GPU IRQ. */
            bus->gpu_gp1_status &= ~(1u << 24);
            break;
        case 0x03u: /* Display disable. */
            bus->gpu_gp1_status = (bus->gpu_gp1_status & ~(1u << 23)) |
                                  ((value & 1u) << 23);
            break;
        case 0x04u: /* DMA direction. */
            bus->gpu_gp1_status = (bus->gpu_gp1_status & ~(3u << 29)) |
                                  ((value & 3u) << 29);
            break;
        case 0x05u: /* Display start (stored in status model only). */
            bus->gpu_gp1_status = (bus->gpu_gp1_status & ~0x0007fffeu) |
                                  ((value & 0x000fffffu) << 1 & 0x0007fffeu);
            break;
        case 0x06u: case 0x07u: /* Horizontal/vertical ranges are not yet modeled. */
            break;
        case 0x08u: /* Display mode. */
            bus->gpu_gp1_status = (bus->gpu_gp1_status & ~0x007f0000u) |
                                  ((value & 0x3fu) << 17);
            break;
        default:
            break;
        }
        return true;
    }
    if (address == IRQ_STATUS) { bus->irq_status &= ~value; return true; }
    if (address == IRQ_MASK) { bus->irq_mask = value & 0x7ffu; return true; }
    if (address >= TIMER_BASE && address < TIMER_BASE + 0x30u) {
        offset = address - TIMER_BASE;
        channel = offset / 0x10u;
        reg = (offset % 0x10u) / 4u;
        if (channel >= PSX_BUS_TIMER_COUNT || reg > 2u || (offset & 3u)) return false;
        if (reg == 0u) bus->timers[channel].count = value & 0xffffu;
        else if (reg == 1u) {
            /* Writing MODE resets the counter and clears latched hit flags. */
            bus->timers[channel].mode = value & 0x3ffu;
            bus->timers[channel].count = 0u;
        } else bus->timers[channel].target = value & 0xffffu;
        return true;
    }
    if (address >= DMA_BASE && address < DMA_BASE + 0x70u) {
        offset = address - DMA_BASE;
        channel = offset / 0x10u;
        reg = (offset % 0x10u) / 4u;
        if (channel >= PSX_BUS_DMA_CHANNELS || reg > 2u || (offset & 3u)) return false;
        if (reg == 0u) bus->dma[channel].madr = value & 0x00ffffffu;
        else if (reg == 1u) bus->dma[channel].bcr = value;
        else bus->dma[channel].chcr = value & 0x71770703u;
        return true;
    }
    if (address == DPCR_ADDR) { bus->dma_control = value; return true; }
    if (address == DICR_ADDR) {
        /* Configuration bits are writable; completion flags are W1C. */
        bus->dma_interrupt = (value & 0x00ffffffu) |
                             (bus->dma_interrupt & 0x7f000000u & ~value) |
                             (bus->dma_interrupt & 0x80000000u);
        return true;
    }
    return bus->memory != NULL && psx_memory_write32(bus->memory, address, value);
}

static void dma_finish(psx_bus_t *bus, uint32_t channel)
{
    bus->dma[channel].chcr &= ~(1u << 24);
    bus->dma_interrupt |= DMA_IRQ_BIT(channel);
    if ((bus->dma_interrupt & (1u << 15)) != 0u ||
        ((bus->dma_interrupt & DMA_IRQ_ENABLE(channel)) != 0u &&
         (bus->dma_interrupt & (1u << 23)) != 0u)) {
        bus->dma_interrupt |= 1u << 31;
    }
    if ((bus->dma_interrupt & (1u << 31)) != 0u)
        bus->irq_status |= 1u << 3;
}

/* Manual-mode RAM-to-GPU DMA. Unsupported modes are stopped safely. */
static void dma_tick(psx_bus_t *bus)
{
    psx_dma_channel_t *dma;
    uint32_t words, i, address, value;
    if (bus == NULL || bus->memory == NULL) return;
    dma = &bus->dma[2];
    if ((dma->chcr & (1u << 24)) == 0u) return;
    if ((bus->dma_control & (1u << (2u * 4u + 3u))) == 0u) return;
    if ((dma->chcr & 1u) == 0u || ((dma->chcr >> 9) & 3u) != 0u) {
        dma_finish(bus, 2u);
        return;
    }
    words = dma->bcr & 0xffffu;
    if (words == 0u) words = 65536u;
    address = dma->madr & 0x00fffffcu;
    for (i = 0; i < words; ++i) {
        if (!psx_memory_read32(bus->memory, address, &value)) {
            dma_finish(bus, 2u);
            return;
        }
        psx_bus_gpu_gp0_write(bus, value);
        bus->dma_words_transferred++;
        address = (dma->chcr & (1u << 1)) != 0u ? address - 4u : address + 4u;
        address &= 0x00ffffffu;
    }
    dma->madr = address;
    dma->bcr &= 0xffff0000u;
    dma_finish(bus, 2u);
}

void psx_bus_tick(psx_bus_t *bus, uint32_t cycles)
{
    uint32_t i;
    if (bus == NULL) return;
    dma_tick(bus);
    for (i = 0; i < PSX_BUS_TIMER_COUNT; ++i) {
        psx_timer_t *timer = &bus->timers[i];
        uint32_t next = timer->count + cycles;
        bool target_hit = timer->target != 0u && next >= timer->target;
        bool overflow = next > 0xffffu;
        timer->count = next & 0xffffu;
        if (target_hit) {
            timer->mode |= 1u << 11;
            if (timer->mode & (1u << 4)) bus->irq_status |= 1u << (4u + i);
            if (timer->mode & (1u << 3)) timer->count = 0u;
        }
        if (overflow) {
            timer->mode |= 1u << 12;
            if (timer->mode & (1u << 5)) bus->irq_status |= 1u << (4u + i);
        }
    }
}
