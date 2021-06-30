/**
 * @file ms5611.c
 * @desc Driver for MS5611 barometric pressure sensor
 * @author Samuel Dewan
 * @date 2019-06-04
 * Last Author:
 * Last Edited On:
 */

#include "ms5611.h"

#include "ms5611-commands.h"

#include <math.h>


#define CONV_WAIT_TIME MS_TO_MILLIS(10)

static const uint8_t reset_cmd = MS5611_CMD_RESET;
static const uint8_t adc_conv_d1_cmd = MS5611_CMD_D1 | MS5611_OSR_4096;
static const uint8_t adc_conv_d2_cmd = MS5611_CMD_D2 | MS5611_OSR_4096;


void init_ms5611 (struct ms5611_desc_t *inst,
                  struct sercom_i2c_desc_t *i2c_inst, uint8_t csb,
                  uint32_t period, uint8_t calculate_altitude)
{
    inst->address = MS5611_ADDR | ((csb) << MS5611_ADDR_CSB_Pos);
    inst->period = period;
    inst->calc_altitude = !!calculate_altitude;
    
    inst->i2c_inst = i2c_inst;
    
    inst->p0_set = 0;
    
    // Start by reading the factory calibration data
    inst->state = MS5611_READ_C1;
    
    ms5611_service(inst);
}

/**
 * Handle a state in which a value should be read from the sensor.
 *
 * @param inst The MS5611 driver instance
 * @param width The width of the data to be read in bytes
 * @param cmd The command to be sent to the MS5611 to get the data
 * @param result_adder The address in which the result should be stored
 *
 * @return 1 if the FSM should procceed to the next state, 0 otherwise
 */
static uint8_t handle_read_state (struct ms5611_desc_t *inst, uint8_t width,
                                  uint8_t cmd, uint8_t *result_addr)
{
    if (inst->i2c_in_progress) {
        // Just finished read transaction
        enum i2c_transaction_state state = sercom_i2c_transaction_state(
                                                    inst->i2c_inst, inst->t_id);
        sercom_i2c_clear_transaction(inst->i2c_inst, inst->t_id);
        inst->i2c_in_progress = 0;
        
        if (state == I2C_STATE_DONE) {
            // Got result!
            // go to next state
            return 1;
        }
        // I2C transaction failed, start a new one
    }
    // Need to start read transaction
    inst->i2c_in_progress = !sercom_i2c_start_reg_read(inst->i2c_inst,
                                                       &inst->t_id,
                                                       inst->address, cmd,
                                                       result_addr, width);
    // Check if transaction is complete on next call
    return 0;
}

/**
 * Handle a state in which a convertion should be run
 *
 * @param inst The MS5611 driver instance
 * @param cmd The command to be sent to start the transaction
 *
 * @return 1 if the FSM should procceed to the next state, 0 otherwise
 */
static uint8_t handle_write_state (struct ms5611_desc_t *inst,
                                   uint8_t const* cmd)
{
    if (inst->i2c_in_progress) {
        // Just finished command transaction
        enum i2c_transaction_state state = sercom_i2c_transaction_state(
                                                    inst->i2c_inst, inst->t_id);
        sercom_i2c_clear_transaction(inst->i2c_inst, inst->t_id);
        inst->i2c_in_progress = 0;
        
        if (state == I2C_STATE_DONE) {
            // Got result!
            // Swap the bytes so that they are in the correct order
            inst->prom_values[0] = __builtin_bswap16(inst->prom_values[0]);
            // go to next state
            return 1;
        }
        // I2C transaction failed, start a new one
    }
    // Need to send command
    inst->i2c_in_progress = !sercom_i2c_start_generic(inst->i2c_inst,
                                                      &inst->t_id,
                                                      inst->address, cmd, 1,
                                                      NULL, 0);
    // Stay in same state
    return 0;
}

/**
 *  Perform calculations to find temperature, pressure and altitude based on
 *  most recent values from sensor.
 */
