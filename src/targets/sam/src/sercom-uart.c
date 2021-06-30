/**
 * @file sercom-uart.c
 * @desc SERCOM UART mode driver which allows interrupt or DMA driven transfers.
 * @author Samuel Dewan
 * @date 2018-12-27
 * Last Author:
 * Last Edited On:
 */

#include "sercom-uart.h"

#include "sercom-tools.h"
#include <string.h>
#include <ctype.h>

static void sercom_uart_isr_dre (Sercom *sercom, uint8_t inst_num, void *state);
static void sercom_uart_isr_rxc (Sercom *sercom, uint8_t inst_num, void *state);
static void sercom_uart_dma_callback (uint8_t chan, void *state);



void init_sercom_uart (struct sercom_uart_desc_t *descriptor, Sercom *sercom,
                       uint32_t baudrate, uint32_t core_freq,
                       uint32_t core_clock_mask, int8_t dma_channel,
                       uint8_t echo, uint8_t tx_pin_group, uint8_t tx_pin_num)
{
    uint8_t instance_num = sercom_get_inst_num(sercom);

    /* Enable the APBC clock for the SERCOM instance */
    enable_bus_clock(sercom_get_bus_clk(instance_num));

    /* Select the core clock for the SERCOM instance */
    set_perph_generic_clock(sercom_get_gclk(instance_num), core_clock_mask);

    /* Reset SERCOM instance */
    sercom->USART.CTRLA.bit.SWRST = 0b1;
    // Wait for reset to complete
    while (sercom->USART.SYNCBUSY.bit.SWRST);

    /* Find baud setting */
    uint16_t baud = 0;
    uint8_t sampr = 0;
    sercom_calc_async_baud(baudrate, core_freq, &baud, &sampr);

    /* Configure CTRL Reg A */
    // Internal clock, asynchronous mode, choose RX and TX pins, sample rate,
    // LSB first, run in standby
    sercom->USART.CTRLA.reg = (SERCOM_USART_CTRLA_MODE_USART_INT_CLK |
                               SERCOM_USART_CTRLA_RXPO(0x1) |
                               SERCOM_USART_CTRLA_TXPO(0x0) |
                               SERCOM_USART_CTRLA_SAMPR(sampr) |
                               SERCOM_USART_CTRLA_DORD |
                               SERCOM_USART_CTRLA_RUNSTDBY);
    /* Set baudrate */
    sercom->USART.BAUD.USARTFP.BAUD = baud;
    /* Configure CTRL Reg B */
    // 8 bit chars, one stop bit, enable receiver and transmitter
    sercom->USART.CTRLB.reg = (SERCOM_USART_CTRLB_CHSIZE(0x0) |
                               SERCOM_USART_CTRLB_RXEN |
                               SERCOM_USART_CTRLB_TXEN);
    // Wait for synchronization
    while(sercom->USART.SYNCBUSY.bit.CTRLB);

    /* Configure interrupts */
    sercom->USART.INTENSET.bit.RXC = 0b1; // RX Complete

    sercom_handlers[instance_num] = (struct sercom_handler_t) {
        .dre_handler = sercom_uart_isr_dre,
        .rxc_handler = sercom_uart_isr_rxc,
        .state = (void*)descriptor
    };

    sercom_enable_interupts(instance_num, (SERCOM_USART_INTFLAG_DRE |
                                           SERCOM_USART_INTFLAG_RXC));

    /* Setup Descriptor */
    descriptor->sercom = sercom;
    descriptor->sercom_instnum = instance_num;
    descriptor->echo = echo;

    // Configure buffers
    init_circular_buffer(&descriptor->out_buffer,
                         (uint8_t*)descriptor->out_buffer_mem,
                         SERCOM_UART_OUT_BUFFER_LEN);
    init_circular_buffer(&descriptor->in_buffer,
                         (uint8_t*)descriptor->in_buffer_mem,
                         SERCOM_UART_IN_BUFFER_LEN);

    // Configure DMA
    if ((dma_channel >= 0) && (dma_channel < DMAC_CH_NUM)) {
        descriptor->dma_chan = (uint8_t)dma_channel;
        descriptor->use_dma = 0b1;
        
        dma_callbacks[dma_channel] = (struct dma_callback_t) {
            .callback = sercom_uart_dma_callback,
            .state = (void*)descriptor
        };
    }

    // Configure break condition state
    descriptor->break_duration = 0;
    descriptor->break_pending = 0;

    // Store TX pin info
    descriptor->tx_pin_group = tx_pin_group;
    descriptor->tx_pin_num = tx_pin_num;

    /* Configure TX pin */
    // Set TX pin as output
    PORT->Group[tx_pin_group].DIRSET.reg = (1 << tx_pin_num);
    // Set TX pin output high
    PORT->Group[tx_pin_group].OUTSET.reg = (1 << tx_pin_num);

    /* Enable SERCOM instance */
    sercom->USART.CTRLA.bit.ENABLE = 0b1;
}

