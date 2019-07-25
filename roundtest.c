#include <stdio.h>
#include <stdint.h>
//#include <math.h>
#include <stdlib.h>

static const uint16_t tc_prescaler_values[] = {1, 2, 4, 8, 16, 64, 512, 1024};


int main (int argc, char **argv)
{
    uint32_t clock = 8000000UL;
    uint32_t target = 780UL;

    uint8_t prescaler = 0xFF;
    uint16_t top;
    uint32_t min_error = UINT32_MAX;
    
    for (int8_t i = 7; i >= 0; i--) {
        printf("%d: ", i);
        uint32_t t = (uint32_t)(((((uint64_t)clock << 32) / (tc_prescaler_values[i] * 1000)) * target) >> 32);
        if ((t - 1) > UINT16_MAX) {
            printf("prescaler = %u, top = %u (too large)\n", tc_prescaler_values[i], t);
            break;
        }
        int32_t p = ((uint64_t)tc_prescaler_values[i] * 1000 * t) / clock;
        uint32_t error = abs(p - (int32_t)target);

        printf("prescaler = %u, top = %u, error = %u)\n", tc_prescaler_values[i], t, error);
        
        if (error == 0) {
            min_error = error;
            top = (uint16_t)(t - 1);
            prescaler = i;
            break;
        } else if (error < min_error) {
            min_error = error;
            top = (uint16_t)t;
            prescaler = i;
        }
    }

    if (prescaler == 0xFF) {
        printf("Failed to find timer value.\n");
    } else {
        printf("Result: prescaler = %u, top = %u, error = %u\n", tc_prescaler_values[prescaler], top, min_error);
    }
}
