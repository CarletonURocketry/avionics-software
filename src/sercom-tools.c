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
    
    // Get nearest integer baud value for given baud rate
    *baud = ((clock + baudrate) / (2 * baudrate)) - 1;
    
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

/*
 *  Interupt service routines
 *  These ISRs allow some context (the sercom instance) to be injected in to an
 *  a more generic handler function.
 */

struct sercom_handler_t sercom_handlers[SERCOM_INST_NUM];

void SERCOM0_Handler (void)
{
    sercom_handlers[0].handler(SERCOM0, 0, sercom_handlers[0].state);
}

void SERCOM1_Handler (void)
{
    sercom_handlers[1].handler(SERCOM1, 1, sercom_handlers[1].state);
}

void SERCOM2_Handler (void)
{
    sercom_handlers[2].handler(SERCOM2, 2, sercom_handlers[2].state);
}

void SERCOM3_Handler (void)
{
    sercom_handlers[3].handler(SERCOM3, 3, sercom_handlers[3].state);
}

#ifdef ID_SERCOM4
void SERCOM4_Handler (void)
{
    sercom_handlers[4].handler(SERCOM4, 4, sercom_handlers[4].state);
}
#endif

#ifdef ID_SERCOM5
void SERCOM5_Handler (void)
{
    sercom_handlers[5].handler(SERCOM5, 5, sercom_handlers[5].state);
}
#endif
