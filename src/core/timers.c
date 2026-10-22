#include "psx/timers.h"

#include <stddef.h>

/* Timer mode bits modeled here: reset-at-target, IRQ-at-target and IRQ-on-overflow. */
void psx_timers_tick(psx_bus_t *bus, uint32_t cycles)
{
    uint32_t i;
    if (bus == NULL) return;
    for (i = 0; i < PSX_BUS_TIMER_COUNT; ++i) {
        psx_timer_t *timer = &bus->timers[i];
        uint64_t next = (uint64_t)timer->count + (uint64_t)cycles;
        bool reset_at_target = (timer->mode & (1u << 3)) != 0u &&
                               timer->target != 0u;
        bool target_hit = timer->target != 0u && next >= timer->target;
        bool overflow = next > 0xffffu && !reset_at_target;
        timer->count = (uint32_t)(next & 0xffffu);
        if (target_hit) {
            timer->mode |= 1u << 11;
            if ((timer->mode & (1u << 4)) != 0u)
                bus->irq_status |= 1u << (4u + i);
            /* Preserve cycles beyond the target in reset-at-target mode. */
            if (reset_at_target) timer->count = (uint32_t)(next % timer->target);
        }
        if (overflow) {
            timer->mode |= 1u << 12;
            if ((timer->mode & (1u << 5)) != 0u)
                bus->irq_status |= 1u << (4u + i);
        }
    }
}