uint16_t sercom_uart_put_string(struct sercom_uart_desc_t *uart,
                                const char *str)
{
    uint16_t i = 0;
    for (; str[i] != '\0'; i++) {
        if (circular_buffer_is_full(&uart->out_buffer)) {
            break;
        }
        
        circular_buffer_push(&uart->out_buffer, (uint8_t)str[i]);
        
        if (uart->echo && (str[i] == '\n')) {
            // Add carriage return as some terminal emulators seem to think that
            // they are typewriters.
            circular_buffer_push(&uart->out_buffer, (uint8_t)'\r');
        }
    }
    
    // Make sure that we start transmission right away if there is no
    // transmission already in progress.
    sercom_uart_service(uart);
    
    return i;
}

void sercom_uart_put_string_blocking(struct sercom_uart_desc_t *uart,
                                     const char *str)
{
    uint8_t carriage_return = 0;
    
    for (const char *i = str; *i != '\0';) {
        // Wait for a character worth of space to become available in the buffer
        while (circular_buffer_is_full(&uart->out_buffer)) {
            // Make sure that we aren't waiting for a transaction which is not
            // in progress.
            sercom_uart_service(uart);
        }
        
        if (carriage_return) {
            // Push a carriage return
            circular_buffer_push(&uart->out_buffer, (uint8_t)'\r');
        } else {
            // Push the next character
            circular_buffer_push(&uart->out_buffer, (uint8_t)*i);
        }
        
        if (uart->echo && (*i == '\n') && !carriage_return) {
            // Add carriage return after newlines
            carriage_return = 1;
        } else {
            i++;
            carriage_return = 0;
        }
    }
    
    // Make sure that we start transmission right away if there is no
    // transmission already in progress.
    sercom_uart_service(uart);
}

uint16_t sercom_uart_put_bytes(struct sercom_uart_desc_t *uart,
                               const uint8_t *bytes, uint16_t length)
{
    uint16_t i = 0;
    for (; i < length; i++) {
        if (circular_buffer_is_full(&uart->out_buffer)) {
            break;
        }
        
        circular_buffer_push(&uart->out_buffer, (uint8_t)bytes[i]);
    }
    
    // Make sure that we start transmission right away if there is no
    // transmission already in progress.
    sercom_uart_service(uart);
    
    return i;
}

void sercom_uart_put_bytes_blocking(struct sercom_uart_desc_t *uart,
                                    const uint8_t *bytes, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++) {
        // Wait for a character worth of space to become available in the buffer
        while (circular_buffer_is_full(&uart->out_buffer)) {
            // Make sure that we aren't waiting for a transaction which is not
            // in progress.
            sercom_uart_service(uart);
        }
        
        circular_buffer_push(&uart->out_buffer, bytes[i]);
    }
    
    // Make sure that we start transmission right away if there is no
    // transmission already in progress.
    sercom_uart_service(uart);
}

