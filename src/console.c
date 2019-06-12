/**
 * @file console.c
 * @desc Abstracts over sercom-uart or usb-serial console.
 * @author Samuel Dewan
 * @date 2019-09-20
 * Last Author:
 * Last Edited On:
 */

#include "console.h"

#ifdef ID_USB
#include "usb/usb-serial.h"
#endif

#ifdef ID_USB
static void console_usb_serial_ready_cb (void *context)
{
    struct console_desc_t *console = (struct console_desc_t *)context;
    
    if (console->init_callback) {
        console->init_callback(console, console->callback_context);
    }
}
#endif

void init_console (struct console_desc_t *console,
                   struct sercom_uart_desc_t *uart, char line_delim)
{
    console->uart = uart;
    console->line_callback = NULL;
    console->callback_context = NULL;
    console->line_delimiter = line_delim;
}

void console_send_str (struct console_desc_t *console, const char *str)
{
    if (console->uart) {
        // SERCOM UART
        sercom_uart_put_string_blocking(console->uart, str);
    } else {
#ifdef ID_USB
        // USB
        usb_serial_put_string_blocking(str);
#endif
    }
}

uint16_t console_send_str_async (struct console_desc_t *console,
                                 const char *str)
{
    if (console->uart) {
        // SERCOM UART
        return sercom_uart_put_string(console->uart, str);
    } else {
#ifdef ID_USB
        // USB
        return usb_serial_put_string(str);
#endif
    }
}

void console_send_bytes (struct console_desc_t *console,
                         const uint8_t *data, uint16_t length)
{
    if (console->uart) {
        // SERCOM UART
        sercom_uart_put_bytes_blocking(console->uart, data, length);
    } else {
#ifdef ID_USB
        // USB
        usb_serial_put_bytes_blocking(data, length);
#endif
    }
}

uint16_t console_send_bytes_async (struct console_desc_t *console,
                                          const uint8_t *data, uint16_t length)
{
    if (console->uart) {
        // SERCOM UART
        return sercom_uart_put_bytes(console->uart, data, length);
    } else {
#ifdef ID_USB
        // USB
        return usb_serial_put_bytes(data, length);
#endif
    }
}

void console_set_line_callback (struct console_desc_t *console,
                                void (*line_callback)(char*,
                                                      struct console_desc_t*,
                                                      void*), void *context)
{
    console->line_callback = line_callback;
    console->callback_context = context;
}

void console_set_init_callback (struct console_desc_t *console,
                                void (*init_callback)(
                                                      struct console_desc_t*,
                                                      void*), void *context)
{
    console->init_callback = init_callback;
    console->callback_context = context;
    
    if (console->uart) {
        // If this is a SERCOM UART based console it is always ready
        console->init_callback(console, console->callback_context);
    } else {
#ifdef ID_USB
        // If this is a USB serial based console then we need to use the USB
        // serial ready callback.
        usb_serial_ready_callback = console_usb_serial_ready_cb;
        usb_serial_callback_context = (void*)console;
#endif
    }
}

void console_service (struct console_desc_t *console)
{
    static char console_in_buffer[CONSOLE_IN_BUFFER_LEN];
    
    uint8_t got_line = 0;
    
    if (console->uart) {
        // SERCOM UART
        if (sercom_uart_has_line(console->uart, console->line_delimiter)) {
            sercom_uart_get_line(console->uart, console->line_delimiter,
                                 console_in_buffer, CONSOLE_IN_BUFFER_LEN);
            got_line = 1;
        }
    } else {
#ifdef ID_USB
        // USB
        if (usb_serial_has_line(console->line_delimiter)) {
            usb_serial_get_line(console->line_delimiter, console_in_buffer,
                                CONSOLE_IN_BUFFER_LEN);
            got_line = 1;
        }
#endif
    }
    
    if (got_line && console->line_callback) {
        console->line_callback(console_in_buffer, console, console->callback_context);
    }
}
