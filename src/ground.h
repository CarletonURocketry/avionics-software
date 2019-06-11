/**
 * @file ground.h
 * @desc Service to receive and relay telemetry packets
 * @author Samuel Dewan
 * @date 2019-06-11
 * Last Author:
 * Last Edited On:
 */

#ifndef ground_h
#define ground_h

#include "global.h"

#include "console.h"
#include "rn2483.h"

extern void init_ground_service(struct console_desc_t *out_console,
                                struct rn2483_desc_t *radio);

extern void ground_service (void);

#endif /* ground_h */