void sercom_uart_put_char (struct sercom_uart_desc_t *uart, char c)
{
    circular_buffer_push(&uart->out_buffer, (uint8_t)c);
    
    if (uart->echo && (c == '\n')) {
        // Add carriage return as some terminal emulators seem to think that
        // they are typewriters.
        circular_buffer_push(&uart->out_buffer, (uint8_t)'\r');
    }
    
    // Make sure that we start transmission right away if there is no
    // transmission already in progress.
    sercom_uart_service(uart);
}

void sercom_uart_get_string (struct sercom_uart_desc_t *uart, char *str,
                             uint16_t len)
{
    for (uint16_t i = 0; i < (len - 1); i++) {
        uint8_t pop_failed = circular_buffer_pop(&uart->in_buffer,
                                                 (uint8_t*)(str + i));
        
        if (pop_failed) {
            str[i] = '\0';
            return;
        }
    }
    // Make sure that string is terminated.
    str[len - 1] = '\0';
}

uint8_t sercom_uart_has_delim (struct sercom_uart_desc_t *uart, char delim)
{
    return circular_buffer_has_char(&uart->in_buffer, delim);
}

void sercom_uart_get_line_delim (struct sercom_uart_desc_t *uart, char delim,
                                 char *str, uint16_t len)
{
    for (uint16_t i = 0; i < (len - 1); i++) {
        uint8_t pop_failed = circular_buffer_pop(&uart->in_buffer,
                                                 (uint8_t*)(str + i));
        
        if (pop_failed || str[i] == delim) {
            str[i] = '\0';
            return;
        }
    }
    // Make sure that string is terminated.
    str[len - 1] = '\0';
}

uint8_t sercom_uart_has_line (struct sercom_uart_desc_t *uart)
{
    return circular_buffer_has_line(&uart->in_buffer);
}

void sercom_uart_get_line (struct sercom_uart_desc_t *uart, char *str,
                           uint16_t len)
{
    uint8_t last_char_cr = 0;
    for (uint16_t i = 0; i < (len - 1); i++) {
        uint8_t pop_failed = circular_buffer_pop(&uart->in_buffer,
                                                 (uint8_t*)(str + i));
        
        if (pop_failed) {
            str[i] = '\0';
            return;
        } else if (last_char_cr && str[i] == '\n') {
            str[i - 1] = '\0';
            return;
        }
        
        last_char_cr = str[i] == '\r';
    }
    
    // We ran out of space in the buffer to pop the next character, we might
    // have just popped a carriage return, and the next character might be a
    // newline, in which case we can pop the newline even though the buffer is
    // full since we don't need to put it in our buffer
    uint8_t c;
    if (last_char_cr && !circular_buffer_peak(&uart->in_buffer, &c)) {
        if (c == '\n') {
            circular_buffer_pop(&uart->in_buffer, &c);
        }
    }
    
    // Make sure that string is terminated.
    str[len - 1] = '\0';
}

char sercom_uart_get_char (struct sercom_uart_desc_t *uart)
{
    char c = '\0';
    circular_buffer_pop(&uart->in_buffer, (uint8_t*)&c);
    return c;
}

uint8_t sercom_uart_out_buffer_empty (struct sercom_uart_desc_t *uart)
{
    return circular_buffer_is_empty(&uart->out_buffer);
}

void sercom_uart_send_break (struct sercom_uart_desc_t *uart,
                             uint8_t duration)
{
    if (duration == 0) {
        return;
    }

    uart->break_duration = duration;
    uart->break_pending = 1;

    sercom_uart_service(uart);
}

