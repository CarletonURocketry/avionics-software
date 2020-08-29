#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_send() starts a new transmition if possible.
 */

#define TEST_DATA_PTR ((const uint8_t*)0x12345678)

static struct rn2483_desc_t radio_descriptor;
static uint8_t send_transaction_id;
static int set_send_trans_state_call_count;

#include "rn2483_service_stubs.c"

static void reset (void)
{
    set_send_trans_state_call_count = 0;
    sercom_uart_has_line_call_count = 0;
    executed_state = (enum rn2483_state)-1;
    radio_descriptor.version = RN2483_VERSION(1, 0, 5);
}

int main (int argc, char **argv)
{
    // Send a message of valid length while driver is in idle state and a
    // transaction slot is available.
    {
        reset();
        radio_descriptor.send_buffer = NULL;
        radio_descriptor.state = RN2483_IDLE;
        send_transaction_id = 1;
        sercom_uart_has_line_retval = 0;
        const uint8_t send_length = 10;

        uint8_t t_id;
        enum rn2483_operation_result ret = rn2483_send(&radio_descriptor,
                                                       TEST_DATA_PTR,
                                                       send_length, &t_id);

        // Check that we got the correct return value and that t_id was set
        ut_assert(ret == RN2483_OP_SUCCESS);
        ut_assert(t_id == send_transaction_id);
        // Check that the send buffer and length where updated properly
        ut_assert(radio_descriptor.send_buffer == TEST_DATA_PTR);
        ut_assert(radio_descriptor.send_length == send_length);
        // Check that we moved into the correct state
        ut_assert(radio_descriptor.state == RN2483_SEND);
        // Check that the set_send_trans_state function was called
        ut_assert(set_send_trans_state_call_count == 1);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the send state right away
        ut_assert(executed_state == RN2483_SEND);
    }

    // Send a message of valid length while a receive is ongoing.
    {
        reset();
        radio_descriptor.send_buffer = NULL;
        radio_descriptor.state = RN2483_RX_OK_WAIT;
        send_transaction_id = 2;
        sercom_uart_has_line_retval = 1;
        const uint8_t send_length = 96;

        uint8_t t_id;
        enum rn2483_operation_result ret = rn2483_send(&radio_descriptor,
                                                       TEST_DATA_PTR,
                                                       send_length, &t_id);

        // Check that we got the correct return value and that t_id was set
        ut_assert(ret == RN2483_OP_SUCCESS);
        ut_assert(t_id == send_transaction_id);
        // Check that the send buffer and length where updated properly
        ut_assert(radio_descriptor.send_buffer == TEST_DATA_PTR);
        ut_assert(radio_descriptor.send_length == send_length);
        // Check that we moved into the correct state
        ut_assert(radio_descriptor.state == RN2483_RXSTOP);
        // Check that the set_send_trans_state function was called
        ut_assert(set_send_trans_state_call_count == 1);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that the rxstop state was run right away
        ut_assert(executed_state == RN2483_RXSTOP);
    }

    // Try to send data when a transmition is already ongoing.
    {
        reset();
        radio_descriptor.send_buffer = (const uint8_t*)0x87654321;
        radio_descriptor.send_length = 42;
        radio_descriptor.state = RN2483_SEND_WAIT;
        send_transaction_id = 0;
        sercom_uart_has_line_retval = 1;
        const uint8_t send_length = 2;

        uint8_t t_id;
        enum rn2483_operation_result ret = rn2483_send(&radio_descriptor,
                                                       TEST_DATA_PTR,
                                                       send_length, &t_id);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_BUSY);
        // Check that the send buffer and length where not updated
        ut_assert(radio_descriptor.send_buffer == (const uint8_t*)0x87654321);
        ut_assert(radio_descriptor.send_length == 42);
        // Check that we remained the same state
        ut_assert(radio_descriptor.state == RN2483_SEND_WAIT);
        // Check that the set_send_trans_state function was not called
        ut_assert(set_send_trans_state_call_count == 0);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that no state was run right away
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    // Try to send too much data.
    {
        reset();
        radio_descriptor.send_buffer = NULL;
        radio_descriptor.send_length = 0;
        radio_descriptor.state = RN2483_IDLE;
        send_transaction_id = 0;
        sercom_uart_has_line_retval = 1;
        const uint8_t send_length = UINT8_MAX;

        uint8_t t_id;
        enum rn2483_operation_result ret = rn2483_send(&radio_descriptor,
                                                       TEST_DATA_PTR,
                                                       send_length, &t_id);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_TOO_LONG);
        // Check that the send buffer and length where not updated
        ut_assert(radio_descriptor.send_buffer == NULL);
        ut_assert(radio_descriptor.send_length == 0);
        // Check that we remained the same state
        ut_assert(radio_descriptor.state == RN2483_IDLE);
        // Check that the set_send_trans_state function was not called
        ut_assert(set_send_trans_state_call_count == 0);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that no state was run right away
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    // Try to send data when there are no free transaction slots.
    {
        reset();
        radio_descriptor.send_buffer = NULL;
        radio_descriptor.send_length = 0;
        radio_descriptor.state = RN2483_IDLE;
        send_transaction_id = RN2483_NUM_SEND_TRANSACTIONS;
        sercom_uart_has_line_retval = 1;
        const uint8_t send_length = 39;

        uint8_t t_id;
        enum rn2483_operation_result ret = rn2483_send(&radio_descriptor,
                                                       TEST_DATA_PTR,
                                                       send_length, &t_id);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_BUSY);
        // Check that the send buffer and length where not updated
        ut_assert(radio_descriptor.send_buffer == NULL);
        ut_assert(radio_descriptor.send_length == 0);
        // Check that we remained the same state
        ut_assert(radio_descriptor.state == RN2483_IDLE);
        // Check that the set_send_trans_state function was not called
        ut_assert(set_send_trans_state_call_count == 0);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that no state was run right away
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    return UT_PASS;
}



uint8_t find_send_trans(struct rn2483_desc_t *inst,
                               enum rn2483_send_trans_state state)
{
    ut_assert(inst == &radio_descriptor);
    ut_assert(state == RN2483_SEND_TRANS_INVALID);

    return send_transaction_id;
}

void set_send_trans_state(struct rn2483_desc_t *inst, int n,
                          enum rn2483_send_trans_state state)
{
    ut_assert(inst == &radio_descriptor);
    ut_assert(n == send_transaction_id);
    ut_assert(state == RN2483_SEND_TRANS_PENDING);

    set_send_trans_state_call_count++;
}
