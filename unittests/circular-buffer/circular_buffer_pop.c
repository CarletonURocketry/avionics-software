#include <string.h>
#include "common.c"

/*
 *  circular_buffer_pop() removes and item from a circular buffer and returns
 *  it.
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));
    uint8_t buffer[256];
    for (size_t i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (uint8_t)(i & 0xFF);
    }
    cb.buffer = buffer;

    // Pop from a buffer with a single element.
    {
        cb.capacity = 128;
        cb.head = 0;
        cb.tail = 1;
        cb.length = 1;
        interrupts_status = INTERRUPTS_ENABLED;
        uint8_t value;
        int ret = circular_buffer_pop(&cb, &value);

        ut_assert(!ret);
        ut_assert(cb.head == 1);
        ut_assert(cb.tail == 1);
        ut_assert(cb.length == 0);
        ut_assert(value == buffer[0]);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Pop from a buffer where head will be reset to 0.
    {
        cb.capacity = 96;
        cb.head = 95;
        cb.tail = 78;
        cb.length = 79;
        interrupts_status = INTERRUPTS_ENABLED;
        uint8_t value;
        int ret = circular_buffer_pop(&cb, &value);

        ut_assert(!ret);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 78);
        ut_assert(cb.length == 78);
        ut_assert(value == buffer[95]);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Try to pop from an empty buffer.
    {
        cb.capacity = 24;
        cb.head = 15;
        cb.tail = 15;
        cb.length = 0;
        interrupts_status = INTERRUPTS_ENABLED;
        uint8_t value;
        int ret = circular_buffer_pop(&cb, &value);

        ut_assert(ret);
        ut_assert(cb.head == 15);
        ut_assert(cb.tail == 15);
        ut_assert(cb.length == 0);
        ut_assert(interrupts_status == INTERRUPTS_ENABLED);
    }

    return UT_PASS;
}

