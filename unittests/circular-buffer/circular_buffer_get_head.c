#include <string.h>
#include "common.c"

/*
 *  circular_buffer_get_head() retrieves a pointer to head of the buffer and
 *  returns the length of the largest contiguous block of data in the buffer
 *  starting at the head.
 *
 *  This function can be used to setup DMA transaction out of the buffer.
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));

    // Get length of buffer where head is at 0.
    {
        cb.buffer = (uint8_t*)0x1000;
        cb.capacity = 256;
        cb.head = 0;
        cb.tail = 10;
        cb.length = 10;
        uint8_t *head;
        uint16_t ret = circular_buffer_get_head(&cb, &head);

        ut_assert(ret == 10);
        ut_assert(head == cb.buffer + cb.head);
    }

    // Get length of buffer where head is after tail.
    {
        cb.buffer = (uint8_t*)0x12345678;
        cb.capacity = 32768;
        cb.head = 24500;
        cb.tail = 10000;
        cb.length = 18268;
        uint8_t *head;
        uint16_t ret = circular_buffer_get_head(&cb, &head);

        ut_assert(ret == 8268);
        ut_assert(head == cb.buffer + cb.head);
    }

    // Get length of empty buffer.
    {
        cb.buffer = (uint8_t*)0xAAAA;
        cb.capacity = 123;
        cb.head = 0;
        cb.tail = 0;
        cb.length = 0;
        uint8_t *head;
        uint16_t ret = circular_buffer_get_head(&cb, &head);

        ut_assert(ret == 0);
        ut_assert(head == cb.buffer + cb.head);
    }

    return UT_PASS;
}

