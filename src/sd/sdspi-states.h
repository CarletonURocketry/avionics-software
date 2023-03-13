/**
 * @file sdspi-states.h
 * @desc SD Card via SPI state machine handlers.
 * @author Samuel Dewan
 * @date 2021-05-28
 * Last Author:
 * Last Edited On:
 */

#ifndef sdspi_states_h
#define sdspi_states_h

#include "sdspi.h"
#include "sdspi-commands.h"

typedef int (*sdspi_state_handler_t)(struct sdspi_desc_t *inst);

/**
 *  Array of functions for handling FSM states.
 *
 *  Each state handler returns 0 if the service function should return or 1 if
 *  the service function should call the handler for the next state immediately.
 */
extern const sdspi_state_handler_t sdspi_state_handlers[];

#endif /* sdspi_states_h */
