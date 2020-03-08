#include <unittest.h>
#include SOURCE_C

/*
 *  sercom_get_inst_num() is a helper function which returns the instance number
 *  for a SERCOM when given a pointer to its registers.
 */


int main (int argc, char **argv)
{
    // Get the instance number for SERCOM0.
    {
        int8_t ret = sercom_get_inst_num(SERCOM0);

        ut_assert(ret == 0);
    }

    // Get the instance number for SERCOM1.
    {
        int8_t ret = sercom_get_inst_num(SERCOM1);

        ut_assert(ret == 1);
    }

    // Get the instance number for SERCOM2.
    {
        int8_t ret = sercom_get_inst_num(SERCOM2);

        ut_assert(ret == 2);
    }

    // Get the instance number for SERCOM3.
    {
        int8_t ret = sercom_get_inst_num(SERCOM3);

        ut_assert(ret == 3);
    }

    // Get the instance number for SERCOM4.
    {
        int8_t ret = sercom_get_inst_num(SERCOM4);

        ut_assert(ret == 4);
    }

    // Get the instance number for SERCOM5.
    {
        int8_t ret = sercom_get_inst_num(SERCOM5);

        ut_assert(ret == 5);
    }

    // Try to get the instance num for something that is not a pointer to the
    // registers for a SERCOM instance.
    {
        int8_t ret = sercom_get_inst_num(NULL);

        ut_assert(ret == -1);
    }

    return UT_PASS;
}
