/**
 * @file gnss.h
 * @desc XA1110 GNSS Receiver Driver
 * @author Tauheed ELahee
 * @date 2019-04-05
 * @last-edited: 2019-05-21
 */

#ifndef gnss_h
#define gnss_h

#include "global.h"
#include "console.h"

/**
 * gnss_line_callback function
 * @param line
 * @param console
 * @param context
 */
extern void gnss_line_callback(char* line, struct console_desc_t* console, void* context);

/**
 * gnss_init_callback function
 * @param console
 * @param context
 */
extern void gnss_init_callback(struct console_desc_t* console, void* context);

/**
   Configures the desctiptor structure with all the necessary data for the GNSS reciever to work.
   Begin the process of sending any commands to the module that are nessary to initilize it.
   @param console
*/
extern void init_gnss_xa1110(struct console_desc_t* console);

/**
 * A set of functions that return the particular part of the struct that conatians the data provided by the GNSS module
 */

extern uint32_t gnss_xa1110_retrieve_sytem_time(void);

extern uint32_t gnss_xa1110_retrieve_utc_time(void);

extern uint32_t gnss_xa1110_retrieve_latitude(void);

extern uint32_t gnss_xa1110_retrieve_longitude(void);

extern uint16_t gnss_xa1110_retrieve_speed(void);

extern uint16_t gnss_xa1110_retrieve_course(void);

extern uint8_t gnss_xa1110_retrieve_status(void);

#endif /* gnss_h */

/*
 * init
 * service
 * retrive for each time, position, velocity and status
 * already 
*/
