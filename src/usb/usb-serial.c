/**
 * @file usb-serial.c
 * @desc USB CDC ACM serial port emulation.
 * @author Samuel Dewan
 * @date 2019-01-19
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-01-20
 */

#include "usb-serial.h"

#include "../global.h"

#include "usb.h"
#include "samd/usb_samd.h"
#include "../circular-buffer.h"

//#include <string.h>
#include <ctype.h>

#define USB_BUF_SIZE 64

static uint8_t usbserial_sending_in = 0;
static uint8_t usbserial_initilized = 0;
static USB_ALIGN uint8_t usbserial_buf_in[USB_BUF_SIZE];
static USB_ALIGN uint8_t usbserial_buf_out[USB_BUF_SIZE];

#define USB_CIRC_BUFF_SIZE  128
static struct circular_buffer_t rx_circ_buff_g;
static struct circular_buffer_t tx_circ_buff_g;

static uint8_t rx_buff_g[USB_CIRC_BUFF_SIZE];
static uint8_t tx_buff_g[USB_CIRC_BUFF_SIZE];

void (*usb_serial_ready_callback) (void *);
void *usb_serial_callback_context;


/**
 *  Start sending any pending data.
 */
static void usb_serial_service(void);


void usb_serial_init(void)
{
    init_circular_buffer(&rx_circ_buff_g, rx_buff_g, USB_CIRC_BUFF_SIZE);
    init_circular_buffer(&tx_circ_buff_g, tx_buff_g, USB_CIRC_BUFF_SIZE);
    
    usbserial_sending_in = 0;
    
    usb_enable_ep(USB_EP_CDC_NOTIFICATION, USB_EP_TYPE_INTERRUPT, 8);
    usb_enable_ep(USB_EP_CDC_OUT, USB_EP_TYPE_BULK, USB_BUF_SIZE);
    usb_enable_ep(USB_EP_CDC_IN, USB_EP_TYPE_BULK, USB_BUF_SIZE);
    
    usbserial_initilized = 1;
    
    usb_ep_start_out(USB_EP_CDC_OUT, usbserial_buf_out, USB_BUF_SIZE);
    
    if (usb_serial_ready_callback) {
        usb_serial_ready_callback(usb_serial_callback_context);
    }
}

void usb_serial_out_completion(void)
{
    uint32_t len = usb_ep_out_length(USB_EP_CDC_OUT);
    
    // Copy data from usbserial_buf_out to rx_circ_buff_g and echo as required
    for (uint32_t i = 0; i < len; i++) {
        uint8_t data = usbserial_buf_out[i];
        uint8_t full = 0;
        
        if (!iscntrl(data) || (data == '\r')) {
            // Should add byte to input buffer
            full = circular_buffer_try_push(&rx_circ_buff_g, data);
            
            if (!full && isprint(data)) {
                // Echo
                usb_serial_put_char((char)data);
            } else if (!full && (data == '\r')) {
                // Echo newline
                usb_serial_put_char('\n');
            }
        } else if (data == 127) {
            // Backspace
            uint8_t empty = circular_buffer_unpush(&rx_circ_buff_g);
            
            if (!empty) {
                usb_serial_put_string("\x1B[1D\x1B[K");
            }
        }
    }
    
    usb_ep_start_out(USB_EP_CDC_OUT, usbserial_buf_out, USB_BUF_SIZE);
}

void usb_serial_in_completion(void)
{
    usbserial_sending_in = 0;
    usb_serial_service();
}

void usb_serial_disable(void)
{
    usb_disable_ep(USB_EP_CDC_NOTIFICATION);
    usb_disable_ep(USB_EP_CDC_OUT);
    usb_disable_ep(USB_EP_CDC_IN);
}

void usb_serial_service(void)
{
    if (!usbserial_sending_in) {
        // We are not currently sending data
        uint16_t i = 0;
        uint8_t result = 1;
        
        // Copy data to the USB in buffer
        do {
            result = circular_buffer_pop(&tx_circ_buff_g, usbserial_buf_in + i);
            i += !result;
        } while (!result && (i < USB_BUF_SIZE));
        
        if (i) {
            // If there was any data to copy to the in buffer, start
            // transmitting it
            usb_ep_start_in(USB_EP_CDC_IN, usbserial_buf_in, i, false);
            usbserial_sending_in = 1;
        }
    }
}



