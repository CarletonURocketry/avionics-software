/**
 * @file hpsens.c
 * @desc Driver for Honeywell HSCMAND060PA3A3 Pressure sensor
 * @author Darwin Jull  
 * @date 2021-07-14
 * Last Author:
 * Last Edited On:
 */


#include "hpsens.h"

#include <math.h>

/** Max/Min output values for HSCMAND060PA3A3*/
static const int16_t pmax = 60;
static const int16_t pmin = 0;
static const uint16_t outmax = 14745;
static const uint16_t outmin = 1638;

//* Returns pressure in milliPSI */
static uint32_t hpsens_pressure_math (uint16_t pressure_output)
{
    return ((((pressure_output - outmin) * (pmax - pmin) * 1000) / (outmax-outmin)) + pmin * 1000);
}

static uint32_t hpsens_temperature_math (uint16_t temperature_output)
{
    return (((temperature_output * 200 * 100) / 2047) - 5000);
}

void init_hpsens (struct hpsens_desc_t *inst,
struct sercom_i2c_desc_t *i2c_inst, uint8_t address, uint32_t period)
{
    inst->i2c_inst = i2c_inst;
    inst->address = address;
    inst->period = period;

    inst->pressure = 0;
    inst->pressurepas = 0;
    inst->temperature = 0;
    inst->last_reading_time = 0;

    hpsens_service(inst);
}

void hpsens_service (struct hpsens_desc_t *inst) 
{
    /** exit service fxn if transaction is underway and not complete*/
    if (inst->i2c_in_progress)
    {
        if (!sercom_i2c_transaction_done(inst->i2c_inst, inst->i2c_transaction_id))
        {
            return;
        }

        enum i2c_transaction_state const s = sercom_i2c_transaction_state(
                                                    inst->i2c_inst,
                                                    inst->i2c_transaction_id);

        sercom_i2c_clear_transaction(inst->i2c_inst, inst->i2c_transaction_id);

        if (s != I2C_STATE_DONE) {
            // Transaction failed
            return;
        }
        
        inst->i2c_in_progress = 0;
        uint16_t pressure_output = inst->sensbuffer[1] | (inst->sensbuffer[0]<< 8);
        uint8_t status = (pressure_output >> 14);
        if (status != 0) 
        {
            return;
        }
        inst->pressure = hpsens_pressure_math(pressure_output);
        inst->pressurepas = 6895 * inst->pressure;
        uint16_t temperature_output = inst->sensbuffer[3] | (inst->sensbuffer[2]<< 8);
        temperature_output >>= 5;
        inst->temperature = hpsens_temperature_math(temperature_output);
    }
    /**exit service fxn if less time than period has passed*/
    else if ((millis - inst->last_reading_time) < inst->period)
    {
        return;
    }
    /**initiate transaction*/
    else if (!sercom_i2c_start_generic(inst->i2c_inst, &inst->i2c_transaction_id, 
                                        inst->address, NULL, 0 , inst->sensbuffer, 4))
    {
        inst->i2c_in_progress = 1;
        inst->last_reading_time = millis;       
        return;
    }
}

    
