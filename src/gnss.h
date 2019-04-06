/**
 * @file gnss.h
 * @desc XA1110 GNSS Receiver Driver
 * @author Tauheed ELahee
 * @date 2019-04-05
 * Last Author:
 * Last Edited On:
 */

#ifndef gnss_h
#define gnss_h

#include "global_h"

struct descriptor {
  uint32_t utc_time;
  int32_t latitude;
  int32_t longitude;
  int16_t speed_over_ground;
  int16_t course_over_ground;
  uint8_t status;
}


/**
   Configures the desctiptor structure with all the necessary data for the GNSS reciever to work.
   Begin the process of sending any commands to the module that are nessary to initilize it.
*/
extern void init_gnss(void);



#endif /* gnss_h */
