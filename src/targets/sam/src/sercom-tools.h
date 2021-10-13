/**
 * @file sercom-tools.h
 * @desc A set of utility functions used in all SERCOM modes
 * @author Samuel Dewan
 * @date 2018-12-27
 * Last Author:
 * Last Edited On:
 */

#ifndef sercom_tools_h
#define sercom_tools_h

#include "global.h"

#define SERCOM_IRQ_PRIORITY     1

#define SERCOM_DMA_RX_PRIORITY  2
#define SERCOM_DMA_TX_PRIORITY  1


// These macros are defined by the SAMD21 headers but not the SAME54 headers for
// some reason
#ifndef SERCOM_USART_CTRLA_MODE_USART_EXT_CLK
#define SERCOM_USART_CTRLA_MODE_USART_EXT_CLK   (0x0 << SERCOM_USART_CTRLA_MODE_Pos)
#endif
#ifndef SERCOM_USART_CTRLA_MODE_USART_INT_CLK
#define SERCOM_USART_CTRLA_MODE_USART_INT_CLK   (0x1 << SERCOM_USART_CTRLA_MODE_Pos)
#endif
#ifndef SERCOM_SPI_CTRLA_MODE_SPI_SLAVE
#define SERCOM_SPI_CTRLA_MODE_SPI_SLAVE     (0x2 << SERCOM_SPI_CTRLA_MODE_Pos)
#endif
#ifndef SERCOM_SPI_CTRLA_MODE_SPI_MASTER
#define SERCOM_SPI_CTRLA_MODE_SPI_MASTER    (0x3 << SERCOM_SPI_CTRLA_MODE_Pos)
#endif
#ifndef SERCOM_I2CS_CTRLA_MODE_I2C_SLAVE
#define SERCOM_I2CS_CTRLA_MODE_I2C_SLAVE    (0x4 << SERCOM_I2CS_CTRLA_MODE_Pos)
#endif
#ifndef SERCOM_I2CM_CTRLA_MODE_I2C_MASTER
#define SERCOM_I2CM_CTRLA_MODE_I2C_MASTER   (0x5 << SERCOM_I2CM_CTRLA_MODE_Pos)
#endif


/**
 *  Abstraction over sercom interrupt handlers which allows a callback to be
 *  specified for each interrupt which is provided context about the SERCOM
 *  instance as well as a pointer to any other context information.
 *  This allows the same function to be used as an interrupt handler on any
 *  SERCOM instance.
 */
extern struct sercom_handler_t {
    // SERCOMn_0_Handler -> Bit 0
    //      USART: DRE
    //      SPI: DRE
    //      I2CM: MB
    //      I2CS: PREC
    union {
        void (*dre_handler)(Sercom*, uint8_t, void*);
        void (*mb_handler)(Sercom*, uint8_t, void*);
        void (*prec_handler)(Sercom*, uint8_t, void*);
    };
    // SERCOMn_1_Handler -> Bit 1
    //      USART: TXC
    //      SPI: TXC
    //      I2CM: SB
    //      I2CS: AMATCH
    union {
        void (*txc_handler)(Sercom*, uint8_t, void*);
        void (*sb_handler)(Sercom*, uint8_t, void*);
        void (*amatch_handler)(Sercom*, uint8_t, void*);
    };
    // SERCOMn_2_Handler -> Bit 2
    //      USART: RXC
    //      SPI: RXC
    //      I2CS: DRDY
    union {
        void (*rxc_handler)(Sercom*, uint8_t, void*);
        void (*drdy_handler)(Sercom*, uint8_t, void*);
    };
    // SERCOMn_3_Handler -> Bit 3, Bit 4, Bit 5, Bit 6/7
    //      USART: RXS, CTSIC, RXBRK, ERROR
    //      SPI: SSL, ERROR
    //      I2CM: ERROR
    //      I2CS: ERROR
    void (*misc_handler)(Sercom*, uint8_t, void*);

    void *state;
} sercom_handlers[SERCOM_INST_NUM];


/**
 *  Calculate the baud register value for a given baudrate.
 *
 *  @param baudrate The target baudrate
 *  @param clock The speed of the SERCOMs core clock
 *  @param baud Location where the calculated baud value will be stored
 *  @param sampr Location where the calculated sample rate setting will be
 *               stored
 *
 *  @return 0 if successful, 1 if baud rate not supported
 */
extern uint8_t sercom_calc_async_baud (const uint32_t baudrate,
                                       const uint32_t clock,
                                       volatile uint16_t *baud, uint8_t *sampr);

/**
 *  Calculate the baud register value for a given baudrate.
 *
 *  @param baudrate The target baudrate
 *  @param clock The speed of the SERCOMs core clock
 *  @param baud Location where the calculated baud value will be stored
 *
 *  @return 0 if successful, 1 if baud rate not supported
 */
extern uint8_t sercom_calc_sync_baud (const uint32_t baudrate,
                                      const uint32_t clock,
                                      volatile uint8_t *baud);

/**
 *  Get the index of a SERCOM instance from it's register address
 *
 *  @param inst A pointer to a SERCOM instances registers
 *
 *  @return The index of the SERCOM, -1 if the pointer is not a valid SERCOM
 */
extern int8_t sercom_get_inst_num (Sercom *const inst) __attribute__((const));

/**
 *  Enable interrupts in the NVIC for a SERCOM instance.
 *
 *  @param inst_num Number of SERCOM instance for which interrupts should be
 *                  enabled
 *  @param interrupt_mask Mask of which interrupt vectors need to be enabled,
 *                        matches layout of INTFLAG register
 */
