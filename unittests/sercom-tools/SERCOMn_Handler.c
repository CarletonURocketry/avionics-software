#include <unittest.h>
#include SOURCE_C

/*
 *  The SERCOMn_Handler() functions are ISRs which simply call a function from a
 *  table.
 */

static unsigned called_functions;

#define HANDLER_FUNC(n, s, i) static void n (Sercom *sercom, uint8_t inst_num,\
                                             void *context)\
{ \
    ut_assert(sercom == s);\
    ut_assert(inst_num == i);\
    ut_assert((uintptr_t)context == (1 << i));\
    called_functions |= (1 << i);\
}

HANDLER_FUNC(handler_0, SERCOM0, 0)
HANDLER_FUNC(handler_1, SERCOM1, 1)
HANDLER_FUNC(handler_2, SERCOM2, 2)
HANDLER_FUNC(handler_3, SERCOM3, 3)
HANDLER_FUNC(handler_4, SERCOM4, 4)
HANDLER_FUNC(handler_5, SERCOM5, 5)


int main (int argc, char **argv)
{
    // Initialize function table
    sercom_handlers[0] = (struct sercom_handler_t){ .handler = handler_0,
                                                    .state = (void*)(1 << 0) };
    sercom_handlers[1] = (struct sercom_handler_t){ .handler = handler_1,
                                                    .state = (void*)(1 << 1) };
    sercom_handlers[2] = (struct sercom_handler_t){ .handler = handler_2,
                                                    .state = (void*)(1 << 2) };
    sercom_handlers[3] = (struct sercom_handler_t){ .handler = handler_3,
                                                    .state = (void*)(1 << 3) };
    sercom_handlers[4] = (struct sercom_handler_t){ .handler = handler_4,
                                                    .state = (void*)(1 << 4) };
    sercom_handlers[5] = (struct sercom_handler_t){ .handler = handler_5,
                                                    .state = (void*)(1 << 5) };

    // Call handler for SERCOM0 ISR.
    {
        called_functions = 0;
        SERCOM0_Handler();

        ut_assert(called_functions == (1 << 0));
    }

    // Call handler for SERCOM1 ISR.
    {
        called_functions = 0;
        SERCOM1_Handler();

        ut_assert(called_functions == (1 << 1));
    }

    // Call handler for SERCOM2 ISR.
    {
        called_functions = 0;
        SERCOM2_Handler();

        ut_assert(called_functions == (1 << 2));
    }

    // Call handler for SERCOM3 ISR.
    {
        called_functions = 0;
        SERCOM3_Handler();

        ut_assert(called_functions == (1 << 3));
    }

    // Call handler for SERCOM4 ISR.
    {
        called_functions = 0;
        SERCOM4_Handler();

        ut_assert(called_functions == (1 << 4));
    }

    // Call handler for SERCOM5 ISR.
    {
        called_functions = 0;
        SERCOM5_Handler();

        ut_assert(called_functions == (1 << 5));
    }

    return UT_PASS;
}