static void do_calculations (struct ms5611_desc_t *inst)
{
    // Calculate temperature
    int32_t dT = inst->d2 - ((int32_t)inst->prom_values[4] * 256);
    inst->temperature = 2000 + ((dT * ((int32_t)inst->prom_values[5]) /
                                 8388608));
    // Second order temperature compensation
    int32_t t2 = 0;
    int64_t off2 = 0;
    int64_t sens2 = 0;
    if (inst->temperature < 2000) {
        t2 = (dT * dT) / 2147483648UL;
        uint64_t a = (inst->temperature - 2000) * (inst->temperature - 2000);
        off2 = 5 * (a / 2);
        sens2 = 5 * (a / 4);
        if (inst->temperature < -1500) {
            a = (inst->temperature - 1500) * (inst->temperature - 1500);
            off2 += 7 * a;
            sens2 += 11 * (a / 2);
        }
    }
    inst->temperature -= t2;
    // Calculate temperature compensated pressure
    int64_t offset = (((int64_t)inst->prom_values[1] * 65536) +
                      (((int64_t)inst->prom_values[3] * dT) / 128)) - off2;
    int64_t sensitivity = ((((int64_t)inst->prom_values[0] * 32768) +
                            (((int64_t)inst->prom_values[2] * dT) / 256)) -
                           sens2);
    inst->pressure = ((((inst->d1 * sensitivity) / 2097152) - offset) / 32768);
    // Set p0 if it has not already been set
    if (!inst->p0_set) {
        inst->p0 = ((float)inst->pressure) / 100.0f;
        inst->p0_set = 1;
    }
    // Calculate altitude
    if (inst->calc_altitude) {
        float t = ((float)(inst->temperature + 27315)) / 100;
        float p = ((float)inst->pressure) / 100;
        inst->altitude = (((powf((inst->p0 / p), 0.1902225604f) - 1.0f) * t) /
                          0.0065f);
    }
}

