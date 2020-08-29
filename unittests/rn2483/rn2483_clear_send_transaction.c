#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_clear_send_transaction() clears a transaction info slot.
 */

struct rn2483_desc_t radio_descriptor;
static int set_id_num;

int main (int argc, char **argv)
{
    // Clear transaction 0.
    {
        rn2483_clear_send_transaction(&radio_descriptor, 0);

        ut_assert(set_id_num == 0);
    }

    // Clear transaction 1.
    {
        rn2483_clear_send_transaction(&radio_descriptor, 1);

        ut_assert(set_id_num == 1);
    }

    return UT_PASS;
}

void set_send_trans_state(struct rn2483_desc_t *inst, int n,
                          enum rn2483_send_trans_state state)
{
    ut_assert(inst == &radio_descriptor);
    set_id_num = n;
    ut_assert(state == RN2483_SEND_TRANS_INVALID);
}
