/**
 * @file sercom-tools.c
 * @desc A set of utility functions used in all SERCOM modes
 * @author Samuel Dewan
 * @date 2018-12-27
 * Last Author:
 * Last Edited On:
 */

#include "sercom-tools.h"

uint8_t sercom_calc_async_baud (const uint32_t baudrate, const uint32_t clock,
                                volatile uint16_t *baud, uint8_t *sampr)
{
    /* Find best sample rate */
    uint8_t sample_rate = 0;
    if ((baudrate * 16) <= clock) {
        sample_rate = 16;
        *sampr = 0x0;
    } else if ((baudrate * 8) <= clock) {
        sample_rate = 8;
        *sampr = 0x2;
    } else if ((baudrate * 3) <= clock) {
        sample_rate = 3;
        *sampr = 0x4;
    } else {
        // Unsupported baud rate
        return 1;
    }
    
    /* Calculate baud value */
    uint64_t temp1 = ((sample_rate * (uint64_t)baudrate) << 32);
    uint64_t ratio = temp1 / clock;
    uint64_t scale = ((uint64_t)1 << 32) - ratio;
    *baud = (65536 * scale) >> 32;
    
    return 0;
}

uint8_t sercom_calc_sync_baud (const uint32_t baudrate, const uint32_t clock,
                               volatile uint8_t *baud)
{
    if ((baudrate * 2) > clock) {
        return 1;
    }
    
    // Get baud value for baud rate, or next lowest supported baud rate if
    // requested rate is not possible
    *baud = (clock - 1) / (2 * baudrate);
    
    return 0;
}

int8_t sercom_get_inst_num (Sercom *const inst)
{
    Sercom *sercom_instances[SERCOM_INST_NUM] = SERCOM_INSTS;
    
    for (int i = 0; i < SERCOM_INST_NUM; i++) {
        if ((uintptr_t)sercom_instances[i] == (uintptr_t)inst) {
            return i;
        }
    }
    
    return -1;
}

void sercom_enable_interrupts(uint8_t inst_num, uint8_t interrupt_mask)
{
#if defined(SAMD2x)
    static uint8_t const sercom_irqs[SERCOM_INST_NUM] = {
#ifdef ID_SERCOM0
        SERCOM0_IRQn,
#endif
#ifdef ID_SERCOM1
        SERCOM1_IRQn,
#endif
#ifdef ID_SERCOM2
        SERCOM2_IRQn,
#endif
#ifdef ID_SERCOM3
        SERCOM3_IRQn,
#endif
#ifdef ID_SERCOM4
        SERCOM4_IRQn,
#endif
#ifdef ID_SERCOM5
        SERCOM5_IRQn,
#endif
    };
    if (interrupt_mask != 0) {
        NVIC_SetPriority(sercom_irqs[inst_num], SERCOM_IRQ_PRIORITY);
        NVIC_EnableIRQ(sercom_irqs[inst_num]);
    }
#elif defined(SAMx5x)
    static uint8_t const sercom_irqs[SERCOM_INST_NUM][4] = {
#ifdef ID_SERCOM0
        {SERCOM0_0_IRQn, SERCOM0_1_IRQn, SERCOM0_2_IRQn, SERCOM0_3_IRQn},
#endif
#ifdef ID_SERCOM1
        {SERCOM1_0_IRQn, SERCOM1_1_IRQn, SERCOM1_2_IRQn, SERCOM1_3_IRQn},
#endif
#ifdef ID_SERCOM2
        {SERCOM2_0_IRQn, SERCOM2_1_IRQn, SERCOM2_2_IRQn, SERCOM2_3_IRQn},
#endif
#ifdef ID_SERCOM3
        {SERCOM3_0_IRQn, SERCOM3_1_IRQn, SERCOM3_2_IRQn, SERCOM3_3_IRQn},
#endif
#ifdef ID_SERCOM4
        {SERCOM4_0_IRQn, SERCOM4_1_IRQn, SERCOM4_2_IRQn, SERCOM4_3_IRQn},
#endif
#ifdef ID_SERCOM5
        {SERCOM5_0_IRQn, SERCOM5_1_IRQn, SERCOM5_2_IRQn, SERCOM5_3_IRQn},
#endif
#ifdef ID_SERCOM6
        {SERCOM6_0_IRQn, SERCOM6_1_IRQn, SERCOM6_2_IRQn, SERCOM6_3_IRQn},
#endif
#ifdef ID_SERCOM7
        {SERCOM7_0_IRQn, SERCOM7_1_IRQn, SERCOM7_2_IRQn, SERCOM7_3_IRQn},
#endif
    };

    if (interrupt_mask & SERCOM_USART_INTFLAG_DRE) {
        NVIC_SetPriority(sercom_irqs[inst_num][0], SERCOM_IRQ_PRIORITY);
        NVIC_EnableIRQ(sercom_irqs[inst_num][0]);
    }
    if (interrupt_mask & SERCOM_USART_INTFLAG_TXC) {
        NVIC_SetPriority(sercom_irqs[inst_num][1], SERCOM_IRQ_PRIORITY);
        NVIC_EnableIRQ(sercom_irqs[inst_num][1]);
    }
    if (interrupt_mask & SERCOM_USART_INTFLAG_RXC) {
        NVIC_SetPriority(sercom_irqs[inst_num][2], SERCOM_IRQ_PRIORITY);
        NVIC_EnableIRQ(sercom_irqs[inst_num][2]);
    }
    if (interrupt_mask & (SERCOM_USART_INTFLAG_RXS |
                          SERCOM_USART_INTFLAG_CTSIC |
                          SERCOM_USART_INTFLAG_RXBRK |
                          SERCOM_USART_INTFLAG_ERROR)) {
        NVIC_SetPriority(sercom_irqs[inst_num][3], SERCOM_IRQ_PRIORITY);
        NVIC_EnableIRQ(sercom_irqs[inst_num][3]);
    }
#endif
}

