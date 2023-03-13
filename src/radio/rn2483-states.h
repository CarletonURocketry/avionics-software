/**
 * @file rn2483-states.h
 * @desc State handler functions for RN2483 driver
 * @author Samuel Dewan
 * @date 2020-02-08
 * Last Author:
 * Last Edited On:
 */

#ifndef rn2483_states_h
#define rn2483_states_h

#include "rn2483.h"


/** Minimum firmware version supported by driver */
#define RN2483_MINIMUM_FIRMWARE RN2483_VERSION(1, 0, 4)
/** Minimum firmware version which supports radio rxstop command */
#define RN2483_MIN_FW_RXSTOP RN2483_VERSION(1, 0, 5)
/** Minimum firmware version which supports radio get rssi command */
#define RN2483_MIN_FW_RSSI RN2483_VERSION(1, 0, 5)


#define RN2483_CMD_TX_LEN 9


typedef int (*rn2483_state_handler_t)(struct rn2483_desc_t *inst);

/**
 *  Array of functions for handling FSM states.
 *
 *  Each state handler returns 0 if the service function should return or 1 if
 *  the service function should call the handler for the next state immediately.
 */
extern const rn2483_state_handler_t rn2483_state_handlers[];

/** Set the current state of a transaction entry */
extern void set_send_trans_state(struct rn2483_desc_t *inst, int n,
                                 enum rn2483_send_trans_state state);

/** Find the first send transaction with a given state */
extern uint8_t find_send_trans(struct rn2483_desc_t *inst,
                               enum rn2483_send_trans_state state);

#endif /* rn2483_states_h */
