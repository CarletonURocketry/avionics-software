/**
 * @file telemetry.h
 * @desc Service to send telemetry
 * @author Samuel Dewan
 * @date 2019-06-11
 * Last Author:
 * Last Edited On:
 */

#ifndef telemetry_h
#define telemetry_h

#include "global.h"

#include "rn2483.h"

#include "ms5611.h"
#include "gnss-xa1110.h"

extern uint8_t telemetry_paused;

extern void init_telemetry_service (struct rn2483_desc_t *radio,
                                    struct ms5611_desc_t *altimeter,
                                    uint32_t telemetry_rate);

extern void telemetry_service (void);

#endif /* telemetry_h */