uint16_t usb_serial_put_string(const char *str)
{
    uint16_t i = 0;
    for (; str[i] != '\0'; i++) {
        if (circular_buffer_is_full(&tx_circ_buff_g)) {
            break;
        }
        
        circular_buffer_push(&tx_circ_buff_g, (uint8_t)str[i]);
        
        if (str[i] == '\n') {
            // Add carriage return as some terminal emulators seem to think that
            // they are typewriters.
            circular_buffer_push(&tx_circ_buff_g, (uint8_t)'\r');
        }
    }
    
    // Make sure that we start transmition right away if there is no transmition
    // already in progress.
    usb_serial_service();
    
    return i;
}

void usb_serial_put_string_blocking(const char *str)
{
    uint8_t carriage_return = 0;
    
    for (const char *i = str; *i != '\0';) {
        // Wait for a character worth of space to become avaliable in the buffer
        while (circular_buffer_is_full(&tx_circ_buff_g)) {
            // Make sure that we aren't waiting for a transaction which is not
            // in progress.
            usb_serial_service();
        }
        
        if (carriage_return) {
            // Push a carriage return
            circular_buffer_push(&tx_circ_buff_g, (uint8_t)'\r');
        } else {
            // Push the next character
            circular_buffer_push(&tx_circ_buff_g, (uint8_t)*i);
        }
        
        if (*i == '\n' && !carriage_return) {
            // Add carriage return after newlines
            carriage_return = 1;
        } else {
            i++;
            carriage_return = 0;
        }
    }
    
    // Make sure that we start transmition right away if there is no transmition
    // already in progress.
    usb_serial_service();
}

uint16_t usb_serial_put_bytes(const uint8_t *bytes, uint16_t length)
{
    uint16_t i = 0;
    for (; i < length; i++) {
        if (circular_buffer_is_full(&tx_circ_buff_g)) {
            break;
        }
        
        circular_buffer_push(&tx_circ_buff_g, (uint8_t)bytes[i]);
    }
    
    // Make sure that we start transmition right away if there is no transmition
    // already in progress.
    usb_serial_service();
    
    return i;
}

void usb_serial_put_bytes_blocking(const uint8_t *bytes, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++) {
        // Wait for a character worth of space to become avaliable in the buffer
        while (circular_buffer_is_full(&tx_circ_buff_g)) {
            // Make sure that we aren't waiting for a transaction which is not
            // in progress.
            usb_serial_service();
        }
        
        circular_buffer_push(&tx_circ_buff_g, bytes[i]);
    }
    
    // Make sure that we start transmition right away if there is no transmition
    // already in progress.
    usb_serial_service();
}

void usb_serial_put_char (const char c)
{
    circular_buffer_push(&tx_circ_buff_g, (uint8_t)c);
    
    if (c == '\n') {
        // Add carriage return as some terminal emulators seem to think that
        // they are typewriters (backwards compatability has gone too far).
        circular_buffer_push(&tx_circ_buff_g, (uint8_t)'\r');
    }
    
    // Make sure that we start transmition right away if there is no transmition
    // already in progress.
    usb_serial_service();
}

void usb_serial_get_string (char *str, uint16_t len)
{
    for (uint16_t i = 0; i < (len - 1); i++) {
        uint8_t pop_failed = circular_buffer_pop(&rx_circ_buff_g,
                                                 (uint8_t*)(str + i));
        
        if (pop_failed) {
            str[i] = '\0';
            return;
        }
    }
    // Make sure that string is terminated.
    str[len - 1] = '\0';
}

uint8_t usb_serial_has_line (char delim)
{
    return (usbserial_initilized &&
            circular_buffer_has_char(&rx_circ_buff_g, delim));
}

void usb_serial_get_line (char delim, char *str, uint16_t len)
{
    for (uint16_t i = 0; i < (len - 1); i++) {
        uint8_t pop_failed = circular_buffer_pop(&rx_circ_buff_g,
                                                 (uint8_t*)(str + i));
        
        if (pop_failed || str[i] == delim) {
            str[i] = '\0';
            return;
        }
    }
    // Make sure that string is terminated.
    str[len - 1] = '\0';
}

char usb_serial_get_char (void)
{
    char c = '\0';
    circular_buffer_pop(&rx_circ_buff_g, (uint8_t*)&c);
    return c;
}

uint8_t usb_serial_out_buffer_empty (void)
{
    return circular_buffer_is_empty(&tx_circ_buff_g);
}
