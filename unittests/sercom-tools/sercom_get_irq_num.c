#include <unittest.h>
#include SOURCE_H

/*
 *  Tests for sercom_get_irq_num(), this function provides the interrupt request
 *  number for a given SERCOM instance.
 */


int main (int argc, char **argv)
{
    // Get IRQ for SERCOM0.
    {
        IRQn_Type irq = sercom_get_irq_num(0);

        ut_assert(irq == SERCOM0_IRQn);
    }

    // Get IRQ for SERCOM1.
    {
        IRQn_Type irq = sercom_get_irq_num(1);

        ut_assert(irq == SERCOM1_IRQn);
    }

    // Get IRQ for SERCOM2.
    {
        IRQn_Type irq = sercom_get_irq_num(2);

        ut_assert(irq == SERCOM2_IRQn);
    }

    // Get IRQ for SERCOM3.
    {
        IRQn_Type irq = sercom_get_irq_num(3);

        ut_assert(irq == SERCOM3_IRQn);
    }

    // Get IRQ for SERCOM4.
    {
        IRQn_Type irq = sercom_get_irq_num(4);

        ut_assert(irq == SERCOM4_IRQn);
    }

    // Get IRQ for SERCOM5.
    {
        IRQn_Type irq = sercom_get_irq_num(5);

        ut_assert(irq == SERCOM5_IRQn);
    }

    // Get IRQ for something that is not a valid SERCOM id. Returned value
    // should not be a valid IRQ.
    {
        IRQn_Type irq = sercom_get_irq_num(64);

        ut_assert(irq >= PERIPH_COUNT_IRQn);
    }

    return UT_PASS;
}
