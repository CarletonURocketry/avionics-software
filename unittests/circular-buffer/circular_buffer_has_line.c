#include <string.h>
#include "common.c"

/*
 *  circular_buffer_has_line() checks a circular buffer contains a line
 *  delimited by "\r\n".
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));
    uint8_t buffer[512];
    cb.buffer = buffer;

    // Search for a line in a buffer that does not wrap.
    {
        cb.capacity = 128;
        cb.head = 0;
        cb.tail = 28;
        cb.length = 28;
        memset(buffer, 0, sizeof(buffer));
        strcpy((char*)buffer, "abcdefghijklmnopqrstuvwxyz\r\n");
        int ret = circular_buffer_has_line(&cb);

        ut_assert(ret);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 28);
        ut_assert(cb.length == 28);
    }

    // Search for a line in a buffer that wraps.
    {
        cb.capacity = 512;
        cb.head = 500;
        cb.tail = 350;
        cb.length = 362;
        memset(buffer, 0, sizeof(buffer));
        strcpy((char*)buffer + 300, "abcdefghijklmnopqrstuvwxyz\r\n");
        int ret = circular_buffer_has_line(&cb);

        ut_assert(ret);
        ut_assert(cb.head == 500);
        ut_assert(cb.tail == 350);
        ut_assert(cb.length == 362);
    }

    // Search for a line in a buffer that does not have a line.
    {
        cb.capacity = 64;
        cb.head = 0;
        cb.tail = 0;
        cb.length = 64;
        memset(buffer, 0, sizeof(buffer));
        strcpy((char*)buffer + 10, "abcdefghij\nklmnopqrstuvwxyz\r");
        int ret = circular_buffer_has_line(&cb);

        ut_assert(!ret);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 0);
        ut_assert(cb.length == 64);
    }

    // Search for a line in a buffer that does not have a line.
    {
        cb.capacity = 28;
        cb.head = 0;
        cb.tail = 0;
        cb.length = 28;
        memset(buffer, 0, sizeof(buffer));
        strcpy((char*)buffer, "abcdefghij\nklmnopqrstuvwxyz\r\n");
        int ret = circular_buffer_has_line(&cb);

        ut_assert(!ret);
        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 0);
        ut_assert(cb.length == 28);
    }

    // Search for a line in an empty buffer.
    {
        cb.capacity = 256;
        cb.head = 154;
        cb.tail = 154;
        cb.length = 0;
        memset(buffer, '\r', sizeof(buffer));
        buffer[10] = '\n';
        buffer[155] = '\n';
        int ret = circular_buffer_has_line(&cb);

        ut_assert(!ret);
        ut_assert(cb.head == 154);
        ut_assert(cb.tail == 154);
        ut_assert(cb.length == 0);
    }

    return UT_PASS;
}

