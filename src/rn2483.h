/**
 * @file rn2483.h
 * @desc Driver for RN2483 LoRa radio
 * @author Samuel Dewan
 * @date 2019-06-07
 * Last Author:
 * Last Edited On:
 */

#ifndef rn2483_h
#define rn2483_h

#include "global.h"

#include "sercom-uart.h"

#define RN2483_BUFFER_LEN   256

#define RN2483_FREQ_MIN 433050000UL
#define RN2483_FREQ_MAX 434790000UL

#define RN2483_PWR_MIN  -3
#define RN2483_PWR_MAX  14

struct rn2483_desc_t;

/**
 *  RN2483 Bandwidth
 */
enum rn2483_state {
    RN2483_RESET,
    RN2483_WRITE_WDT,
    RN2483_PAUSE_MAC,
    RN2483_WRITE_MODE,
    RN2483_WRITE_FREQ,
    RN2483_WRITE_PWR,
    RN2483_WRITE_SF,
    RN2483_WRITE_CRC,
    RN2483_WRITE_IQI,
    RN2483_WRITE_CR,
    RN2483_WRITE_SYNC,
    RN2483_WRITE_BW,
    RN2483_IDLE,
    RN2483_SEND,
    RN2483_SEND_WAIT,
    RN2483_RECEIVE,
    RN2483_RECEIVE_WAIT,
    RN2483_GET_SNR,
    RN2483_FAILED
};

/**
 *  Status code returned when starting a radio operation
 */
enum rn2483_operation_result {
    /** Operation was started successfuly */
    RN2483_OP_SUCCESS,
    /** Radio is busy */
    RN2483_OP_BUSY,
    /** Data size is too long */
    RN2483_OP_TOO_LONG
};


/**
 *  RN2483 Spreading Factor
 */
enum rn2483_sf {
    RN2483_SF_SF7,
    RN2483_SF_SF8,
    RN2483_SF_SF9,
    RN2483_SF_SF10,
    RN2483_SF_SF11,
    RN2483_SF_SF12
};

/**
 *  RN2483 Coding Rate
 */
enum rn2483_cr {
    RN2483_CR_4_5,
    RN2483_CR_4_6,
    RN2483_CR_4_7,
    RN2483_CR_4_8
};

/**
 *  RN2483 Bandwidth
 */
enum rn2483_bw {
    RN2483_BW_125,
    RN2483_BW_250,
    RN2483_BW_500
};

/**
 *  Type for callback function used for receiving data.
 */
typedef void (*rn2483_recv_callback)(struct rn2483_desc_t *inst, void *context,
                                     uint8_t *data, uint8_t length, int8_t snr);

/**
 *  Descriptor for RN2483 radio module driver instance.
 */
struct rn2483_desc_t {
    /** UART instance to which the radio is connected */
    struct sercom_uart_desc_t *uart;
    /** Callback function to be called when data is received */
    rn2483_recv_callback receive_callback;
    /** Context variable for callback */
    void *callback_context;
    
    /** Radio configuration information */
    struct {
        /** Centre frequency */
        uint32_t freq;
        /** Sync word */
        uint8_t sync_byte;
        /** Power level  */
        int8_t power;
        /** LoRa spreading factor */
        enum rn2483_sf spreading_factor:3;
        /** LoRa coding rate */
        enum rn2483_cr coding_rate:2;
        /** Bandwidth */
        enum rn2483_bw bandwidth:2;
        /** Whether a CRC should be added to the data */
        uint8_t crc:1;
        /** Whether the I and Q streams should be inverted */
        uint8_t invert_qi:1;
    } settings;
    
    /** Buffer used for marshalling commands and receiving responses */
    char buffer[RN2483_BUFFER_LEN];
    
    /** Pointer for sending commands over multiple calls to service if UART
        buffer becomes full */
    uint8_t out_pos;
    
    /** Current state of driver */
    enum rn2483_state state:6;
    /** Whether a new line needs to be received before the driver can
        continue */
    uint8_t waiting_for_line:1;
    /** Whether the command to be sent next has been marshalled */
    uint8_t cmd_ready:1;
};

/**
 *  Initialize instance of RN2483 radio driver.
 *
 *  @param inst The instance of the driver to be initialized
 *  @param uart The uart instance used to communicate with the radio
 *  @param freq Centre frequency in hertz, from 433050000 to 434790000
 *  @param power Power level in dBm, from -3 to 14
 *  @param spreading_factor LoRa spreading factor
 *  @param coding_rate LoRa coding rate
 *  @param bandwidth Bandwidth
 *  @param send_crc Whether a CRC should be added to the data
 *  @param invert_qi Whether the I and Q streams should be inverted
 *  @param sync_byte Sync word
 */
extern void init_rn2483 (struct rn2483_desc_t *inst,
                         struct sercom_uart_desc_t *uart, uint32_t freq,
                         int8_t power, enum rn2483_sf spreading_factor,
                         enum rn2483_cr coding_rate, enum rn2483_bw bandwidth,
                         uint8_t send_crc, uint8_t invert_qi,
                         uint8_t sync_byte);

/**
 *  Service to be run in each iteration of the main loop.
 *
 *  @param inst The driver instance for which the service should be run
 */
extern void rn2483_service (struct rn2483_desc_t *inst);

/**
 *  Send data via radio.
 *
 *  @param inst Driver instance
 *  @param data The data to be sent
 *  @param length The number of bytes to be sent
 *
 *  @return Operation status
 */
extern enum rn2483_operation_result rn2483_send (struct rn2483_desc_t *inst,
                                                 const uint8_t *data,
                                                 uint8_t length);

/**
 *  Receive data from radio.
 *
 *  @param inst Driver instance
 *  @param window_size Length of receive window in symbols, 0 for indefinite
 *  @param callback Function to be called when data is received
 *  @param context Context variable to be provided to callback
 *
 *  @return Opertaion status
 */
extern enum rn2483_operation_result rn2483_receive (struct rn2483_desc_t *inst,
                                                    uint32_t window_size,
                                                rn2483_recv_callback callback,
                                                    void *context);

#endif /* rn2483_h */

