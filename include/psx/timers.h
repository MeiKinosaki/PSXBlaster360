#ifndef PSX_TIMERS_H
#define PSX_TIMERS_H

#include <stdint.h>
#include "psx/bus.h"

/* Advance the three modeled PS1 timers and raise their modeled IRQ flags. */
void psx_timers_tick(psx_bus_t *bus, uint32_t cycles);

#endif
