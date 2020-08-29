#include <unittest.h>
#include SOURCE_C

/*
 *  rn2483_service() is the service function to be run in each iteration of the
 *  main loop.
 */


/* Test state */

/** Radio instance descriptor */
static struct rn2483_desc_t radio_descriptor;

/** Posible types for test steps */
enum test_step_type {
    /** A step where a state handler function should be called */
    STATE = 0,
    /** A step where the sercom_uart_has_line should be called */
    LINE = 1,
    /** Indicates the last step in the list, this step is a placholder that
        should not correlate with any action */
    END = 0xFFFFFFFF
};

/** Descriptor for a test step */
struct test_step_info {
    /** Type of test step */
    enum test_step_type type;
    /** The value that inst->state should have during this step (ignored for
        LINE steps and should equal the state value for the next valid state) */
    enum rn2483_state state;
    /** The value taht waiting_for_line should be set to after this step
        (ignored for LINE steps) */
    int wait_for_line;
    /** The return value of this step */
    int ret_val;
};

static int step_index;
static struct test_step_info *step_list;

static int state_zero (struct rn2483_desc_t *inst);
static int state_one (struct rn2483_desc_t *inst);

static void reset(enum rn2483_state first_state, int waiting_for_line)
{
    step_index = 0;
    radio_descriptor.state = first_state;
    radio_descriptor.waiting_for_line = waiting_for_line;
    radio_descriptor.uart = (void*)0xAAAAAAAA;
}

int main (int argc, char **argv)
{
    // Simple test the runs a single state and returns
    {
        // Start with state 1, not waiting for line
        reset(1, 0);
        step_list = (struct test_step_info[]){
            // Do state one and return 0 to indicate that we are at the end
            { .type = STATE, .state = 1, .wait_for_line = 0, .ret_val = 0 },
            // End marker
            { .type = END }
        };

        rn2483_service(&radio_descriptor);

        // Check that we went through all of the expected steps
        ut_assert(step_list[step_index].type == END);
    }

    // Simple test that check for a new line on the serial input and does not
    // find one
    {
        // Start with state 1, waiting for line
        reset(1, 1);
        step_list = (struct test_step_info[]){
            // Indicate that there is no new line available
            { .type = LINE, .state = 1, .wait_for_line = 1, .ret_val = 0 },
            // End marker
            { .type = END }
        };

        rn2483_service(&radio_descriptor);

        // Check that we went through all of the expected steps
        ut_assert(step_list[step_index].type == END);
    }

    // Test that goes through two states.
    {
        // Start in state 0, not waiting for line
        reset(0, 0);
        step_list = (struct test_step_info[]){
            // Run state 0, do not wait for line afterwords
            { .type = STATE, .state = 0, .wait_for_line = 0, .ret_val = 1 },
            // Run state 1, return afterwords
            { .type = STATE, .state = 1, .wait_for_line = 0, .ret_val = 0 },
            // End marker
            { .type = END }
        };

        rn2483_service(&radio_descriptor);

        // Check that we went through all of the expected steps
        ut_assert(step_list[step_index].type == END);
    }

    // Run a state and then wait for a line that is not available.
    {
        // Start in state 1, not waiting for line
        reset(1, 0);
        step_list = (struct test_step_info[]){
            // Run state 1, wait for line before moving to state 0 next
            { .type = STATE, .state = 1, .wait_for_line = 1, .ret_val = 1 },
            // Indicate that there is no new line available
            { .type = LINE, .state = 0, .wait_for_line = 1, .ret_val = 0 },
            // End marker
            { .type = END }
        };

        rn2483_service(&radio_descriptor);

        // Check that we went through all of the expected steps
        ut_assert(step_list[step_index].type == END);
    }

    // Run a state then wait for another line that is available.
    {
        // Start in state 0, not waiting for line
        reset(0, 0);
        step_list = (struct test_step_info[]){
            // Run state 0, wait for a line before moving to state 1 next
            { .type = STATE, .state = 0, .wait_for_line = 1, .ret_val = 1 },
            // Indicate that a line is available
            { .type = LINE, .state = 1, .wait_for_line = 1, .ret_val = 1 },
            // Run state 1 and return aftwords
            { .type = STATE, .state = 1, .wait_for_line = 0, .ret_val = 0 },
            // End marker
            { .type = END }
        };

        rn2483_service(&radio_descriptor);

        // Check that we went through all of the expected steps
        ut_assert(step_list[step_index].type == END);
    }

    // Wait for a line that has been received, then run a few states
    // interspersed with waiting and then end when waiting for a line that has
    // not yet been received.
    {
        reset(1, 1);
        step_list = (struct test_step_info[]){
            // Indicate that a line has been received
            { .type = LINE, .state = 1, .wait_for_line = 1, .ret_val = 1 },
            // Run state 1 and wait for another line afterwords
            { .type = STATE, .state = 1, .wait_for_line = 1, .ret_val = 1 },
            // Inficate that a line has been received
            { .type = LINE, .state = 0, .wait_for_line = 1, .ret_val = 1 },
            // Run state 0 and move to state 1 next
            { .type = STATE, .state = 0, .wait_for_line = 0, .ret_val = 1 },
            // Run state 1 and move to state 1 next
            { .type = STATE, .state = 1, .wait_for_line = 0, .ret_val = 1 },
            // Run state 1 and wait for a line afterwords
            { .type = STATE, .state = 1, .wait_for_line = 1, .ret_val = 1 },
            // Indicate that a line has been received
            { .type = LINE, .state = 0, .wait_for_line = 1, .ret_val = 1 },
            // Run state 0 and wait for a line aftewords
            { .type = STATE, .state = 0, .wait_for_line = 1, .ret_val = 1 },
            // Indicate that no line has been received
            { .type = LINE, .state = 0, .wait_for_line = 1, .ret_val = 0 },
            // End marker
            { .type = END }
        };

        rn2483_service(&radio_descriptor);

        // Check that we went through all of the expected steps
        ut_assert(step_list[step_index].type == END);
    }

    return UT_PASS;
}

/* Stub variables */
const rn2483_state_handler_t rn2483_state_handlers[] = {
    state_zero,
    state_one
};

/* Stub functions */
uint8_t sercom_uart_has_line (struct sercom_uart_desc_t *uart)
{
    // Get the current step information and increment step_index
    struct test_step_info current_step_info = step_list[step_index++];
    // Check that we are performing the correct action
    ut_assert(current_step_info.type == LINE);
    // Check that waiting_for_line bit is set
    ut_assert(radio_descriptor.waiting_for_line);
    // Check that the uart argument is correct
    ut_assert(radio_descriptor.uart == uart);
    // Return value from step info
    return current_step_info.ret_val;
}

static int do_test_state(struct rn2483_desc_t *inst, enum rn2483_state state)
{
    // Get the current step information and increment step_index
    struct test_step_info current_step_info = step_list[step_index++];
    // Check that we are performing the correct action
    ut_assert(current_step_info.type == STATE);
    // Check that the correct state function is being run
    ut_assert(current_step_info.state == state);
    // Get the information for the next step
    struct test_step_info next_step_info = step_list[step_index];
    // Set inst->state to the next state value
    inst->state = next_step_info.state;
    // Set the wiating_for_line flag if required
    inst->waiting_for_line = current_step_info.wait_for_line;
    // Return value from step info
    return current_step_info.ret_val;
}

static int state_zero (struct rn2483_desc_t *inst)
{
    return do_test_state(inst, 0);
}

static int state_one (struct rn2483_desc_t *inst)
{
    return do_test_state(inst, 1);
}

