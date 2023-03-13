/**
 * @file kx134-1211.h
 * @desc Driver for KX134-1211 accelerometer
 * @author Samuel Dewan
 * @date 2021-07-09
 * Last Author:
 * Last Edited On:
 */

#ifndef kx134_1211_h
#define kx134_1211_h

#include "global.h"

#include "sercom-spi.h"
#include "gpio.h"
#include "src/telemetry/telemetry.h"

#define KX134_1211_BAUDRATE                 10000000UL

#define KX134_1211_SAMPLE_THRESHOLD_8BIT    128
#define KX134_1211_SAMPLE_THRESHOLD_16BIT   64

enum kx134_1211_range {
    /** +/-8 g */
    KX134_1211_RANGE_8G,
    /** +/-16 g */
    KX134_1211_RANGE_16G,
    /** +/-32 g */
    KX134_1211_RANGE_32G,
    /** +/-64 g */
    KX134_1211_RANGE_64G
};

enum kx134_1211_low_pass_rolloff {
    /** IR filter corner frequency set to ODR/9 */
    KX134_1211_LOW_PASS_ROLLOFF_9,
    /** IR filter corner frequency set to ODR/2 */
    KX134_1211_LOW_PASS_ROLLOFF_2,
};

enum kx134_1211_odr {
    /** 0.781 Hz */
    KX134_1211_ODR_781,
    /** 1.563 Hz */
    KX134_1211_ODR_1563,
    /** 3.125 Hz */
    KX134_1211_ODR_3125,
    /** 6.25 Hz */
    KX134_1211_ODR_6250,
    /** 12.5 Hz */
    KX134_1211_ODR_12500,
    /** 25 Hz */
    KX134_1211_ODR_25000,
    /** 50 Hz */
    KX134_1211_ODR_50000,
    /** 100 Hz */
    KX134_1211_ODR_100000,
    /** 200 Hz */
    KX134_1211_ODR_200000,
    /** 400 Hz */
    KX134_1211_ODR_400000,
    /** 800 Hz */
    KX134_1211_ODR_800000,
    /** 1600 Hz */
    KX134_1211_ODR_1600000,
    /** 3200 Hz */
    KX134_1211_ODR_3200000,
    /** 6400 Hz */
    KX134_1211_ODR_6400000,
    /** 12800 Hz */
    KX134_1211_ODR_12800000,
    /** 25600 Hz */
    KX134_1211_ODR_25600000
};

enum kx134_1211_resolution {
    /** 8 bit resolution */
    KX134_1211_RES_8_BIT,
    /** 16 bit resolution */
    KX134_1211_RES_16_BIT
};




enum kx134_1211_state {
    /** Wait for device to boot (typical 20 ms, max 50 ms) then write 0 to
        mysterious register 0x7f */
    KX134_1211_POWER_ON,
    /** Write 0 to CNTL2 */
    KX134_1211_CLEAR_CNTL2,
    /** Write 0x80 to CNTL2 to initiate software reset */
    KX134_1211_SOFTWARE_RESET,
    /** Wait for software reset to complete (minimum 2 ms) then check Who Am I
        register (should be 0x46) */
    KX134_1211_CHECK_WAI,
    /** Check command test response register (should be 0x55) */
    KX134_1211_CHECK_COTR,
    /** Write CNTL1 to enable accelerometer with default settings */
    KX134_1211_ENABLE_ACCEL,
    /** Wait for accelerometer to be ready (min 21.6 ms at ODR = 50, round up to
        50 ms to make sure (since 1/ODR = 20 ms)) then take reading with self
        test off */
    KX134_1211_READ_ST_OFF,
    /** Write CNTL1 to disable accelerometer */
    KX134_1211_DISABLE_ACCEL,
    /** Write 0xCA to SELF_TEST register to enable self test */
    KX134_1211_ENABLE_SELF_TEST,
    ///** Write CNTL1 to enable accelerometer with default settings */
    //KX134_1211_ENABLE_ACCEL,  (reuse previously defined state)
    /** Wait for accelerometer to be ready (min 21.6 ms at ODR = 50, round up to
        50 ms to make sure (since 1/ODR = 20 ms)) then take reading with self
        test on */
    KX134_1211_READ_ST_ON,
    ///** Write CNTL1 to disable accelerometer */
    //KX134_1211_DISABLE_ACCEL, (reuse previously defined state)
    /** Write SELF_TEST, BUF_CNTL1 and BUF_CNTL2 to disable self test and
        configure buffer in stream mode */
    KX134_1211_CONFIG_BUFFER,
    /** Write CNTL2 through CNTL6, ODCNTL and INC1 though INC6 to configure
        sensor */
    KX134_1211_CONFIG,
    ///** Write CNTL1 to enable accelerometer */
    //KX134_1211_ENABLE_ACCEL,  (reuse previously defined state)
    /** Reading data is handled by callbacks */
    KX134_1211_RUNNING,
    /** Driver failed */
    KX134_1211_FAILED,
    /** WAI not recognized */
    KX134_1211_FAILED_WAI,
    /** Check command test response invalid */
    KX134_1211_FAILED_COTR,
    /** Self test failed */
    KX134_1211_FAILED_SELF_TEST
};

struct kx134_1211_desc_t {
    /** SPI instance used by this sensor */
    struct sercom_spi_desc_t *spi_inst;

    /** Telemetry service instance */
    struct telemetry_service_desc_t *telem;

    /** Mask for SPI chip select pin */
    uint32_t cs_pin_mask;

    /** Buffer for commands and data from sensor */
    uint8_t buffer[480];

    uint8_t *telem_buffer;

    /** Time of last reading from sensor */
    uint32_t last_reading_time;