void sercom_uart_service (struct sercom_uart_desc_t *uart)
{
    /* Acquire service function lock */
    if (uart->service_lock) {
        // Could not acquire lock, service is already being run
        return;
    } else {
        uart->service_lock = 1;
    }

    /* Check if currently sending data */
    if ((uart->use_dma && dma_chan_is_active(uart->dma_chan)) ||
        (!uart->use_dma && uart->sercom->USART.INTENSET.bit.DRE)) {
        // Sending data is already in progress
        uart->service_lock = 0;
        return;
    }

    /* Break condition */
    if ((uart->break_duration != 0) && !uart->break_pending) {
        // Currently sending a break condition
        if ((millis - uart->break_start_time) > uart->break_duration) {
            // Break time is complete
            // Switch TX pin back to being controlled by SERCOM
            PORT->Group[uart->tx_pin_group].PINCFG[uart->tx_pin_num].bit.PMUXEN
                                                                            = 1;
            uart->break_duration = 0;
        } else {
            uart->service_lock = 0;
            return;
        }
    }

    if (uart->break_pending && (uart->break_duration != 0)) {
        // Need to send a break condition
        // Disable SERCOM control of TX pin
        PORT->Group[uart->tx_pin_group].PINCFG[uart->tx_pin_num].bit.PMUXEN = 0;
        uart->break_start_time = millis;
        uart->break_pending = 0;
        uart->service_lock = 0;
        return;
    }

    /* Data */
    if (circular_buffer_is_empty(&uart->out_buffer)) {
        // No data to be sent
        uart->service_lock = 0;
        return;
    } else if (uart->use_dma && !dma_chan_is_active(uart->dma_chan)) {
        // A DMA write operation is not in progress
        // Start writing data via DMA
        dma_config_circular_buffer_to_static(
                                &uart->dma_tran, uart->dma_chan,
                                &uart->out_buffer,
                                (volatile uint8_t*)&uart->sercom->USART.DATA,
                                sercom_get_dma_tx_trigger(uart->sercom_instnum),
                                SERCOM_DMA_TX_PRIORITY);
    } else if (!uart->use_dma && !uart->sercom->USART.INTENSET.bit.DRE) {
        // A interrupt driven write operation is not in progress
        // Start data register empty interrupts.
        uart->sercom->USART.INTENSET.bit.DRE = 0b1;
    }
    
    uart->service_lock = 0;
}



static void sercom_uart_isr_dre (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_uart_desc_t *uart = (struct sercom_uart_desc_t*)state;
    
    /* TX */
    uint8_t c = '\0';
    uint8_t empty = circular_buffer_pop(&uart->out_buffer, &c);

    if (!empty) {
        // Send next char
        sercom->USART.DATA.reg = c;
    } else {
        // All chars sent, disable DRE interrupt
        sercom->USART.INTENCLR.bit.DRE = 0b1;
    }

    // For some reason the RXC interrupt seems to get disabled every time the
    // interrupt service routine runs. Not clear why this happens, it is not
    // mentioned in the datasheet.
    sercom->USART.INTENSET.bit.RXC = 0b1;
}

static void sercom_uart_isr_rxc (Sercom *sercom, uint8_t inst_num, void *state)
{
    struct sercom_uart_desc_t *uart = (struct sercom_uart_desc_t*)state;

    /* RX complete */
    uint8_t data = sercom->USART.DATA.reg;

    if (!uart->echo) {
        // Always add bytes to input buffer when echo is off
        circular_buffer_try_push(&uart->in_buffer, data);
    } else if (!iscntrl(data) || (data == '\r')) {
        // Should add byte to input buffer
        uint8_t full = circular_buffer_try_push(&uart->in_buffer, data);

        if (!full && isprint(data)) {
            // Echo
            sercom_uart_put_char(uart, (char)data);
        } else if (!full && (data == '\r')) {
            // Echo newline
            sercom_uart_put_char(uart, '\n');
        }
    } else if (data == 127) {
        // Backspace
        uint8_t empty = circular_buffer_unpush(&uart->in_buffer);

        if (!empty) {
            sercom_uart_put_string(uart, "\x1B[1D\x1B[K");
        }
    }

    // For some reason the RXC interrupt seems to get disabled every time the
    // interrupt service routine runs. Not clear why this happens, it is not
    // mentioned in the datasheet.
    sercom->USART.INTENSET.bit.RXC = 0b1;
}

static void sercom_uart_dma_callback (uint8_t chan, void *state)
{
    sercom_uart_service((struct sercom_uart_desc_t*)state);
}
