#include <string.h>
#include "common.c"

/*
 *  circular_buffer_is_empty() checks if a circular buffer is empty.  
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));

    // Check if an empty buffer is recognized.
    {
        cb.length = 0;
        int ret = circular_buffer_is_empty(&cb);

        ut_assert(ret);
    }

    // Check if a non-empty buffer is recognized.
    {
        cb.length = 800;
        int ret = circular_buffer_is_empty(&cb);

        ut_assert(!ret);
    }

    return UT_PASS;
}

