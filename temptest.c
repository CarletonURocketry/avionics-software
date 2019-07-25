#include <stdio.h>
#include <stdint.h>

int main (int argc, char **argv)
{
    /* Read values from Temperature Log Row */
    // Room temperature (in hundred nanodegrees celsius)
    int32_t temp_r = ((30 * 10000000) +
                      (1 * 1000000));
    
    // Hot temperature (in hundred nanodegrees celsius)
    int32_t temp_h = ((84 * 10000000) +
                      (7 * 1000000));
    
    printf("temp_r: %d\ntemp_h: %d\n\n", temp_r, temp_h);
    
    // 1 V reference actual voltage for room temperature measurment
    uint16_t int1v_r = 996;
    
    // 1 V reference actual voltage for hot temperature measurment
    uint16_t int1v_h = 992;
    
    // ADC value for room temperature measurment
    uint16_t adc_r_val = 0xAAF;
    
    // ADC value for hot temperature measurment
    uint16_t adc_h_val = 0xCA0;
    
    /* Get measured ADC value */
    uint16_t adc_m_val = 43500;
    
    // 0.4637926171 degc/mvolt
    
    /* Compute coefficients to convert ADC values to hundred nanovolts */
    uint32_t adc_r_co = ((10000 * int1v_r) + 2048) / 4095;
    uint32_t adc_h_co = ((10000 * int1v_h) + 2048) / 4095;
    uint32_t adc_m_course_co = ((10000 * 1000) + 32768) / 65535;
    
    printf("adc_r_co: %u\nadc_h_co: %u\nadc_m_course_co: %u\n\n", adc_r_co, adc_h_co, adc_m_course_co);
    
    /* Compute voltages (in hundred nanovolts) */
    uint32_t v_adc_r = adc_r_val * adc_r_co;
    uint32_t v_adc_h = adc_h_val * adc_h_co;
    uint32_t v_adc_m = adc_m_val * adc_m_course_co;
    
    printf("v_adc_r: %u\nv_adc_h: %u\nv_adc_m: %u\n\n", v_adc_r, v_adc_h, v_adc_m);
    
    /* Compute course temp (in hundred nanodegrees celsius) */
    int32_t denominator = v_adc_h - v_adc_r;
    printf("denominator: %d\n", denominator);
    
    int32_t delta_v = v_adc_m - v_adc_r;
    printf("delta_v: %d\n", delta_v);
    
    int32_t delta_t = temp_h - temp_r;
    printf("delta_t: %d\n", delta_t);
    
    int64_t numerator = (int64_t)delta_v * delta_t;
    printf("numerator: %lld\n", numerator);
    
    int32_t temp_c = temp_r + (((int64_t)(v_adc_m - v_adc_r) * (temp_h - temp_r)) / denominator);
    printf("Course temp: %f C (%d)\n", temp_c / 10000000.0, (temp_c / 100000));
}
