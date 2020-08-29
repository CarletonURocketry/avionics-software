#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_update_settings() indicates that the radio settings have changed and
 *  need to be written to the radio again.
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
    // Run rn2483_update_settings when in idle state.
    {
        reset();
        radio_descriptor.state = RN2483_IDLE;
        radio_descriptor.settings_dirty = 1;
        radio_descriptor.frequency_dirty = 1;

        rn2483_update_settings(&radio_descriptor);

        // Check that we went into the first settings update state
        ut_assert(radio_descriptor.state == RN2483_WRITE_WDT);
        // Check that the settings and frequency dirty flags where cleared
        ut_assert(radio_descriptor.settings_dirty == 0);
        ut_assert(radio_descriptor.frequency_dirty == 0);
        // Checl that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the first settings update state right away
        ut_assert(executed_state == RN2483_WRITE_WDT);
    }

    // Run rn2483_update_settings when not in idle state.
    {
        reset();
        radio_descriptor.state = RN2483_GET_SNR;
        radio_descriptor.settings_dirty = 1;
        radio_descriptor.frequency_dirty = 0;

        rn2483_update_settings(&radio_descriptor);

        // Check that the current state did not change
        ut_assert(radio_descriptor.state == RN2483_GET_SNR);
        // Check that the settings dirty flag was set
        ut_assert(radio_descriptor.settings_dirty == 1);
        // Check that the frequency dirty flag was not changed
        ut_assert(radio_descriptor.frequency_dirty == 0);
        // Checl that the sercom_uart_has_line function was not called
        ut_assert(sercom_uart_has_line_call_count == 0);
        // Check that we ran the current state again right away
        ut_assert(executed_state == RN2483_GET_SNR);
    }


    return UT_PASS;
}
