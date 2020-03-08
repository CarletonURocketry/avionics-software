#include <string.h>
#include "common.c"

/*
 *  circular_buffer_peak() returns the next item from a circular buffer without
 *  removing it.
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
        cb.capacity = 200;
        cb.head = 50;
        cb.tail = 15;
        cb.length = 165;
        interrupts_status = INTERRUPTS_ENABLED;
        uint8_t value;
        int ret = circular_buffer_peak(&cb, &value);

        ut_assert(!ret);
        ut_assert(cb.head == 50);
        ut_assert(cb.tail == 15);
        ut_assert(cb.length == 165);
        ut_assert(value == buffer[50]);
    }

    // Try to pop from an empty buffer.
    {
        cb.capacity = 64;
        cb.head = 63;
        cb.tail = 63;
        cb.length = 0;
        interrupts_status = INTERRUPTS_ENABLED;
        uint8_t value;
        int ret = circular_buffer_peak(&cb, &value);

        ut_assert(ret);
        ut_assert(cb.head == 63);
        ut_assert(cb.tail == 63);
        ut_assert(cb.length == 0);
    }

    return UT_PASS;
}

