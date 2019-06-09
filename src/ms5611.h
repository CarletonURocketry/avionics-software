/**
 * @file ms5611.h
 * @desc Driver for MS5611 barometric pressure sensor
 * @author Samuel Dewan
 * @date 2019-06-04
 * Last Author:
 * Last Edited On:
 */

#ifndef ms5611_h
#define ms5611_h

#include "global.h"

#include "sercom-i2c.h"

enum ms5611_state {
    MS5611_RESET,
    MS5611_RESET_WAIT,
    MS5611_READ_C1,
    MS5611_READ_C2,
    MS5611_READ_C3,
    MS5611_READ_C4,
    MS5611_READ_C5,
    MS5611_READ_C6,
    MS5611_IDLE,
    MS5611_CONVERT_PRES,
    MS5611_CONVERT_PRES_WAIT,
    MS5611_READ_PRES,
    MS5611_CONVERT_TEMP,
    MS5611_CONVERT_TEMP_WAIT,
    MS5611_READ_TEMP,
    MS5611_FAILED
};

struct ms5611_desc_t {
    /** I2C instance used by this sensor */
    struct sercom_i2c_desc_t *i2c_inst;
    
    /** Time of last reading from sensor */
    uint32_t last_reading_time;
    /** Temperature compensated presure read from sensor */
    int32_t pressure;
    /** Temperature read from sensor */
    int32_t temperature;
    /** Altitude calculated from sensor */
    float altitude;
    
    /** Pressure used as 0 for altitude calculations */
    float p0;
    /** Digital pressure value from ADC  */
    uint32_t d1;
    /** Digital tempuratue value from ADC */
    uint32_t d2;

    /** Conversion start time */
    uint32_t conv_start_time;
    
    /** Time between readings of the sensor */
    uint32_t period;
    
    /** Values read from sensor PROM */
    uint16_t prom_values[6];
    /** I2C address for sensor */
    uint8_t address;
    /** I2C transaction id */
    uint8_t t_id;
    /** Drive current state */
    enum ms5611_state state:4;
    /** Currently waiting for an I2C transaction to complete */
    uint8_t i2c_in_progress:1;
    /** Flag to indicate whether altitude should be calculated */
    uint8_t calc_altitude:1;
    /** Flag to indicate whether p0 has been initialized */
    uint8_t p0_set:1;
};

/**
 * Initialize an instance of the MS5611 driver.
 *
 * @param inst Pointer to the instance descriptor to be initialized
 * @param i2c_inst I2C interface to be used by this driver instance
 * @param csb Non-zero value if CSB pin of sensor is pulled high
 * @param period Period in milliseconds at which the sensor should be polled
 * @param calculate_altitude Whether the altitude value should be calculated
 *                           when the sensor is polled
 */
extern void init_ms5611 (struct ms5611_desc_t *inst,
                         struct sercom_i2c_desc_t *i2c_inst, uint8_t csb,
                         uint32_t period, uint8_t calculate_altitude);




/**
 * Service to be run in each iteration of the main loop.
 *
 * @param inst The MS5611 driver instance for which the service should be run
 */
extern void ms5611_service (struct ms5611_desc_t *inst);

/**
 * Get the most recently measured pressure value.
 *
 * @param inst The MS5611 driver instance
 *
 * @return The most recently measured pressure in hundredths of a mbar
 */
static inline int32_t ms5611_get_pressure (struct ms5611_desc_t *inst)
{
    return inst->pressure;
}

/**
 * Get the most recently measured temperature value.
 *
 * @param inst The MS5611 driver instance
 *
 * @return The most recently measured temperature in hundredths of a degree
 *         celsius
 */
static inline int32_t ms5611_get_temperature (struct ms5611_desc_t *inst)
{
    return inst->temperature;
}

/**
 * Get the most recently measured altitude value.
 *
 * @param inst The MS5611 driver instance
 *
 * @return The most recently measured altitude in meters
 */
static inline float ms5611_get_altitude (struct ms5611_desc_t *inst)
{
    return inst->altitude;
}

/**
 * Get the last time at which a reading was started.
 *
 * @param inst The MS5611 driver instance
 *
 * @return The value of millis when the last reading was started
 */
static inline uint32_t ms5611_get_last_reading_time (struct ms5611_desc_t *inst)
{
    return inst->last_reading_time;
}

/**
 * Set the period at which readings are taken.
 *
 * @param inst The MS5611 driver instance
 * @param period The new period at which readings should be taken in
 *               milliseconds
 */
static inline void ms5611_set_period (struct ms5611_desc_t *inst,
                                      uint32_t period)
{
    inst->period = period;
}

/**
 * Tare altitude calculations by setting the refernce pressure to the last
 * measured pressure.
 *
 * @param inst The MS5611 driver instance
 */
static inline void ms5611_tare_now (struct ms5611_desc_t *inst)
{
    inst->p0 = ((float)inst->pressure) / 100.0;
}

/**
 * Tare altitude calculations next time a measurment takes place by setting the
 * reference pressure to the measured pressure.
 *
 * @param inst The MS5611 driver instance
 */
static inline void ms5611_tare_next (struct ms5611_desc_t *inst)
{
    inst->p0_set = 0;
}

#endif /* ms5611_h */