    union {
        /** Temporary storage for time of sensor read before we have actually
            read the data */
        uint32_t next_reading_time;
        /** Time used for delays during initialization */
        uint32_t init_delay_start_time;
    };
    /** X acceleration from last sensor reading */
    int16_t last_x;
    /** Y acceleration from last sensor reading */
    int16_t last_y;
    /** Z acceleration from last sensor reading */
    int16_t last_z;

    /** Sensitivity of accelerometer with current settings in LSB/g */
    uint16_t sensitivity;

    /** SPI transaction id */
    uint8_t t_id;

    /** Group for SPI chip select pin */
    uint8_t cs_pin_group;

    /** Driver current state */
    enum kx134_1211_state state:5;
    /** Next state for enable and disable states */
    enum kx134_1211_state en_next_state:5;

    /** Range setting */
    enum kx134_1211_range range:2;
    /** Low pass filter roll-off setting */
    enum kx134_1211_low_pass_rolloff rolloff:1;
    /** Output data rate */
    enum kx134_1211_odr odr:4;
    /** Resolution */
    enum kx134_1211_resolution resolution:1;

    /** Flag to indicate that the delay for the current state is complete */
    uint8_t delay_done:1;
    /** Flag to indicate that we the SPI command to be sent in the current state
        has been marshaled */
    uint8_t cmd_ready:1;
    /** Flag to indicate that we are waiting for an SPI transaction */
    uint8_t spi_in_progress:1;
    /** Flag to indicate that we currently are writing to a telemetry buffer */
    uint8_t telem_buffer_write:1;
};

/**
 * Initialize an instance of the KX134-1211 driver.
 *
 * @param inst Pointer to the instance descriptor to be initialized
 * @param spi_inst SPI interface to be used by this driver instance
 * @param int1_pin Pin for interrupt 1
 * @param int2_pin Pin for interrupt 2 (can be PIN_NONE)
 * @param range G-range setting
 * @param rolloff Low pass filter roll-off setting
 * @param odr Output data rate
 * @param resolution Resolution setting
 */
extern void init_kx134_1211 (struct kx134_1211_desc_t *inst,
                             struct sercom_spi_desc_t *spi_inst,
                             uint8_t cs_pin_group, uint32_t cs_pin_mask,
                             union gpio_pin_t int1_pin,
                             union gpio_pin_t int2_pin,
                             enum kx134_1211_range range,
                             enum kx134_1211_low_pass_rolloff rolloff,
                             enum kx134_1211_odr odr,
                             enum kx134_1211_resolution resolution);

static inline void kx134_1211_register_telem(struct kx134_1211_desc_t *inst,
                                        struct telemetry_service_desc_t *telem)
{
    inst->telem = telem;
}

/**
 * Service to be run in each iteration of the main loop.
 *
 * @param inst The MS5611 driver instance for which the service should be run
 */
extern void kx134_1211_service (struct kx134_1211_desc_t *inst);

/**
 * Get the time of the most recent measurement.
 *
 * @param inst The KX134-1211 driver instance
 *
 * @return The value of millis for the most recent measurement
 */
static inline uint32_t kx134_1211_get_last_time (struct kx134_1211_desc_t *inst)
{
    return inst->last_reading_time;
}

/**
 * Get the X axis acceleration value from the most recent measurement.
 *
 * @param inst The KX134-1211 driver instance
 *
 * @return The most recent X axis acceleration measurement
 */
static inline int16_t kx134_1211_get_last_x (struct kx134_1211_desc_t *inst)
{
    return inst->last_x;
}

/**
 * Get the Y axis acceleration value from the most recent measurement.
 *
 * @param inst The KX134-1211 driver instance
 *
 * @return The most recent Y axis acceleration measurement
 */
static inline int16_t kx134_1211_get_last_y (struct kx134_1211_desc_t *inst)
{
    return inst->last_y;
}

/**
 * Get the Z axis acceleration value from the most recent measurement.
 *
 * @param inst The KX134-1211 driver instance
 *
 * @return The most recent Z axis acceleration measurement
 */
static inline int16_t kx134_1211_get_last_z (struct kx134_1211_desc_t *inst)
{
    return inst->last_z;
}

/**
 *  Get the sensitivity of the accelerometer with it's current settings.
 *
 *  @param inst The KX134-1211 driver instance
 *
 *  @return The accelerometer sensitivity in LSB/g
 */
static inline uint16_t kx134_1211_get_sensitivity(
                                                struct kx134_1211_desc_t *inst)
{
    return inst->sensitivity;
}




//
//  Telemetry related functions defined in telemetry.c
//

/**
 *  Post data from KX134 accelerometer to telemetry service.
 *
 *  @param inst Telemetry service instance
 *  @param time Mission time for data being posted
 *  @param odr Output data rate
 *  @param range Acceleration range
 *  @param roll Low-pass filter rolloff
 *  @param res Resolution
 *  @param sensor_payload_length Amount of sensor data to be posted in bytes
 *
 *  @return Pointer to a buffer into which sensor data can be copied or NULL if
 *          a buffer cannot be provided
 */
extern uint8_t *telemetry_post_kx134_accel(
                                        struct telemetry_service_desc_t *inst,
                                        uint32_t time, enum kx134_1211_odr odr,
                                        enum kx134_1211_range range,
                                        enum kx134_1211_low_pass_rolloff roll,
                                        enum kx134_1211_resolution res,
                                        uint16_t sensor_payload_length);

/**
 *  Indicate to telemetry service that KX134 acceleration data has been copied
 *  into buffer received from telemetry_post_kx134_accel().
 *
 *  @param inst Telemetry service instance
 *  @param buffer The buffer into which data has been copied.
 */
extern int telemetry_finish_kx134_accel(struct telemetry_service_desc_t *inst,
                                        uint8_t *buffer);

#endif /* kx134_1211_h */
