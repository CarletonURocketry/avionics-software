#include <string.h>
#include "common.c"

/*
 *  circular_buffer_move_head() allows the head position to be updated.
 *
 *  This function can be used to update the buffer state after a DMA
 *  transaction.
 */


int main (int argc, char **argv)
{
    struct circular_buffer_t cb;
    memset(&cb, 0, sizeof(cb));

    // Move head of buffer when head is lower than tail.
    {
        cb.capacity = 500;
        cb.head = 60;
        cb.tail = 284;
        cb.length = 224;
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_move_head(&cb, 100);

        ut_assert(cb.head == 160);
        ut_assert(cb.tail == 284);
        ut_assert(cb.length == 124);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Move head of buffer when head is higher than tail.
    {
        cb.capacity = 64;
        cb.head = 54;
        cb.tail = 20;
        cb.length = 30;
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_move_head(&cb, 6);

        ut_assert(cb.head == 60);
        ut_assert(cb.tail == 20);
        ut_assert(cb.length == 24);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Move head so that it loops back to 0.
    {
        cb.capacity = 256;
        cb.head = 210;
        cb.tail = 57;
        cb.length = 103;
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_move_head(&cb, 46);

        ut_assert(cb.head == 0);
        ut_assert(cb.tail == 57);
        ut_assert(cb.length == 57);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Try to move head of empty buffer.
    {
        cb.capacity = 2048;
        cb.head = 600;
        cb.tail = 600;
        cb.length = 0;
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_move_head(&cb, 70);

        ut_assert(cb.head == 600);
        ut_assert(cb.tail == 600);
        ut_assert(cb.length == 0);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Move head by more than length when head is less than tail.
    {
        cb.capacity = 512;
        cb.head = 40;
        cb.tail = 100;
        cb.length = 60;
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_move_head(&cb, 130);

        ut_assert(cb.head == 100);
        ut_assert(cb.tail == 100);
        ut_assert(cb.length == 0);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    // Move head by more than length when head is more than tail.
    {
        cb.capacity = 768;
        cb.head = 320;
        cb.tail = 160;
        cb.length = 608;
        interrupts_status = INTERRUPTS_ENABLED;
        circular_buffer_move_head(&cb, 750);

        ut_assert(cb.head == 160);
        ut_assert(cb.tail == 160);
        ut_assert(cb.length == 0);
        ut_assert(interrupts_status == INTERRUPTS_CYCLED);
    }

    return UT_PASS;
}