/*
 *  Interrupt service routines
 *  These ISRs allow some context (the sercom instance) to be injected in to an
 *  a more generic handler function.
 */

struct sercom_handler_t sercom_handlers[SERCOM_INST_NUM];

#if defined(SAMD2x)

#define SERCOM_HANDLER(isr, sercom, num) RAMFUNC void isr (void)\
{\
    if (sercom->USART.INTFLAG.bit.DRE && sercom->USART.INTENSET.bit.DRE) {\
        sercom_handlers[num].dre_handler(sercom, num, sercom_handlers[num].state);\
    }\
    if (sercom->USART.INTFLAG.bit.TXC && sercom->USART.INTENSET.bit.TXC) {\
        sercom_handlers[num].txc_handler(sercom, num, sercom_handlers[num].state);\
    }\
    if (sercom->USART.INTFLAG.bit.RXC && sercom->USART.INTENSET.bit.RXC) {\
        sercom_handlers[num].rxc_handler(sercom, num, sercom_handlers[num].state);\
    }\
    if ((sercom->USART.INTFLAG.bit.RXS && sercom->USART.INTENSET.bit.RXS) ||\
        (sercom->USART.INTFLAG.bit.CTSIC && sercom->USART.INTENSET.bit.CTSIC) ||\
        (sercom->USART.INTFLAG.bit.RXBRK && sercom->USART.INTENSET.bit.RXBRK) ||\
        (sercom->USART.INTFLAG.bit.ERROR && sercom->USART.INTENSET.bit.ERROR)) {\
        sercom_handlers[num].misc_handler(sercom, num, sercom_handlers[num].state);\
    }\
}

#ifdef ID_SERCOM0
SERCOM_HANDLER(SERCOM0_Handler, SERCOM0, 0)
#endif

#ifdef ID_SERCOM1
SERCOM_HANDLER(SERCOM1_Handler, SERCOM1, 1)
#endif

#ifdef ID_SERCOM2
SERCOM_HANDLER(SERCOM2_Handler, SERCOM2, 2)
#endif

#ifdef ID_SERCOM3
SERCOM_HANDLER(SERCOM3_Handler, SERCOM3, 3)
#endif

#ifdef ID_SERCOM4
SERCOM_HANDLER(SERCOM4_Handler, SERCOM4, 4)
#endif

#ifdef ID_SERCOM5
SERCOM_HANDLER(SERCOM5_Handler, SERCOM5, 5)
#endif

#elif defined(SAMx5x)

#ifdef ID_SERCOM0
RAMFUNC void SERCOM0_0_Handler (void)
{
    sercom_handlers[0].dre_handler(SERCOM0, 0, sercom_handlers[0].state);
}

RAMFUNC void SERCOM0_1_Handler (void)
{
    sercom_handlers[0].txc_handler(SERCOM0, 0, sercom_handlers[0].state);
}

RAMFUNC void SERCOM0_2_Handler (void)
{
    sercom_handlers[0].rxc_handler(SERCOM0, 0, sercom_handlers[0].state);
}

RAMFUNC void SERCOM0_3_Handler (void)
{
    sercom_handlers[0].misc_handler(SERCOM0, 0, sercom_handlers[0].state);
}
#endif


#ifdef ID_SERCOM1
RAMFUNC void SERCOM1_0_Handler (void)
{
    sercom_handlers[1].dre_handler(SERCOM1, 1, sercom_handlers[1].state);
}

RAMFUNC void SERCOM1_1_Handler (void)
{
    sercom_handlers[1].txc_handler(SERCOM1, 1, sercom_handlers[1].state);
}

