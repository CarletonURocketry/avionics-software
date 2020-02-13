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


/** Minimum firmware version supported by driver */
#define RN2483_MINIMUM_FIRMWARE RN2483_VERSION(1, 0, 4)
/** Minimum firmware version which supports radio rxstop command */
#define RN2483_MIN_FW_RXSTOP RN2483_VERSION(1, 0, 5)
/** Minimum firmware version which supports radio get rssi command */
#define RN2483_MIN_FW_RSSI RN2483_VERSION(1, 0, 5)



typedef int (*rn2483_stat_handler_t)(struct rn2483_desc_t *inst);

/**
 *  Array of functions for handling FSM states.
 *
 *  Each state handler returns 0 if the service function should return or 1 if
 *  the service function should call the handler for the next state immediately.
 */
extern const rn2483_stat_handler_t rn2483_state_handlers[];



extern const char* const RN2483_CMD_RESET;
extern const char* const RN2483_CMD_WDT;
extern const char* const RN2483_CMD_PAUSE_MAC;

extern const char* const RN2483_CMD_MODE;
extern const char* const RN2483_CMD_FREQ;
extern const char* const RN2483_CMD_PWR;
extern const char* const RN2483_CMD_SF;
extern const char* const RN2483_CMD_CRC;
extern const char* const RN2483_CMD_IQI;
extern const char* const RN2483_CMD_CR;
extern const char* const RN2483_CMD_SYNC;
extern const char* const RN2483_CMD_BW;

extern const char* const RN2483_CMD_TX;
extern const char* const RN2483_CMD_RX;
extern const char* const RN2483_CMD_SNR;
extern const char* const RN2483_CMD_RSSI;
extern const char* const RN2483_CMD_RXSTOP;

extern const char* const RN2483_CMD_SET_PINMODE;
extern const char* const RN2483_CMD_SET_PINDIG;
extern const char* const RN2483_CMD_GET_PINDIG;
extern const char* const RN2483_CMD_GET_PINANA;


#endif /* rn2483_states_h */
