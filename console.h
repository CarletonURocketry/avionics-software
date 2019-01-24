/**
 * @file console.h
 * @desc Abstracts over sercom-uart or usb-serial console.
 * @author Samuel Dewan
 * @date 2019-09-20
 * Last Author:
 * Last Edited On:
 */

#ifndef console_h
#define console_h

#include "sercom-uart.h"


#define CONSOLE_IN_BUFFER_LEN 256


/**
 *  Descriptor for a console instance.
 */
struct console_desc_t {
    struct sercom_uart_desc_t *uart;
    
    void (*line_callback)(char*, struct console_desc_t*, void*);
    void (*init_callback)(struct console_desc_t*, void*);
    void *callback_context;
    
    char line_delimiter;
};


/**
 *  Initilizes a console instance.
 *
 *  @param console The conosle descriptor to be initilized.
 *  @param uart Pointer to a sercom uart descriptor or NULL if a usb serial
 *  interface should be used.
 *  @param line_delim The delimter for new lines received
 */
extern void init_console (struct console_desc_t *console,
                          struct sercom_uart_desc_t *uart,
                          const char line_delim);

/**
 *  Send a string on a console. This function will block until the whole string
 *  is copied in to the buffer even if the whole string does not initialy fit in
 *  the avaliable free space.
 *
 *  @param console The console on which the string should be sent
 *  @param str The string to be sent
 */
extern void console_send_str (struct console_desc_t *console, const char *str);

/**
 *  Send a string on a console. This function copies the string into the output
 *  buffer and then returns immediately. It does not check that there is enough
 *  free space in the buffer for the string. If there is not enough space, the
 *  output may be corupted.
 *
 *  @param console The console on which the string should be sent
 *  @param str The string to be sent
 */
extern void console_send_str_async (struct console_desc_t *console,
                                    const char *str);

/**
 *  Set the function which should be called when a full line has been recieved.
 *  The function will be passed the line as a string, a pointer to the console
 *  on which to the line was recieved and a pointer to an object which contains
 *  context infomation (if provided).
 *  If the the callback is not set or set to NULL then lines will be discarded
 *  when they as they are recieved.
 *
 *  @param console The console for which the callback should be set
 *  @param line_callback The function to be called when a complete line has been
 *                       recieved
 *  @param context A point which will be passed to the callback
 */
extern void console_set_line_callback (struct console_desc_t *console,
                                       void (*line_callback)(char*,
                                                        struct console_desc_t*,
                                                        void*), void *context);

/**
 *  Set the function which should be called when the console is ready to be
 *  initialized.
 *
 *  @param console The console for which the callback should be set
 *  @param init_callback The function to be called when the console is ready.
 *  @param context A point which will be passed to the callback
 */
extern void console_set_init_callback (struct console_desc_t *console,
                                       void (*init_callback)(
                                                        struct console_desc_t*,
                                                        void*), void *context);

/**
 *  Service to be run in each iteration of the main loop.
 *
 *  @param console The console for which the service should be run
 */
extern void console_service (struct console_desc_t *console);

#endif /* console_h */
