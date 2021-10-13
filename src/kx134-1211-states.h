/**
 * @file kx134-1211-states.h
 * @desc Driver state machine for KX134-1211 accelerometer
 * @author Samuel Dewan
 * @date 2021-07-10
 * Last Author:
 * Last Edited On:
 */

#ifndef kx134_1211_states_h
#define kx134_1211_states_h

#include "kx134-1211.h"
#include "kx134-1211-registers.h"

typedef int (*kx134_1211_state_handler_t)(struct kx134_1211_desc_t *inst);


extern void kx134_1211_spi_callback(void *context);
extern int kx134_1211_handle_read_buffer(struct kx134_1211_desc_t *inst);


/**
 *  Array of functions for handling FSM states.
 *
 *  Each state handler returns 0 if the service function should return or 1 if
 *  the service function should call the handler for the next state immediately.
 */
extern const kx134_1211_state_handler_t kx134_1211_state_handlers[];

#endif /* kx134_1211_states_h */