void ms5611_service (struct ms5611_desc_t *inst)
{
    // If this is a wait state there is no point in continuing unless the I2C
    // transaction has completed
    if (inst->i2c_in_progress && !sercom_i2c_transaction_done(inst->i2c_inst,
                                                              inst->t_id)) {
        // Still waiting for transaction to complete
        return;
    }
    
    switch (inst->state) {
        case MS5611_RESET:
            if (handle_write_state(inst, &reset_cmd)) {
                // Go to the next state
                inst->state = MS5611_RESET_WAIT;
                // Record time
                inst->conv_start_time = millis;
            } else {
                break;
            }
            /* fall through */
        case MS5611_RESET_WAIT:
            if ((millis - inst->conv_start_time) < CONV_WAIT_TIME) {
                // Not yet time to move on
                break;
            }
            inst->state = MS5611_READ_C1;
            /* fall through */
        case MS5611_READ_C1:
            // In process of reading C1 from the sensor
            if (handle_read_state(inst, 2,
                                  (MS5611_CMD_PROM_READ | MS5611_PROM_C1),
                                  (uint8_t*)(&(inst->prom_values[0])))) {
                // Swap bytes to correct endianness
                inst->prom_values[0] = __builtin_bswap16(inst->prom_values[0]);
                // Go to the next state
                inst->state = MS5611_READ_C2;
            } else {
                break;
            }
            /* fall through */
        case MS5611_READ_C2:
            // In process of reading C2 from the sensor
            if (handle_read_state(inst, 2,
                                  (MS5611_CMD_PROM_READ | MS5611_PROM_C2),
                                  (uint8_t*)(&(inst->prom_values[1])))) {
                // Swap bytes to correct endianness
                inst->prom_values[1] = __builtin_bswap16(inst->prom_values[1]);
                // Go to the next state
                inst->state = MS5611_READ_C3;
            } else {
                break;
            }
            /* fall through */
        case MS5611_READ_C3:
            // In process of reading C3 from the sensor
            if (handle_read_state(inst, 2,
                                  (MS5611_CMD_PROM_READ | MS5611_PROM_C3),
                                  (uint8_t*)(&(inst->prom_values[2])))) {
                // Swap bytes to correct endianness
                inst->prom_values[2] = __builtin_bswap16(inst->prom_values[2]);
                // Go to the next state
                inst->state = MS5611_READ_C4;
            } else {
                break;
            }
            /* fall through */
        case MS5611_READ_C4:
            // In process of reading C4 from the sensor
            if (handle_read_state(inst, 2,
                                  (MS5611_CMD_PROM_READ | MS5611_PROM_C4),
                                  (uint8_t*)(&(inst->prom_values[3])))) {
                // Swap bytes to correct endianness
                inst->prom_values[3] = __builtin_bswap16(inst->prom_values[3]);
                // Go to the next state
                inst->state = MS5611_READ_C5;
            } else {
                break;
            }
            /* fall through */
        case MS5611_READ_C5:
            // In process of reading C5 from the sensor
            if (handle_read_state(inst, 2,
                                  (MS5611_CMD_PROM_READ | MS5611_PROM_C5),
                                  (uint8_t*)(&(inst->prom_values[4])))) {
                // Swap bytes to correct endianness
                inst->prom_values[4] = __builtin_bswap16(inst->prom_values[4]);
                // Go to the next state
                inst->state = MS5611_READ_C6;
            } else {
                break;
            }
            /* fall through */
        case MS5611_READ_C6:
            // In process of reading C6 from the sensor
            if (handle_read_state(inst, 2,
                                  (MS5611_CMD_PROM_READ | MS5611_PROM_C6),
                                  (uint8_t*)(&(inst->prom_values[5])))) {
                // Swap bytes to correct endianness
                inst->prom_values[5] = __builtin_bswap16(inst->prom_values[5]);
                // Go to the next state
                inst->state = MS5611_IDLE;
            } else {
                break;
            }
            /* fall through */
        case MS5611_IDLE:
            // Waiting for it to be time to start a new read
            if ((millis - inst->last_reading_time) < inst->period) {
                // Not yet time to move on
                break;
            }
            inst->last_reading_time = millis;
            inst->state = MS5611_CONVERT_PRES;
            /* fall through */
        case MS5611_CONVERT_PRES:
            // In process of sending command to take presure measurment
            if (handle_write_state(inst, &adc_conv_d1_cmd)) {
                // Go to the next state
                inst->state = MS5611_CONVERT_PRES_WAIT;
                // Record time
                inst->conv_start_time = millis;
            } else {
                break;
            }
            /* fall through */
        case MS5611_CONVERT_PRES_WAIT:
            // Waiting for it to be time to read result
            if ((millis - inst->conv_start_time) < CONV_WAIT_TIME) {
                // Not yet time to move on
                break;
            }
            inst->state = MS5611_READ_PRES;
            /* fall through */
        case MS5611_READ_PRES:
            // In process of reading back pressure measurment
            if (handle_read_state(inst, 3, MS5611_CMD_ADC_READ,
                                  ((uint8_t*)(&(inst->d1))) + 1)) {
                *((uint8_t*)(&(inst->d1))) = 0;
                // Swap bytes to correct endianness
                inst->d1 = __builtin_bswap32(inst->d1);
                // Go to the next state
                inst->state = MS5611_CONVERT_TEMP;
            } else {
                break;
            }
            /* fall through */
        case MS5611_CONVERT_TEMP:
            // In process of sending command to take temperature measurment
            if (handle_write_state(inst, &adc_conv_d2_cmd)) {
                // Go to the next state
                inst->state = MS5611_CONVERT_TEMP_WAIT;
                // Record time
                inst->conv_start_time = millis;
            } else {
                break;
            }
            /* fall through */
        case MS5611_CONVERT_TEMP_WAIT:
            // Waiting for it to be time to read result
            if ((millis - inst->conv_start_time) < CONV_WAIT_TIME) {
                // Not yet time to move on
                break;
            }
            inst->state = MS5611_READ_TEMP;
            /* fall through */
        case MS5611_READ_TEMP:
            // In process of reading back temperature measurment
            if (!(handle_read_state(inst, 3, MS5611_CMD_ADC_READ,
                                   ((uint8_t*)(&(inst->d2))) + 1))) {
                break;
            }
            // Swap bytes to correct endianness
            *((uint8_t*)(&(inst->d2))) = 0;
            inst->d2 = __builtin_bswap32(inst->d2);
            do_calculations(inst);
            // Wait till next measurment should be taken
            inst->state = MS5611_IDLE;
            break;
        case MS5611_FAILED:
            // Something has gone wrong
            break;
    }
}