extern void sercom_enable_interrupts(uint8_t inst_num, uint8_t interrupt_mask);

/**
 *  Determine the generic clock id for a given SERCOM instance number.
 *
 *  @param inst_num The index of a SERCOM instance
 *
 *  @return The generic genric clock id for the given SERCOM or 0 if the
 *          instance number is not valid
 */
static inline enum peripheral_generic_clock sercom_get_gclk (uint8_t inst_num)
{
    switch (inst_num) {
#ifdef ID_SERCOM0
        case 0:
            return PERPH_GCLK_SERCOM0_CORE;
#endif
#ifdef ID_SERCOM1
        case 1:
            return PERPH_GCLK_SERCOM1_CORE;
#endif
#ifdef ID_SERCOM2
        case 2:
            return PERPH_GCLK_SERCOM2_CORE;
#endif
#ifdef ID_SERCOM3
        case 3:
            return PERPH_GCLK_SERCOM3_CORE;
#endif
#ifdef ID_SERCOM4
        case 4:
            return PERPH_GCLK_SERCOM4_CORE;
#endif
#ifdef ID_SERCOM5
        case 5:
            return PERPH_GCLK_SERCOM5_CORE;
#endif
#ifdef ID_SERCOM6
        case 6:
            return PERPH_GCLK_SERCOM6_CORE;
#endif
#ifdef ID_SERCOM7
        case 7:
            return PERPH_GCLK_SERCOM7_CORE;
#endif
        default:
            return 0;
    }
}

/**
 *  Determine the bus clock number for a given SERCOM instance.
 *
 *  @param inst_num The index of a SERCOM instance
 *
 *  @return The bus clock number for the SERCOM instance or 0 if the instance
 *          number is not valid
 */
static inline enum peripheral_bus_clock sercom_get_bus_clk (uint8_t inst_num)
{
    switch (inst_num) {
#ifdef ID_SERCOM0
        case 0:
            return PERPH_BUS_CLK_SERCOM0_APB;
#endif
#ifdef ID_SERCOM1
        case 1:
            return PERPH_BUS_CLK_SERCOM1_APB;
#endif
#ifdef ID_SERCOM2
        case 2:
            return PERPH_BUS_CLK_SERCOM2_APB;
#endif
#ifdef ID_SERCOM3
        case 3:
            return PERPH_BUS_CLK_SERCOM3_APB;
#endif
#ifdef ID_SERCOM4
        case 4:
            return PERPH_BUS_CLK_SERCOM4_APB;
#endif
#ifdef ID_SERCOM5
        case 5:
            return PERPH_BUS_CLK_SERCOM5_APB;
#endif
#ifdef ID_SERCOM6
        case 6:
            return PERPH_BUS_CLK_SERCOM6_APB;
#endif
#ifdef ID_SERCOM7
        case 7:
            return PERPH_BUS_CLK_SERCOM7_APB;
#endif
        default:
            return 0;
    }
}

/**
 *  Determine DMAC RX trigger id for a given SERCOM instance
 *
 *  @param inst_num The index of a SERCOM instance
 *
 *  @return The DMAC RX trigger id for the SERCOM instance or 0 if the instance
 *          number is not valid
 */
static inline uint32_t sercom_get_dma_rx_trigger (uint8_t inst_num)
{
    switch (inst_num) {
#ifdef ID_SERCOM0
        case 0:
            return SERCOM0_DMAC_ID_RX;
#endif
#ifdef ID_SERCOM1
        case 1:
            return SERCOM1_DMAC_ID_RX;
#endif
#ifdef ID_SERCOM2
        case 2:
            return SERCOM2_DMAC_ID_RX;
#endif
#ifdef ID_SERCOM3
        case 3:
            return SERCOM3_DMAC_ID_RX;
#endif
#ifdef ID_SERCOM4
        case 4:
            return SERCOM4_DMAC_ID_RX;
#endif
#ifdef ID_SERCOM5
        case 5:
            return SERCOM5_DMAC_ID_RX;
#endif
#ifdef ID_SERCOM6
        case 6:
            return SERCOM6_DMAC_ID_RX;
#endif
#ifdef ID_SERCOM7
        case 7:
            return SERCOM7_DMAC_ID_RX;
#endif
        default:
            return 0;
    }
}

/**
 *  Determine DMAC TX trigger id for a given SERCOM instance
 *
 *  @param inst_num The index of a SERCOM instance
 *
 *  @return The DMAC TX trigger id for the SERCOM instance or 0 if the instance
 *          number is not valid
 */
static inline uint32_t sercom_get_dma_tx_trigger (uint8_t inst_num)
{
    switch (inst_num) {
#ifdef ID_SERCOM0
        case 0:
            return SERCOM0_DMAC_ID_TX;
#endif
#ifdef ID_SERCOM1
        case 1:
            return SERCOM1_DMAC_ID_TX;
#endif
#ifdef ID_SERCOM2
        case 2:
            return SERCOM2_DMAC_ID_TX;
#endif
#ifdef ID_SERCOM3
        case 3:
            return SERCOM3_DMAC_ID_TX;
#endif
#ifdef ID_SERCOM4
        case 4:
            return SERCOM4_DMAC_ID_TX;
#endif
#ifdef ID_SERCOM5
        case 5:
            return SERCOM5_DMAC_ID_TX;
#endif
#ifdef ID_SERCOM6
        case 6:
            return SERCOM6_DMAC_ID_TX;
#endif
#ifdef ID_SERCOM7
        case 7:
            return SERCOM7_DMAC_ID_TX;
#endif
        default:
            return 0;
    }
}

#endif /* sercom_tools_h */
