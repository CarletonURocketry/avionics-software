#include <unittest.h>

static enum {
    INTERRUPTS_ENABLED,
    INTERRUPTS_DISABLED,
    INTERRUPTS_CYCLED
} interrupts_status;

static inline void my_disable_irq(void)
{
    ut_assert(interrupts_status == INTERRUPTS_ENABLED);
    interrupts_status = INTERRUPTS_DISABLED;
}

static inline void my_enable_irq(void)
{
    ut_assert(interrupts_status == INTERRUPTS_DISABLED);
    interrupts_status = INTERRUPTS_CYCLED;
}

#define __disable_irq my_disable_irq
#define __enable_irq my_enable_irq
#include SOURCE_H
#undef __disable_irq
#undef __enable_irq

