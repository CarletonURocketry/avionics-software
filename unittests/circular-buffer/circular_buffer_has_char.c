#include <string.h>
#include "common.c"

/*
 *  circular_buffer_has_char() checks if a given character is contained in a
 *  circular buffer.
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));
    uint8_t buffer[512];
    cb.buffer = buffer;

    // Search for a character in a buffer that does not wrap.
    {
        cb.capacity = 128;
        cb.head = 0;
        cb.tail = 27;
        cb.length = 27;
        memset(buffer, 0, sizeof(buffer));
        strcpy((char*)buffer, "abcdefghijklmnopqrstuvwxyz!");
        int ret = circular_buffer_has_char(&cb, '!');

        ut_assert(ret);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 27);
        ut_assert(cb.length == 27);
    }

    // Search for a character in a buffer that wraps.
    {
        cb.capacity = 27;
        cb.head = 10;
        cb.tail = 10;
        cb.length = 27;
        memset(buffer, 0, sizeof(buffer));
        strcpy((char*)buffer, "abcdefghijklmnopqrstuvwxyz!");
        int ret = circular_buffer_has_char(&cb, '!');

        ut_assert(ret);
        ut_assert(cb.head == 10);
        ut_assert(cb.tail == 10);
        ut_assert(cb.length == 27);
    }

    // Search for a character that is not in the buffer.
    {
        cb.capacity = 512;
        cb.head = 87;
        cb.tail = 14;
        cb.length = 439;
        memset(buffer, 0, sizeof(buffer));
        strcpy((char*)buffer, "abcdefghijklmnopqrstuvwxyz!");
        int ret = circular_buffer_has_char(&cb, '\n');

        ut_assert(!ret);
        ut_assert(cb.head == 87);
        ut_assert(cb.tail == 14);
        ut_assert(cb.length == 439);
    }

    // Search for a character in an empty buffer.
    {
        cb.capacity = 256;
        cb.head = 155;
        cb.tail = 155;
        cb.length = 0;
        memset(buffer, '*', sizeof(buffer));
        int ret = circular_buffer_has_char(&cb, '*');

        ut_assert(!ret);
        ut_assert(cb.head == 155);
        ut_assert(cb.tail == 155);
        ut_assert(cb.length == 0);
    }

    return UT_PASS;
}

