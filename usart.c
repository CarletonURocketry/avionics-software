//
//  usart.c
//  index
//
//  Created by Samuel Dewan on 2018-12-26.
//  Copyright © 2018 Samuel Dewan. All rights reserved.
//

#include "usart.h"

#include "sercom-tools.h"

static uint8_t sercom_calc_baudrate (const uint32_t baudrate,
                                     const uint32_t clock, uint16_t *baud,
                                     uint8_t *sampr)
{
    // Find best sample rate
    uint8_t sample_rate = 0;
    if ((baudrate * 16) <= clock) {
        sample_rate = 16;
        *sampr = 0x0;
    } else if ((baudrate * 16) <= clock) {
        sample_rate = 8;
        *sampr = 0x2;
    } else if ((baudrate * 16) <= clock) {
        sample_rate = 3;
        *sampr = 0x4;
    } else {
        // Unsupported baud rate
        return 1;
    }
    
    // Calculate baud value
    uint64_t temp1 = ((sample_rate * (uint64_t)baudrate) << 32);
    uint64_t ratio = temp1 / clock;
    uint64_t scale = ((uint64_t)1 << 32) - ratio;
    *baud = (65536 * scale) >> 32;
    
    return 0;
}

static const char *out_buffer;
static uint16_t out_length;
static uint16_t out_position;

static void uart_test_isr (Sercom *sercom, uint8_t inst_num, void *state)
{
    if (sercom->USART.INTFLAG.bit.DRE && sercom->USART.INTENSET.bit.DRE) {
        if (out_position < out_length) {
            // Send next char
            sercom->USART.DATA.reg = out_buffer[out_position];
            out_position++;
        } else {
            // All chars sent, disable DRE interupt
            sercom->USART.INTENCLR.bit.DRE = 0b1;
        }
    }
    
    if (sercom->USART.INTFLAG.bit.RXC) {
        char data = sercom->USART.DATA.reg;
        while (!sercom->USART.INTFLAG.bit.DRE);
        sercom->USART.DATA.reg = data;
        
        if (data == '\r') {
            while (!sercom->USART.INTFLAG.bit.DRE);
            sercom->USART.DATA.reg = '\n';
        }
    }
    
    
    // For some reason the RXC interupt seems to get disabled every time the
    // interupt service routine runs. Not clear why this happens.
    sercom->USART.INTENSET.bit.RXC = 0b1;
}

void init_sercom_usart (Sercom *inst, uint32_t baudrate)
{
    // Reset SERCOM instance
    inst->USART.CTRLA.bit.SWRST = 0b1;
    // Wait for reset to complete
    while (inst->USART.SYNCBUSY.bit.SWRST);
    
    // Find baud setting
    uint16_t baud = 0;
    uint8_t sampr = 0;
    sercom_calc_baudrate(baudrate, F_CPU, &baud, &sampr);
    
    /* Configure */
    // Internal clock, asynchronous mode, choose RX and TX pins, sample rate,
    // LSB first, run in standby
    inst->USART.CTRLA.reg = (SERCOM_USART_CTRLA_MODE_USART_INT_CLK |
                             SERCOM_USART_CTRLA_RXPO(0x1) |
                             SERCOM_USART_CTRLA_TXPO(0x0) |
                             SERCOM_USART_CTRLA_SAMPR(sampr) |
                             SERCOM_USART_CTRLA_DORD |
                             SERCOM_USART_CTRLA_RUNSTDBY);
    /* Set baudrate */
    inst->USART.BAUD.USARTFP.BAUD = baud;
    // 8 bit chars, one stop bit, enable reciever and transmitter
    inst->USART.CTRLB.reg = (SERCOM_USART_CTRLB_CHSIZE(0x0) |
                             SERCOM_USART_CTRLB_RXEN |
                             SERCOM_USART_CTRLB_TXEN);
    /* Wait for synchronization */
    while(inst->USART.SYNCBUSY.bit.CTRLB);
    
    /* Configure interupts */
    inst->USART.INTENSET.bit.RXC = 0b1; // RX Complete
    
    int8_t inst_num = sercom_get_inst_num(inst);
    
    sercom_handlers[inst_num] = (struct sercom_handler_t) {
        .handler = uart_test_isr,
        .state = (void*)0
    };
    
    NVIC_EnableIRQ(sercom_get_irq_num(inst_num));
    
    /* Enable SERCOM instance */
    inst->USART.CTRLA.bit.ENABLE = 0b1;
}

void sercom_usart_send (Sercom *const inst, const char *data)
{
//    for (const char *c = data; *c != '\0'; c++) {
//        // Wait for SERCOM to be ready for new data
//        while (!inst->USART.INTFLAG.bit.DRE);
//        // Write data to data register
//        inst->USART.DATA.bit.DATA = *c;
//    }
    
    out_buffer = data;
    out_position = 0;
    
    out_length = 0;
    for (const char *c = data; *c != '\0'; c++) {
        out_length++;
    }
    
    // Wait for SERCOM to be ready for new data
    //while (!inst->USART.INTFLAG.bit.DRE);
    // Write first char
    //inst->USART.DATA.bit.DATA = out_buffer[0];
    // Enable DRE interupt in order to send the rest of the string in the ISR
    inst->USART.INTENSET.bit.DRE = 0b1;
}
