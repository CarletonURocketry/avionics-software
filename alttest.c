#include <stdio.h>
#include <stdint.h>
#include <math.h>

int main (int argc, char **argv)
{
    uint16_t prom_values[] = {40127, 36924, 23317, 23282, 33464, 28312};
    
    uint32_t d1 = 9085466;
    uint32_t d2 = 8569150;
    
    // Calculate temperature
    int32_t dT = d2 - ((int32_t)prom_values[4] * 256);
    int32_t temperature = 2000 + ((dT * ((int32_t)prom_values[5]) /
                                      8388608));
    // Calculate temperature compensated pressure
    int64_t offset = (((int64_t)prom_values[1] * 65536) +
                      (((int64_t)prom_values[3] * dT) / 128));
    int64_t sensitivity = (((int64_t)prom_values[0] * 32768) +
                           (((int64_t)prom_values[2] * dT) /
                            256));
    int32_t pressure = ((((d1 * sensitivity) / 2097152) - offset) /
                      32768);
    
    printf("Temp: %d, Pressure: %d\n", temperature, pressure);
    
    
    // Calculate altitude
    float p0 = 1013.25;
    
    float t = (float)(temperature + 27315) / 100;
    float p = ((float)pressure) / 100;
    float alt = ((powf((p0 / p), 0.1902225604) - 1.0) * t)/0.0065;
    
    printf("Temp: %f, Pressure: %f, Altitude: %f\n", t, p, alt);
}
