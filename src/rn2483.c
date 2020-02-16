/**
 * @file rn2483.c
 * @desc Driver for RN2483 LoRa radio
 * @author Samuel Dewan
 * @date 2019-06-07
 * Last Author:
 * Last Edited On:
 */

#include "rn2483.h"
#include "rn2483-states.h"

#include <string.h>


#define RN2483_ANALOG_PINS_MASK ((1<<RN2483_GPIO0)  | (1<<RN2483_GPIO1)  | \
                                 (1<<RN2483_GPIO2)  | (1<<RN2483_GPIO3)  | \
                                 (1<<RN2483_GPIO5)  | (1<<RN2483_GPIO6)  | \
                                 (1<<RN2483_GPIO7)  | (1<<RN2483_GPIO8)  | \
                                 (1<<RN2483_GPIO9)  | (1<<RN2483_GPIO10) | \
                                 (1<<RN2483_GPIO11) | (1<<RN2483_GPIO12) | \
                                 (1<<RN2483_GPIO13))
#define RN2483_PIN_SUPPORTS_ANA(x) (RN2483_ANALOG_PINS_MASK & (1<<x))



void init_rn2483 (struct rn2483_desc_t *inst, struct sercom_uart_desc_t *uart,
                  uint32_t freq, int8_t power, enum rn2483_sf spreading_factor,
                  enum rn2483_cr coding_rate, enum rn2483_bw bandwidth,
                  uint8_t send_crc, uint8_t invert_qi, uint8_t sync_byte)
{
    inst->uart = uart;
    
    
    if (freq > RN2483_FREQ_MAX) {
        inst->settings.freq = RN2483_FREQ_MAX;
    } else if (freq < RN2483_FREQ_MIN) {
        inst->settings.freq = RN2483_FREQ_MIN;
    } else {
        inst->settings.freq = freq;
    }
    
    if (power > RN2483_PWR_MAX) {
        inst->settings.power = RN2483_PWR_MAX;
    } else if (power < RN2483_PWR_MIN) {
        inst->settings.power = RN2483_PWR_MIN;
    } else {
        inst->settings.power = power;
    }
    
    inst->settings.spreading_factor = spreading_factor;
    inst->settings.coding_rate = coding_rate;
    inst->settings.bandwidth = bandwidth;
    inst->settings.crc = !!send_crc;
    inst->settings.invert_qi = !!invert_qi;
    inst->settings.sync_byte = sync_byte;
    
    // Initialize GPIO pins to inputs
    for (enum rn2483_pin pin = 0; pin < RN2483_NUM_PINS; pin++) {
        inst->pins[pin].raw = (RN2483_PIN_DESC_MODE(RN2483_PIN_MODE_INPUT) |
                               RN2483_PIN_DESC_MODE_DIRTY);
    }
    
    // Start by reseting module
    inst->state = RN2483_RESET;
    
    inst->waiting_for_line = 0;
    inst->cmd_ready = 0;
    inst->out_pos = 0;
}

void rn2483_service (struct rn2483_desc_t *inst)
{
    int do_next_state = 1;
    while (do_next_state) {
        if (inst->waiting_for_line && !sercom_uart_has_line(inst->uart)) {
            // Waiting for a line and a new line has not yet been received
            return;
        }
        
        do_next_state = rn2483_state_handlers[inst->state](inst);
    }
}

enum rn2483_operation_result rn2483_send (struct rn2483_desc_t *inst,
                                          uint8_t const *data, uint8_t length,
                                          uint8_t *transaction_id)
{
    // Check that we are not already sending something and for message length
    if (inst->send_buffer != NULL) {
        return RN2483_OP_BUSY;
    } else if (length > ((RN2483_BUFFER_LEN - (RN2483_CMD_TX_LEN + 2)) / 2)) {
        // Message is too large to be sent
        return RN2483_OP_TOO_LONG;
    }
    
    // Check for an open transaction slot
    uint8_t id = find_send_trans(inst, RN2483_SEND_TRANS_INVALID);\
    if (id == RN2483_NUM_SEND_TRANSACTIONS) {
        return RN2483_OP_BUSY;
    }
    
    // Get transaction ready
    *transaction_id = id;
    
    inst->send_buffer = data;
    inst->send_length = length;
    
    set_send_trans_state(inst, id, RN2483_SEND_TRANS_PENDING);
    
    if (inst->state == RN2483_IDLE) {
        // If we are idle, jump right to send state
        inst->state = RN2483_SEND;
    } else if (inst->state == RN2483_RECEIVE) {
        // We are in the process of sending the receive command or are waiting
        // for the first response to the receive command, we need to indicate
        // that we should abort the receive (if possible) as soon as we are done
        // starting it
        inst->state = RN2483_RECEIVE_ABORT;
    } else if ((inst->version >= RN2483_MIN_FW_RXSTOP) &&
               (inst->state == RN2483_RECEIVE_WAIT)) {
        // rxstop command is supported and we are in receive wait, we should
        // cancel the ongoing reception
        inst->state = RN2483_RXSTOP;
        inst->waiting_for_line = 0;
    }
    
    // Start sending right away if possible
    rn2483_service(inst);
    
    return RN2483_OP_SUCCESS;
}

