#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_receive() starts a reception and sets the receive callback.
 */

static struct rn2483_desc_t radio_descriptor;

#include "rn2483_service_stubs.c"

static void reset (void)
{
    sercom_uart_has_line_call_count = 0;
    executed_state = (enum rn2483_state)-1;
    radio_descriptor.version = RN2483_VERSION(1, 0, 5);
}

int main (int argc, char **argv)
{
    // Start receiving from idle state.
    {
        reset();
        radio_descriptor.state = RN2483_IDLE;
        radio_descriptor.receive = 0;
        radio_descriptor.waiting_for_line = 0;
        radio_descriptor.receive_callback = (rn2483_recv_callback)0xABCDEF01;
        radio_descriptor.callback_context = (void*)0x10FEDCBA;
        sercom_uart_has_line_retval = 0;
        rn2483_recv_callback cb = (rn2483_recv_callback)0xABABABAB;
        void *context = (void*)0x12345678;

        enum rn2483_operation_result ret = rn2483_receive(&radio_descriptor, cb,
                                                          context);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_SUCCESS);
        // Check that the receive bit is set and that the callback and context
        // where stored properly
        ut_assert(radio_descriptor.receive == 1);
        ut_assert(radio_descriptor.receive_callback == cb);
        ut_assert(radio_descriptor.callback_context == context);
        // Check that we did not change state
        ut_assert(radio_descriptor.state == RN2483_IDLE);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the idle state right away
        ut_assert(executed_state == RN2483_IDLE);
    }

    // Try to start receiving when already receiving.
    {
        reset();
        radio_descriptor.state = RN2483_RECEIVE;
        radio_descriptor.receive = 1;
        radio_descriptor.waiting_for_line = 0;
        radio_descriptor.receive_callback = (rn2483_recv_callback)0xABCDEF01;
        radio_descriptor.callback_context = (void*)0x10FEDCBA;
        sercom_uart_has_line_retval = 0;
        rn2483_recv_callback cb = (rn2483_recv_callback)0xABABABAB;
        void *context = (void*)0x12345678;

        enum rn2483_operation_result ret = rn2483_receive(&radio_descriptor, cb,
                                                          context);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_BUSY);
        // Check that the receive bit is still set and that the callback and
        // context where not changed
        ut_assert(radio_descriptor.receive == 1);
        ut_assert(radio_descriptor.receive_callback ==
                                            (rn2483_recv_callback)0xABCDEF01);
        ut_assert(radio_descriptor.callback_context == (void*)0x10FEDCBA);
        // Check that we did not change state
        ut_assert(radio_descriptor.state == RN2483_RECEIVE);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we did not run any state right away
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    // Try to start receiving when driver is in failed state.
    {
        reset();
        radio_descriptor.state = RN2483_FAILED;
        radio_descriptor.receive = 0;
        radio_descriptor.waiting_for_line = 0;
        radio_descriptor.receive_callback = (rn2483_recv_callback)0xABCDEF01;
        radio_descriptor.callback_context = (void*)0x10FEDCBA;
        sercom_uart_has_line_retval = 0;
        rn2483_recv_callback cb = (rn2483_recv_callback)0xABABABAB;
        void *context = (void*)0x12345678;

        enum rn2483_operation_result ret = rn2483_receive(&radio_descriptor, cb,
                                                          context);

        // Check that we got the correct return value
        ut_assert(ret == RN2483_OP_BAD_STATE);
        // Check that the receive bit was not set and that the callback and
        // context where not changed
        ut_assert(radio_descriptor.receive == 0);
        ut_assert(radio_descriptor.receive_callback ==
                  (rn2483_recv_callback)0xABCDEF01);
        ut_assert(radio_descriptor.callback_context == (void*)0x10FEDCBA);
        // Check that we did not change state
        ut_assert(radio_descriptor.state == RN2483_FAILED);
        // Check that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we did not run any state right away
        ut_assert(executed_state == (enum rn2483_state)-1);
    }

    return UT_PASS;
}
