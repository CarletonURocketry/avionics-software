#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_get_send_state() fetches the current state of a send transction.
 */

int main (int argc, char **argv)
{
    // Get the state for transaction 0.
    {
        struct rn2483_desc_t radio_descriptor;
        const uint8_t transaction_id = 0;
        radio_descriptor.send_transactions = (RN2483_SEND_TRANS_INVALID <<
                            (transaction_id * RN2483_SEND_TRANSACTION_SIZE));

        enum rn2483_send_trans_state ret = rn2483_get_send_state(
                                                            &radio_descriptor,
                                                            transaction_id);

        ut_assert(ret == RN2483_SEND_TRANS_INVALID);
    }

    // Get the state for transaction 2.
    {
        struct rn2483_desc_t radio_descriptor;
        const uint8_t transaction_id = 2;
        radio_descriptor.send_transactions = (RN2483_SEND_TRANS_FAILED <<
                            (transaction_id * RN2483_SEND_TRANSACTION_SIZE));

        enum rn2483_send_trans_state ret = rn2483_get_send_state(
                                                            &radio_descriptor,
                                                            transaction_id);

        ut_assert(ret == RN2483_SEND_TRANS_FAILED);
    }

    return UT_PASS;
}