enum rn2483_send_trans_state rn2483_get_send_state (struct rn2483_desc_t *inst,
                                                    uint8_t transaction_id)
{
    unsigned int offset = (RN2483_SEND_TRANSACTION_SIZE * transaction_id);
    uint16_t state = ((inst->send_transactions >> offset) &
                      RN2483_SEND_TRANSACTION_MASK);
    
    return (enum rn2483_send_trans_state)state;
}

void rn2483_clear_send_transaction (struct rn2483_desc_t *inst,
                                    uint8_t transaction_id)
{
    set_send_trans_state(inst, transaction_id, RN2483_SEND_TRANS_INVALID);
}

enum rn2483_operation_result rn2483_receive (struct rn2483_desc_t *inst,
                                             rn2483_recv_callback callback,
                                             void *context)
{
    if (inst->receive) {
        // If we are already receiving return busy
        return RN2483_OP_BUSY;
    } else if (inst->state == RN2483_FAILED) {
        return RN2483_OP_BAD_STATE;
    }
    
    // Store receive callback
    inst->receive_callback = callback;
    inst->callback_context = context;
    
    // Enable continuous receive and run service to start receiving if possible
    inst->receive = 1;
    rn2483_service(inst);
    
    return RN2483_OP_SUCCESS;
}

enum rn2483_operation_result rn2483_receive_stop (struct rn2483_desc_t *inst)
{
    if (!inst->receive && (inst->state != RN2483_RECEIVE) &&
        (inst->state != RN2483_RECEIVE_WAIT)) {
        // No receive to cancel
        return RN2483_OP_BAD_STATE;
    } else if (inst->state == RN2483_FAILED) {
        return RN2483_OP_BAD_STATE;
    }
    
    // Disable continuous receive
    inst->receive = 0;
    
    if (inst->state == RN2483_RECEIVE) {
        // We are in the process of sending the receive command or are waiting
        // for the first response to the receive command, we need to indicate
        // that we should abort the receive (if possible) as soon as we are done
        // starting it
        inst->state = RN2483_RECEIVE_ABORT;
    } else if ((inst->version >= RN2483_MIN_FW_RXSTOP) &&
               (inst->state == RN2483_RECEIVE_WAIT)) {
        // rxstop command is supported and we are in receive wait, we should
        // cancel the ongoing reception
        inst->state = RN2483_RXSTOP;
        inst->waiting_for_line = 0;
        
        rn2483_service(inst);
    }
    
    return RN2483_OP_SUCCESS;
}



void rn2483_poll_gpio(struct rn2483_desc_t *inst)
{
    for (enum rn2483_pin pin = 0; pin < RN2483_NUM_PINS; pin++) {
        // Mark value dirty if pin has been explicitly set as an
        // input
        if (((inst->pins[pin].mode == RN2483_PIN_MODE_INPUT) ||
             (inst->pins[pin].mode == RN2483_PIN_MODE_ANALOG)) &&
            (inst->pins[pin].mode_explicit)) {
            inst->pins[pin].value_dirty = 1;
        }
    }
}

uint8_t rn2483_poll_gpio_in_progress(struct rn2483_desc_t *inst)
{
    for (enum rn2483_pin pin = 0; pin < RN2483_NUM_PINS; pin++) {
        // If pin is an input and is being polled return 1
        if (((inst->pins[pin].mode == RN2483_PIN_MODE_INPUT) ||
             (inst->pins[pin].mode == RN2483_PIN_MODE_ANALOG)) &&
            (inst->pins[pin].value_dirty)) {
            return 1;
        }
    }
    return 0;
}

uint8_t rn2483_set_pin_mode(struct rn2483_desc_t *inst, enum rn2483_pin pin,
                            enum rn2483_pin_mode mode)
{
    // If the mode has not changed, don't bother sending command to radio
    if (inst->pins[pin].mode == mode) {
        return 0;
    }
    
    // Check that mode is valid for pin
    if ((mode == RN2483_PIN_MODE_ANALOG) && !RN2483_PIN_SUPPORTS_ANA(pin)) {
        return 1;
    }
    
    // Update pin mode in cache, reset the pin's value to 0, and mark the pin's
    // mode has having been explicitly set.
    inst->pins[pin].raw = (RN2483_PIN_DESC_VALUE(0) |
                           RN2483_PIN_DESC_MODE(mode) |
                           RN2483_PIN_DESC_MODE_DIRTY |
                           RN2483_PIN_DESC_VALUE_DIRTY |
                           RN2483_PIN_DESC_MODE_EXP);
    
    // Run service to start sending command to radio if possible
    rn2483_service(inst);
    
    return 0;
}

enum rn2483_pin_mode rn2483_get_pin_mode(struct rn2483_desc_t *inst,
                                         enum rn2483_pin pin)
{
    return inst->pins[pin].mode;
}

void rn2483_set_output(struct rn2483_desc_t *inst, enum rn2483_pin pin,
                       uint8_t value)
{
    // If the value has not changed, don't bother sending command to radio
    if (inst->pins[pin].value == value) {
        return;
    }
    
    // Update pin value in cache
    inst->pins[pin].value = value;
    inst->pins[pin].value_dirty = 1;
    // Run service to start sending command to radio if possible
    rn2483_service(inst);
}

void rn2483_toggle_output(struct rn2483_desc_t *inst, enum rn2483_pin pin)
{
    // Update pin value in cache
    inst->pins[pin].value = !inst->pins[pin].value;
    inst->pins[pin].value_dirty = 1;
    // Run service to start sending command to radio if possible
    rn2483_service(inst);
}
