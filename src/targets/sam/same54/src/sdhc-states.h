/**
 * @file sdhc-states.h
 * @desc Driver state machine for SD host controller
 * @author Samuel Dewan
 * @date 2021-07-01
 * Last Author:
 * Last Edited On:
 */

#ifndef sdhc_states_h
#define sdhc_states_h

#include "sdhc.h"


//  Internal Configuration
#define SDHC_CLK_INIT       400000UL
#define SDHC_CLK_NORMAL     25000000UL
#define SDHC_CLK_HIGH_SPEED 50000000UL

#define SDHC_INTERRUPT_PRIORITY 6

#define SDHC_NUM_INIT_RETRIES       5
#define SDHC_ACMD41_INIT_TIMEOUT    MS_TO_MILLIS(1000)
#define SDHC_NUM_OP_RETRIES         3

#define SDHC_BLOCK_SIZE 512




enum sdhc_substate {
    SDHC_SUBSTATE_START = 0,
    SDHC_SUBSTATE_CMD_WAIT,
    SDHC_SUBSTATE_CMD_DONE,
    SDHC_SUBSTATE_TRAN_WAIT,
    SDHC_SUBSTATE_TRAN_DONE,
    SDHC_SUBSTATE_CMD_ERROR,
    SDHC_SUBSTATE_TRAN_ERROR
};



typedef int (*sdhc_state_handler_t)(struct sdhc_desc_t *inst);

/**
 *  Array of functions for handling FSM states.
 *
 *  Each state handler returns 0 if the service function should return or 1 if
 *  the service function should call the handler for the next state immediately.
 */
extern const sdhc_state_handler_t sdhc_state_handlers[];

#endif /* sdhc_states_h */
