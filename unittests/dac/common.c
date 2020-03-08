#include <samd21j18a.h>

#undef DAC
static Dac dac;
#define DAC (&dac)

#include SOURCE_C

