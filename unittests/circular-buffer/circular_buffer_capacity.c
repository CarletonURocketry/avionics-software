#include <string.h>
#include "common.c"

/*
 *  circular_buffer_capacity() gets the capacity of a circular buffer,
 *  circular buffer.  
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));

    // Test with a capacity of 0.
    {
        cb.capacity = 0;
        uint16_t ret = circular_buffer_capacity(&cb);

        ut_assert(ret == 0);
    }

    // Test with a capacity that is not 0.
    {
        cb.capacity = UINT16_MAX;
        uint16_t ret = circular_buffer_capacity(&cb);

        ut_assert(ret == UINT16_MAX);
    }

    return UT_PASS;
}

