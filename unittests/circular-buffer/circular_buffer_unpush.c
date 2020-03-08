#include <string.h>
#include "common.c"

/*
 *  circular_buffer_unpush() removes an item from the tail of the buffer.
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));

    // Simple unpush.
    {
        cb.capacity = 8192;
        cb.head = 2000;
        cb.tail = 5723;
        cb.length = 3723;
        interrupts_status = INTERRUPTS_ENABLED;
        int ret = circular_buffer_unpush(&cb);

        ut_assert(!ret);
        ut_assert(cb.head == 2000);
        ut_assert(cb.tail == 5722);
        ut_assert(cb.length == 3722);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Unpush when tail is at 0.
    {
        cb.capacity = 12;
        cb.head = 10;
        cb.tail = 0;
        cb.length = 2;
        interrupts_status = INTERRUPTS_ENABLED;
        int ret = circular_buffer_unpush(&cb);

        ut_assert(!ret);
        ut_assert(cb.head == 10);
        ut_assert(cb.tail == 11);
        ut_assert(cb.length == 1);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Try to unpush when buffer is empty,
    {
        cb.capacity = 256;
        cb.head = 197;
        cb.tail = 197;
        cb.length = 0;
        interrupts_status = INTERRUPTS_ENABLED;
        int ret = circular_buffer_unpush(&cb);

        ut_assert(ret);
        ut_assert(cb.head == 197);
        ut_assert(cb.tail == 197);
        ut_assert(cb.length == 0);
        ut_assert(interrupts_status == INTERRUPTS_ENABLED);
    }

    return UT_PASS;
}

