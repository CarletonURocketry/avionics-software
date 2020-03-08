#include <string.h>
#include "common.c"

/*
 *  circular_buffer_unused() determines how much space is free inside of a
 *  circular buffer.  
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));

    // Test with an empty buffer.
    {
        cb.capacity = 1024;
        cb.length = 0;
        uint16_t ret = circular_buffer_unused(&cb);

        ut_assert(ret == 1024);
    }

    // Test with a buffer that is not empty.
    {
        cb.capacity = 128;
        cb.length = 96;
        uint16_t ret = circular_buffer_unused(&cb);

        ut_assert(ret == 32);
    }

    return UT_PASS;
}

