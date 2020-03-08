#include <string.h>
#include "common.c"

/*
 *  circular_buffer_push() adds an entry to a circular buffer.
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
        circular_buffer_push(&cb, 0xAA);

        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 1);
        ut_assert(cb.length == 1);
        buffer_copy[0] = 0xAA;
        ut_assert(memcmp(buffer, buffer_copy, sizeof(buffer)) == 0);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Push to a buffer that is not empty.
    {
        cb.capacity = 256;
        cb.head = 80;
        cb.tail = 219;
        cb.length = 139;
        memcpy(buffer_copy, buffer, sizeof(buffer));
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_push(&cb, 0xAA);

        ut_assert(cb.head == 80);
        ut_assert(cb.tail == 220);
        ut_assert(cb.length == 140);
        buffer_copy[219] = 0xAA;
        ut_assert(memcmp(buffer, buffer_copy, sizeof(buffer)) == 0);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Push to a buffer where the tail is at the end.
    {
        cb.capacity = 1024;
        cb.head = 247;
        cb.tail = 1023;
        cb.length = 776;
        memcpy(buffer_copy, buffer, sizeof(buffer));
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_push(&cb, 0xAA);

        ut_assert(cb.head == 247);
        ut_assert(cb.tail == 0);
        ut_assert(cb.length == 777);
        buffer_copy[1023] = 0xAA;
        ut_assert(memcmp(buffer, buffer_copy, sizeof(buffer)) == 0);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Push to a full buffer.
    {
        cb.capacity = 512;
        cb.head = 388;
        cb.tail = 388;
        cb.length = 512;
        memcpy(buffer_copy, buffer, sizeof(buffer));
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_push(&cb, 0xAA);

        ut_assert(cb.head == 389);
        ut_assert(cb.tail == 389);
        ut_assert(cb.length == 512);
        buffer_copy[388] = 0xAA;
        ut_assert(memcmp(buffer, buffer_copy, sizeof(buffer)) == 0);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Push to a full buffer where the tail is at the end.
    {
        cb.capacity = 8;
        cb.head = 7;
        cb.tail = 7;
        cb.length = 8;
        memcpy(buffer_copy, buffer, sizeof(buffer));
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_push(&cb, 0xAA);

        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 0);
        ut_assert(cb.length == 8);
        buffer_copy[7] = 0xAA;
        ut_assert(memcmp(buffer, buffer_copy, sizeof(buffer)) == 0);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    return UT_PASS;
}

