#include <string.h>
#include "common.c"

/*
 *  circular_buffer_try__push() adds an entry to a circular buffer only if there
 *  is space in the buffer.
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));
    uint8_t buffer[1024];
    for (size_t i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (uint8_t)(i & 0xFF);
    }
    cb.buffer = buffer;
    uint8_t buffer_copy[sizeof(buffer)];

    // Push to empty buffer.
    {
        cb.capacity = 64;
        cb.head = 0;
        cb.tail = 0;
        cb.length = 0;
        memcpy(buffer_copy, buffer, sizeof(buffer));
        interrupts_status = INTERRUPTS_ENABLED;
        int ret = circular_buffer_try_push(&cb, 0xAA);

        ut_assert(ret == 0);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 1);
        ut_assert(cb.length == 1);
        buffer_copy[0] = 0xAA;
        ut_assert(memcmp(buffer, buffer_copy, sizeof(buffer)) == 0);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }


    // Try to push to a full buffer.
    {
        cb.capacity = 96;
        cb.head = 0;
        cb.tail = 0;
        cb.length = 96;
        memcpy(buffer_copy, buffer, sizeof(buffer));
        interrupts_status = INTERRUPTS_ENABLED;
        int ret = circular_buffer_try_push(&cb, 0xAA);

        ut_assert(ret == 1);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 0);
        ut_assert(cb.length == 96);
        ut_assert(memcmp(buffer, buffer_copy, sizeof(buffer)) == 0);
        ut_assert(interrupts_status == INTERRUPTS_ENABLED);
    }

    return UT_PASS;
}

