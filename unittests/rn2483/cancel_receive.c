#include <unittest.h>
#include SOURCE_C

/*
 *  cancel_receive() cancels a receive transction if one is ongoing.
 */

int main (int argc, char **argv)
{
    // Simple test where radio is not currently receiving. The driver state
    // should not change.
    {
        struct rn2483_desc_t radio_descriptor;
        radio_descriptor.state = RN2483_IDLE;
        radio_descriptor.version = RN2483_VERSION(1, 0, 4);
        radio_descriptor.waiting_for_line = 0;

        cancel_receive(&radio_descriptor);

        ut_assert(radio_descriptor.state == RN2483_IDLE);
        ut_assert(radio_descriptor.waiting_for_line == 0);
    }

    // Radio is not currently receiving and firmare support rx_stop command. The
    // driver state should not be changed.
    {
        struct rn2483_desc_t radio_descriptor;
        radio_descriptor.state = RN2483_SET_PIN_MODE;
        radio_descriptor.version = RN2483_VERSION(1, 0, 5);
        radio_descriptor.waiting_for_line = 1;

        cancel_receive(&radio_descriptor);

        ut_assert(radio_descriptor.state == RN2483_SET_PIN_MODE);
        ut_assert(radio_descriptor.waiting_for_line == 1);
    }

    // Test where driver is currently sending the receive command or waiting for
    // the first response to the receive command. The driver should be brought
    // into the receive abort state so that the receive can be canceled once the
    // first receive command response has been received.
    {
        struct rn2483_desc_t radio_descriptor;
        radio_descriptor.state = RN2483_RECEIVE;
        radio_descriptor.version = RN2483_VERSION(1, 0, 4);
        radio_descriptor.waiting_for_line = 1;

        cancel_receive(&radio_descriptor);

        ut_assert(radio_descriptor.state == RN2483_RECEIVE_ABORT);
        ut_assert(radio_descriptor.waiting_for_line == 1);
    }

    // Test where radio firmware version supports the rx_stop command and the
    // driver is waiting for data after having already received the first
    // receive command response. The driver should be brought into the RX_STOP
    // state and the watiting_for_line bit should be cleared.
    {
        struct rn2483_desc_t radio_descriptor;
        radio_descriptor.state = RN2483_RX_OK_WAIT;
        radio_descriptor.version = RN2483_VERSION(1, 0, 5);
        radio_descriptor.waiting_for_line = 1;

        cancel_receive(&radio_descriptor);

        ut_assert(radio_descriptor.state == RN2483_RXSTOP);
        ut_assert(radio_descriptor.waiting_for_line == 0);
    }

    // Test where radio firmware version does not support the rx_stop command
    // and the driver is waiting for data after having already received the
    // first receive command response. The driver state should not change (we
    // have to wait out the receive timeout).
    {
        struct rn2483_desc_t radio_descriptor;
        radio_descriptor.state = RN2483_RX_OK_WAIT;
        radio_descriptor.version = RN2483_VERSION(1, 0, 4);
        radio_descriptor.waiting_for_line = 1;

        cancel_receive(&radio_descriptor);

        ut_assert(radio_descriptor.state == RN2483_RX_OK_WAIT);
        ut_assert(radio_descriptor.waiting_for_line == 1);
    }

    return UT_PASS;
}
