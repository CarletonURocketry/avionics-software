/**
 * @file hpsens.h
 * @desc Driver for Honeywell HSCMAND060PA3A3 Pressure sensor
 * @author Darwin Jull  
 * @date 2021-07-14
 * Last Author:
 * Last Edited On:
 */


#ifndef hpsens_h

#define hpsens_h

/** Main I2C address for HSCMAND060PA3A3, 
 see nomenclature guide*/
#define HPSENS_ADDR_MAIN 0b0111000

/** Standard address for Honeywell devices is 40*/
#define HPSENS_ADDR_40 0b0101000

/** Other addresses*/
#define HPSENS_ADDR_56 0b0111000

#define HPSENS_ADDR_72 0b1001000

#define HPSENS_ADDR_88 0b1011000

#define HPSENS_ADDR_104 0b1101000

#define HPSENS_ADDR_120 0b1111000

#define HPSENS_ADDR_136 0b10001000

#define HPSENS_ADDR_152 0b10011000


#include "global.h"

#include "sercom-i2c.h"


struct hpsens_desc_t {

    /** Serial driver instance, sensor address, and transaction ID */
    struct sercom_i2c_desc_t *i2c_inst;
    uint8_t address;
    uint8_t i2c_transaction_id;

    /** Recieved information from sensor */
    int32_t pressure;
    int32_t pressurepas;
    int32_t temperature;
    uint8_t sensbuffer[4];

    /** Other information */
    uint32_t last_reading_time;
    uint32_t period;
    uint8_t i2c_in_progress:1;
};

extern void init_hpsens (struct hpsens_desc_t *inst, struct sercom_i2c_desc_t *i2c_inst,
 uint8_t address, uint32_t period);

extern void hpsens_service (struct hpsens_desc_t *inst); 

static inline int32_t hpsens_get_pressure (struct hpsens_desc_t *inst)
{
    return inst->pressure;
};

static inline int32_t hpsens_get_temperature (struct hpsens_desc_t *inst)
{
    return inst->temperature;
};

static inline uint32_t hpsens_get_last_reading_time (struct hpsens_desc_t *inst)
{
    return inst->last_reading_time;
};

static inline void hpsens_set_period (struct hpsens_desc_t *inst, uint32_t period)
{
    inst->period = period;
};

#endif
