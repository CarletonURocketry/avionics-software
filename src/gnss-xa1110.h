/**
 * @file gnss.h
 * @desc XA1110 GNSS Receiver Driver
 * @author Tauheed ELahee
 * @date 2019-04-05
 * @last-edited: 2019-06-09
 */

#ifndef gnss_h
#define gnss_h

#include "global.h"
#include "console.h"

/**
   Configures the desctiptor structure with all the necessary data for the GNSS reciever to work.
   Begin the process of sending any commands to the module that are nessary to initilize it.
   @param console
*/
extern uint8_t init_gnss_xa1110(struct console_desc_t* console);

extern struct gnss {
  uint32_t gnss_xa1110_system_time;
  uint32_t gnss_xa1110_utc_time;
  int32_t gnss_xa1110_latitude;
  int32_t gnss_xa1110_longitude;
  int16_t gnss_xa1110_speed;
  int16_t gnss_xa1110_course;
  uint8_t gnss_xa1110_status;
} gnss_xa1110_descriptor;

/**
 * A set of functions that return the particular part of the struct that conatians the data provided by the GNSS module
 */

static inline uint32_t gnss_xa1110_retrieve_sytem_time(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_system_time;
}

static inline uint32_t gnss_xa1110_retrieve_utc_time(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_utc_time;
}

static inline uint32_t gnss_xa1110_retrieve_latitude(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_latitude;
}

static inline uint32_t gnss_xa1110_retrieve_longitude(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_longitude;
}

static inline uint16_t gnss_xa1110_retrieve_speed(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_speed;
}

static inline uint16_t gnss_xa1110_retrieve_course(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_course;
}

static inline uint8_t gnss_xa1110_retrieve_status(void)
{
  return gnss_xa1110_descriptor.gnss_xa1110_status;
}

#endif /* gnss_h */
