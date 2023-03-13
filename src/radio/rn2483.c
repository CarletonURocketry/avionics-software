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
                  struct rn2483_lora_settings_t *settings)
{
    inst->uart = uart;
    inst->settings = settings;
    
    // Initialize GPIO pins to inputs
    for (enum rn2483_pin pin = 0; pin < RN2483_NUM_PINS; pin++) {
        inst->pins[pin].raw = (RN2483_PIN_DESC_MODE(RN2483_PIN_MODE_INPUT) |
                               RN2483_PIN_DESC_MODE_DIRTY);
    }
    
    // Start by reseting module
    inst->state = RN2483_RESET;
    
    inst->waiting_for_line = 0;
    inst->cmd_ready = 0;
    inst->position = 0;
    inst->reset_try_count = 0;
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

/**
 *  Start the process of canceling an ongoing receive operation if there is one.
 */
static void cancel_receive (struct rn2483_desc_t *inst)
{
    if (inst->state == RN2483_RECEIVE) {
        // We are in the process of sending the receive command or are waiting
        // for the first response to the receive command, we need to indicate
        // that we should abort the receive (if possible) as soon as we are done
        // starting it
        inst->state = RN2483_RECEIVE_ABORT;
    } else if ((inst->version >= RN2483_MIN_FW_RXSTOP) &&
               (inst->state == RN2483_RX_OK_WAIT)) {
        // rxstop command is supported and we are in receive wait, we should
        // cancel the ongoing reception
        inst->state = RN2483_RXSTOP;
        inst->waiting_for_line = 0;
    }
}


enum rn2483_operation_result rn2483_send (struct rn2483_desc_t *inst,
                                          uint8_t const *data, uint8_t length,
                                          uint8_t *transaction_id)
{
    // Check that we are not already sending something and for message length
    if (inst->send_buffer != NULL) {
        return RN2483_OP_BUSY;
    } else if (length > 127) {
        // Message is too large to be sent
        return RN2483_OP_TOO_LONG;
    }
    
    // Check for an open transaction slot
    uint8_t id = find_send_trans(inst, RN2483_SEND_TRANS_INVALID);
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
    } else {
        // Cancel the receive operation if there is one ongoing
        cancel_receive(inst);
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
                          (inst->state != RN2483_RX_OK_WAIT) &&
                          (inst->state != RN2483_RX_DATA_WAIT)) {
        // No receive to cancel
        return RN2483_OP_BAD_STATE;
    } else if (inst->state == RN2483_FAILED) {
        return RN2483_OP_BAD_STATE;
    }
    
    // Disable continuous receive
    inst->receive = 0;
    
    // Cancel any ongoing receive operation
    cancel_receive(inst);
    
    // We may be able to continue on to a new state immediately
    rn2483_service(inst);
    
    return RN2483_OP_SUCCESS;
}



void rn2483_update_settings(struct rn2483_desc_t *inst)
{
    if (inst->state == RN2483_IDLE) {
        // If we are idle, jump right to first initialization state
        inst->state = RN2483_WRITE_WDT;
        inst->settings_dirty = 0;
        inst->frequency_dirty = 0;
    } else {
        // Cancel the receive operation if there is one ongoing
        cancel_receive(inst);
        // Indicate that we should update the settings asap
        inst->settings_dirty = 1;
    }

    // Start the update right away if possible
    rn2483_service(inst);
}

void rn2483_update_frequency_settings(struct rn2483_desc_t *inst)
{
    if (inst->state == RN2483_IDLE) {
        // If we are idle, jump right to updating the frequency
        inst->state = RN2483_UPDATE_FREQ;
        inst->frequency_dirty = 0;
    } else {
        // Cancel the receive operation if there is one ongoing
        cancel_receive(inst);
        // Indicate that we should update the frequency asap
        inst->frequency_dirty = 1;
    }

    // Start the update right away if possible
    rn2483_service(inst);
}



void rn2483_poll_gpio(struct rn2483_desc_t *inst)
{
    uint8_t dirty = 0;
    for (enum rn2483_pin pin = 0; pin < RN2483_NUM_PINS; pin++) {
        // Mark value dirty if pin has been explicitly set as an
        // input
        if (((inst->pins[pin].mode == RN2483_PIN_MODE_INPUT) ||
             (inst->pins[pin].mode == RN2483_PIN_MODE_ANALOG)) &&
            (inst->pins[pin].mode_explicit)) {
            inst->pins[pin].value_dirty = 1;
            
            dirty = 1;
        }
    }
    
    if (dirty) {
        // If any pins need to be polled, cancel any ongoing receive so that we
        // can poll the gpio right away.
        cancel_receive(inst);
    }
    
    rn2483_service(inst);
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
        inst->pins[pin].mode_explicit = 1;
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
    
    // Cancel any ongoing receive so that we update the gpio status right away.
    cancel_receive(inst);
    
    // Run service to start sending command to radio if possible
    rn2483_service(inst);
    
    return 0;
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
    // Cancel any ongoing receive so that we update the gpio status right away.
    cancel_receive(inst);
    // Run service to start sending command to radio if possible
    rn2483_service(inst);
}

void rn2483_toggle_output(struct rn2483_desc_t *inst, enum rn2483_pin pin)
{
    // Update pin value in cache
    inst->pins[pin].value = !inst->pins[pin].value;
    inst->pins[pin].value_dirty = 1;
    // Cancel any ongoing receive so that we update the gpio status right away.
    cancel_receive(inst);
    // Run service to start sending command to radio if possible
    rn2483_service(inst);
}