RAMFUNC void SERCOM1_2_Handler (void)
{
    sercom_handlers[1].rxc_handler(SERCOM1, 1, sercom_handlers[1].state);
}

RAMFUNC void SERCOM1_3_Handler (void)
{
    sercom_handlers[1].misc_handler(SERCOM1, 1, sercom_handlers[1].state);
}
#endif


#ifdef ID_SERCOM2
RAMFUNC void SERCOM2_0_Handler (void)
{
    sercom_handlers[2].dre_handler(SERCOM2, 2, sercom_handlers[2].state);
}

RAMFUNC void SERCOM2_1_Handler (void)
{
    sercom_handlers[2].txc_handler(SERCOM2, 2, sercom_handlers[2].state);
}

RAMFUNC void SERCOM2_2_Handler (void)
{
    sercom_handlers[2].rxc_handler(SERCOM2, 2, sercom_handlers[2].state);
}

RAMFUNC void SERCOM2_3_Handler (void)
{
    sercom_handlers[2].misc_handler(SERCOM2, 2, sercom_handlers[2].state);
}
#endif


#ifdef ID_SERCOM3
RAMFUNC void SERCOM3_0_Handler (void)
{
    sercom_handlers[3].dre_handler(SERCOM3, 3, sercom_handlers[3].state);
}

RAMFUNC void SERCOM3_1_Handler (void)
{
    sercom_handlers[3].txc_handler(SERCOM3, 3, sercom_handlers[3].state);
}

RAMFUNC void SERCOM3_2_Handler (void)
{
    sercom_handlers[3].rxc_handler(SERCOM3, 3, sercom_handlers[3].state);
}

RAMFUNC void SERCOM3_3_Handler (void)
{
    sercom_handlers[3].misc_handler(SERCOM3, 3, sercom_handlers[3].state);
}
#endif


#ifdef ID_SERCOM4
RAMFUNC void SERCOM4_0_Handler (void)
{
    sercom_handlers[4].dre_handler(SERCOM4, 4, sercom_handlers[4].state);
}

RAMFUNC void SERCOM4_1_Handler (void)
{
    sercom_handlers[4].txc_handler(SERCOM4, 4, sercom_handlers[4].state);
}

RAMFUNC void SERCOM4_2_Handler (void)
{
    sercom_handlers[4].rxc_handler(SERCOM4, 4, sercom_handlers[4].state);
}

RAMFUNC void SERCOM4_3_Handler (void)
{
    sercom_handlers[4].misc_handler(SERCOM4, 4, sercom_handlers[4].state);
}
#endif


#ifdef ID_SERCOM5
RAMFUNC void SERCOM5_0_Handler (void)
{
    sercom_handlers[5].dre_handler(SERCOM5, 5, sercom_handlers[5].state);
}

RAMFUNC void SERCOM5_1_Handler (void)
{
    sercom_handlers[5].txc_handler(SERCOM5, 5, sercom_handlers[5].state);
}

RAMFUNC void SERCOM5_2_Handler (void)
{
    sercom_handlers[5].rxc_handler(SERCOM5, 5, sercom_handlers[5].state);
}

RAMFUNC void SERCOM5_3_Handler (void)
{
    sercom_handlers[5].misc_handler(SERCOM5, 5, sercom_handlers[5].state);
}
#endif


#ifdef ID_SERCOM6
RAMFUNC void SERCOM6_0_Handler (void)
{
    sercom_handlers[6].dre_handler(SERCOM6, 6, sercom_handlers[6].state);
}

RAMFUNC void SERCOM6_1_Handler (void)
{
    sercom_handlers[6].txc_handler(SERCOM6, 6, sercom_handlers[6].state);
}

RAMFUNC void SERCOM6_2_Handler (void)
{
    sercom_handlers[6].rxc_handler(SERCOM6, 6, sercom_handlers[6].state);
}

RAMFUNC void SERCOM6_3_Handler (void)
{
    sercom_handlers[6].misc_handler(SERCOM6, 6, sercom_handlers[6].state);
}
#endif


#ifdef ID_SERCOM7
RAMFUNC void SERCOM7_0_Handler (void)
{
    sercom_handlers[7].dre_handler(SERCOM7, 7, sercom_handlers[7].state);
}

RAMFUNC void SERCOM7_1_Handler (void)
{
    sercom_handlers[7].txc_handler(SERCOM7, 7, sercom_handlers[7].state);
}

RAMFUNC void SERCOM7_2_Handler (void)
{
    sercom_handlers[7].rxc_handler(SERCOM7, 7, sercom_handlers[7].state);
}

RAMFUNC void SERCOM7_3_Handler (void)
{
    sercom_handlers[7].misc_handler(SERCOM7, 7, sercom_handlers[7].state);
}
#endif

#endif // SAMx5x
