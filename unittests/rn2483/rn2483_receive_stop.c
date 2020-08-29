#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_receive_stop() stops and ongoing reception.
 */

static struct rn2483_desc_t radio_descriptor;

#include "rn2483_service_stubs.c"

static void reset (void)
{
    sercom_uart_has_line_call_count = 0;
    executed_state = (enum rn2483_state)-1;
    radio_descriptor.version = RN2483_VERSION(1, 0, 5);
    radio_descriptor.waiting_for_line = 0;
    sercom_uart_has_line_retval = 0;
}

int main (int argc, char **argv)
{
    // Cancel an ongoing receive where receive bit is set.
    {
        reset();
        radio_descriptor.state = RN2483_IDLE;
        radio_descriptor.receive = 1;

        enum rn2483_operation_result ret = rn2483_receive_stop(
                                                            &radio_descriptor);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_SUCCESS);
        // Check that the receive bit is cleared
        ut_assert(radio_descriptor.receive == 0);
        // Check that we did not change state
        ut_assert(radio_descriptor.state == RN2483_IDLE);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the idle state right away
        ut_assert(executed_state == RN2483_IDLE);
    }

    // Cancel an ongoing receive where receive bit is not set but we are in the
    // receive state.
    {
        reset();
        radio_descriptor.state = RN2483_RECEIVE;
        radio_descriptor.receive = 0;

        enum rn2483_operation_result ret = rn2483_receive_stop(
                                                               &radio_descriptor);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_SUCCESS);
        // Check that the receive bit is cleared
        ut_assert(radio_descriptor.receive == 0);
        // Check that we changed to the receive abort state
        ut_assert(radio_descriptor.state == RN2483_RECEIVE_ABORT);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the receive abort state right away
        ut_assert(executed_state == RN2483_RECEIVE_ABORT);
    }

    // Cancel an ongoing receive where receive bit is not set but we are in the
    // receive ok wait state.
    {
        reset();
        radio_descriptor.state = RN2483_RX_OK_WAIT;
        radio_descriptor.receive = 0;
        radio_descriptor.waiting_for_line = 1;

        enum rn2483_operation_result ret = rn2483_receive_stop(
                                                               &radio_descriptor);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_SUCCESS);
        // Check that the receive bit is cleared
        ut_assert(radio_descriptor.receive == 0);
        // Check that we changed to the rx stop state
        ut_assert(radio_descriptor.state == RN2483_RXSTOP);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the rx stop right away
        ut_assert(executed_state == RN2483_RXSTOP);
    }

    // Cancel an ongoing receive where receive bit is not set but we are in the
    // receive data wait state.
    {
        reset();
        radio_descriptor.state = RN2483_RX_DATA_WAIT;
        radio_descriptor.receive = 0;

        enum rn2483_operation_result ret = rn2483_receive_stop(
                                                            &radio_descriptor);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_SUCCESS);
        // Check that the receive bit is cleared
        ut_assert(radio_descriptor.receive == 0);
        // Check that we did not chang state
        ut_assert(radio_descriptor.state == RN2483_RX_DATA_WAIT);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the rx stop right away
        ut_assert(executed_state == RN2483_RX_DATA_WAIT);
    }

    // Try to cancel a receive when none is ongoing.
    {
        reset();
        radio_descriptor.state = RN2483_IDLE;
        radio_descriptor.receive = 0;

        enum rn2483_operation_result ret = rn2483_receive_stop(
                                                               &radio_descriptor);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_BAD_STATE);
        // Check that the receive bit is still cleared
        ut_assert(radio_descriptor.receive == 0);
        // Check that we did not change state
        ut_assert(radio_descriptor.state == RN2483_IDLE);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we did not run the service function
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    // Try to cancel a receive when the driver is in the failed state.
    {
        reset();
        radio_descriptor.state = RN2483_FAILED;
        radio_descriptor.receive = 1;

        enum rn2483_operation_result ret = rn2483_receive_stop(
                                                               &radio_descriptor);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_BAD_STATE);
        // Check that the receive bit is not changed
        ut_assert(radio_descriptor.receive == 1);
        // Check that we did not change state
        ut_assert(radio_descriptor.state == RN2483_FAILED);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we did not run the service function
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    return UT_PASS;
}
