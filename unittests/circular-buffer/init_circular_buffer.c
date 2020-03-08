#include <string.h>
#include "common.c"

/*
 *  init_circular_buffer() initializes a new circular buffer.  
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;

    // Starting with a zeroed out struct, initialize a circular buffer with
    // memory at at address 0x1000 and a length of 80.
    {
        memset(&cb, 0, sizeof(cb));
        init_circular_buffer(&cb, (uint8_t*)0x1000, 80);

        ut_assert(cb.buffer == (uint8_t*)0x1000);
        ut_assert(cb.capacity == 80);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 0);
        ut_assert(cb.length == 0);
    }

    // Starting with a struct that is not zeroed, initialize a circular buffer
    // with memory at at address 0x12345678 and a length of 1024.
    {
        memset(&cb, 0xAA, sizeof(cb));
        init_circular_buffer(&cb, (uint8_t*)0x12345678, 1024);

        ut_assert(cb.buffer == (uint8_t*)0x12345678);
        ut_assert(cb.capacity == 1024);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 0);
        ut_assert(cb.length == 0);
    }

    return UT_PASS;
}

