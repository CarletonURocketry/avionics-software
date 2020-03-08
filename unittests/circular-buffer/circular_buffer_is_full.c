#include <string.h>
#include "common.c"

/*
 *  circular_buffer_is_full() checks if a circular buffer is full.  
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));

    // Check if a full buffer is recognized.
    {
        cb.capacity = 1024;
        cb.length = 1024;
        int ret = circular_buffer_is_full(&cb);

        ut_assert(ret);
    }

    // Check if a buffer that is not full is recognized.
    {
        cb.capacity = 256;
        cb.length = 168;
        int ret = circular_buffer_is_full(&cb);

        ut_assert(!ret);
    }

    return UT_PASS;
}

