#include <string.h>
#include "common.c"

/*
 *  circular_buffer_clear() empties a circular buffer.  
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;

    // Clear a buffer.
    {
        cb.buffer = (uint8_t*)0x87654321;
        cb.capacity = 16238;
        cb.head = 789;
        cb.tail = 10945;
        cb.length = 10156;
        circular_buffer_clear(&cb);

        ut_assert(cb.buffer == (uint8_t*)0x87654321);
        ut_assert(cb.capacity == 16238);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 0);
        ut_assert(cb.length == 0);
    }

    // Clear a zeroed buffer.
    {
        memset(&cb, 0, sizeof(cb));
        circular_buffer_clear(&cb);

        ut_assert(cb.buffer == (uint8_t*)0);
        ut_assert(cb.capacity == 0);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 0);
        ut_assert(cb.length == 0);
    }

    return UT_PASS;
}

