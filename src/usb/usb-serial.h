/**
 * @file usb-serial.h
 * @desc USB CDC ACM serial port emulation.
 * @author Samuel Dewan
 * @date 2019-01-19
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-01-20
 */

#ifndef usb_serial_h
#define usb_serial_h

#include "../global.h"

/** Callback function to be called when USB serial is initialized. */
extern void (*usb_serial_ready_callback) (void *);
/** Context to be pased to the serial ready callback function */
extern void *usb_serial_callback_context;


/**
 *  Initilize USB CDC ACM serial port emulation.
 */
void usb_serial_init(void);

/**
 *  Callback for completion of reception from USB interface
 */
void usb_serial_out_completion(void);

/**
 *  Callback for completion of transmition from USB interface
 */
void usb_serial_in_completion(void);

/**
 *  Disable the USB serial endpoints.
 */
void usb_serial_disable(void);


/**
 *  Queue a string to be written to the serial interface.
 *
 *  @param str The string to be written.
 *
 *  @return The number of characters which could be queued for
 *          transmition.
 */
extern uint16_t usb_serial_put_string(const char *str);

/**
 *  Queue a string to be written to the serial interface. If there is not enough
 *  space for the string in the buffer, block until there is.
 *
 *  @param str The string to be written.
 */
extern void usb_serial_put_string_blocking(const char *str);

/**
 *  Queue a byte array to be written to the serial interface.
 *
 *  @param bytes The array to be written
 *  @param length The number of bytes to be writen
 *
 *  @return The number of bytes which could be added to the queue.
 */
extern uint16_t usb_serial_put_bytes(const uint8_t *bytes, uint16_t length);

/**
 *  Queue a string to be written to the serial interface. If there is not enough
 *  space for the string in the buffer, block until there is.
 *
 *  @param bytes The array to be written
 *  @param length The number of bytes to be writen
 */
extern void usb_serial_put_bytes_blocking(const uint8_t *bytes,
                                          uint16_t length);

/**
 *  Write a character to a serial interface.
 *
 *  @param c The character to be written
 */
extern void usb_serial_put_char (const char c);

/**
 *  Get string from serial interface input buffer.
 *
 *  @param str The string in which the data should be stored.
 *  @param len The maximum number of chars to be read from the buffer.
 */
extern void usb_serial_get_string (char *str, uint16_t len);

/**
 *  Determine if there is a full line avaliable to be read from the serial
 *  interface buffer.
 *
 *  @param delim The delemiter for new lines (ie. '\n').
 *
 *  @return 0 if there is no line avaliable, 1 if a line is avaliable.
 */
extern uint8_t usb_serial_has_line (char delim);

/**
 *  Read a string from the input buffer up to the next occurance of a delimiter.
 *
 *  @param delim The delemiter for new lines (ie. '\n').
 *  @param str The string in which the data should be stored.
 *  @param len The maximum number of chars to be read from the input buffer.
 */
extern void usb_serial_get_line (char delim, char *str, uint16_t len);

/**
 *  Get a character from the serial interface input buffer.
 *
 *  @return The least recently recieved character in the input buffer
 */
extern char usb_serial_get_char (void);

/**
 *  Determine if the out buffer of a serial interface is empty.
 *
 */
extern uint8_t usb_serial_out_buffer_empty (void);

#endif /* usb_serial_h */
