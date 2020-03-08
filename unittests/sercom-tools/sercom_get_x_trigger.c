#include <unittest.h>
#include SOURCE_H

/*
 *  Tests for sercom_get_dma_rx_trigger() and sercom_get_dma_tx_trigger(). These
 *  functions provide DMA trigger numbers for a given SERCOM instance.
 */


int main (int argc, char **argv)
{
    /* sercom_get_dma_rx_trigger() */
    // Get DMA RX trigger id for SERCOM0.
    {
        uint32_t id = sercom_get_dma_rx_trigger(0);

        ut_assert(id == SERCOM0_DMAC_ID_RX);
    }

    // Get DMA RX trigger id for SERCOM1.
    {
        uint32_t id = sercom_get_dma_rx_trigger(1);

        ut_assert(id == SERCOM1_DMAC_ID_RX);
    }

    // Get DMA RX trigger id for SERCOM2.
    {
        uint32_t id = sercom_get_dma_rx_trigger(2);

        ut_assert(id == SERCOM2_DMAC_ID_RX);
    }

    // Get DMA RX trigger id for SERCOM3.
    {
        uint32_t id = sercom_get_dma_rx_trigger(3);

        ut_assert(id == SERCOM3_DMAC_ID_RX);
    }

    // Get DMA RX trigger id for SERCOM4.
    {
        uint32_t id = sercom_get_dma_rx_trigger(4);

        ut_assert(id == SERCOM4_DMAC_ID_RX);
    }

    // Get DMA RX trigger id for SERCOM5.
    {
        uint32_t id = sercom_get_dma_rx_trigger(5);

        ut_assert(id == SERCOM5_DMAC_ID_RX);
    }

    // Get DMA RX trigger id for something that is not a valid SERCOM id.
    // Returned value should be 0;
    {
        uint32_t id = sercom_get_dma_rx_trigger(64);

        ut_assert(id == 0);
    }

    /* sercom_get_dma_tx_trigger() */
    // Get DMA TX trigger id for SERCOM0.
    {
        uint32_t id = sercom_get_dma_tx_trigger(0);

        ut_assert(id == SERCOM0_DMAC_ID_TX);
    }

    // Get DMA TX trigger id for SERCOM1.
    {
        uint32_t id = sercom_get_dma_tx_trigger(1);

        ut_assert(id == SERCOM1_DMAC_ID_TX);
    }

    // Get DMA TX trigger id for SERCOM2.
    {
        uint32_t id = sercom_get_dma_tx_trigger(2);

        ut_assert(id == SERCOM2_DMAC_ID_TX);
    }

    // Get DMA TX trigger id for SERCOM3.
    {
        uint32_t id = sercom_get_dma_tx_trigger(3);

        ut_assert(id == SERCOM3_DMAC_ID_TX);
    }

    // Get DMA TX trigger id for SERCOM4.
    {
        uint32_t id = sercom_get_dma_tx_trigger(4);

        ut_assert(id == SERCOM4_DMAC_ID_TX);
    }

    // Get DMA TX trigger id for SERCOM5.
    {
        uint32_t id = sercom_get_dma_tx_trigger(5);

        ut_assert(id == SERCOM5_DMAC_ID_TX);
    }

    // Get DMA TX trigger id for something that is not a valid SERCOM id.
    // Returned value should be 0;
    {
        uint32_t id = sercom_get_dma_tx_trigger(64);

        ut_assert(id == 0);
    }

    return UT_PASS;
}
