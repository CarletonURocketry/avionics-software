/**
 * @file mpu9250-registers.h
 * @desc Register definitions for MPU9250 IMU
 * @author Samuel Dewan
 * @date 2021-09-29
 * Last Author:
 * Last Edited On:
 */

#ifndef mpu9250_registers_h
#define mpu9250_registers_h

// MSB of register address specifies read or write for SPI interface
#define MPU9250_READ        (1U << 7)
#define MPU9250_WRITE       (0U << 7)

#define MPU9250_I2C_ADDR    0b1101000U


#define MPU9250_TEMP_ROOM_OFFSET    0
/** Temperature sensor sensitivity in degrees celsius per LSB */
#define MPU9250_TEMP_SENSITIVITY    321



//
//  0x00 - SELF_TEST_X_GYRO
//  Gyroscope X-axis self test output generated during manufacturing tests.
//
#define MPU9250_REG_ST_X_GYRO               0x00

//
//  0x01 - SELF_TEST_Y_GYRO
//  Gyroscope Y-axis self test output generated during manufacturing tests.
//
#define MPU9250_REG_ST_Y_GYRO               0x01

//
//  0x02 - SELF_TEST_Z_GYRO
//  Gyroscope Z-axis self test output generated during manufacturing tests.
//
#define MPU9250_REG_ST_Z_GYRO               0x02


//
//  0x0D - SELF_TEST_X_ACCEL
//  Accelerometer X-axis self test output generated during manufacturing tests.
//
#define MPU9250_REG_ST_X_ACCEL              0x0D

//
//  0x0E - SELF_TEST_Y_ACCEL
//  Accelerometer Y-axis self test output generated during manufacturing tests.
//
#define MPU9250_REG_ST_Y_ACCEL              0x0E

//
//  0x0F - SELF_TEST_Z_ACCEL
//  Accelerometer Z-axis self test output generated during manufacturing tests.
//
#define MPU9250_REG_ST_Z_ACCEL              0x0F




//
//  0x13 - XG_OFFSET_H
//  Gyro Offset - X High
//
#define MPU9250_REG_XG_OFFSET_H             0x13

//
//  0x14 - XG_OFFSET_L
//  Gyro Offset - X Low
//
#define MPU9250_REG_XG_OFFSET_L             0x14


//
//  0x15 - YG_OFFSET_H
//  Gyro Offset - Y High
//
#define MPU9250_REG_YG_OFFSET_H             0x15

//
//  0x16 - YG_OFFSET_L
//  Gyro Offset - Y Low
//
#define MPU9250_REG_YG_OFFSET_L             0x16


//
//  0x17 - ZG_OFFSET_H
//  Gyro Offset - Z High
//
#define MPU9250_REG_ZG_OFFSET_H             0x17

//
//  0x18 - ZG_OFFSET_L
//  Gyro Offset - Z Low
//
#define MPU9250_REG_ZG_OFFSET_L             0x18




//
//  0x19 - SMPLRT_DIV
//  Divides internal sample rate, SR = Internal_Sample_Rate / (1 + SMPLRT_DIV)
//
#define MPU9250_REG_SMPLRT_DIV              0x19


//
//  0x1A - CONFIG
//  Configuration.
//
#define MPU9250_REG_CONFIG                  0x1A

// DLPF_CFG: Digital Low Pass Filter Config
#define MPU9250_CONFIG_DLPF_CFG_Pos     0
#define MPU9250_CONFIG_DLPF_CFG_Msk     (0x7U << MPU9250_CONFIG_DLPF_CFG_Pos)
#define MPU9250_CONFIG_DLPF_CFG(x)      (((x) << MPU9250_CONFIG_DLPF_CFG_Pos) \
                                            & MPU9250_CONFIG_DLPF_CFG_Msk)
// EXT_SYNC_SET: Enables the FSYNC pin data to be sampled (???)
#define MPU9250_CONFIG_EXT_SYNC_SET_Pos 3
#define MPU9250_CONFIG_EXT_SYNC_SET_Msk (0x7U << \
                                            MPU9250_CONFIG_EXT_SYNC_SET_Pos)
#define MPU9250_CONFIG_EXT_SYNC_SET(x)  (((x) << \
                                            MPU9250_CONFIG_EXT_SYNC_SET_Pos) & \
                                         MPU9250_CONFIG_EXT_SYNC_SET_Msk)
// FIFO_MODE: Selects whether FIFO overwrites old data when full
#define MPU9250_CONFIG_FIFO_MODE_Pos    6
#define MPU9250_CONFIG_FIFO_MODE_Msk    (1U << MPU9250_CONFIG_FIFO_MODE_Pos)
#define MPU9250_CONFIG_FIFO_MODE_OVERWRITE      (0U << \
                                                 MPU9250_CONFIG_FIFO_MODE_Pos)
#define MPU9250_CONFIG_FIFO_MODE_NO_OVERWRITE   (1U << \
                                                 MPU9250_CONFIG_FIFO_MODE_Pos)


//
//  0x1B - GYRO_CONFIG
//  Gyroscope Configuration.
//
#define MPU9250_REG_GYRO_CONFIG             0x1B

// FCHOICE_B: Inverted gyroscope Fchoice, which is used to bypass the DLPF
#define MPU9250_GYRO_CONFIG_FCHOICE_B_Pos   0
#define MPU9250_GYRO_CONFIG_FCHOICE_B_Msk   (0x3U << \
                                             MPU9250_GYRO_CONFIG_FCHOICE_B_Pos)
#define MPU9250_GYRO_CONFIG_FCHOICE_B(x)    (((x) << \
                                            MPU9250_GYRO_CONFIG_FCHOICE_B_Pos) \
                                            & MPU9250_GYRO_CONFIG_FCHOICE_B_Msk)
// GYRO_FS_SEL: Select gyroscope full scale range
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_Pos 3
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_Msk (0x3U << \
                                            MPU9250_GYRO_CONFIG_GYRO_FS_SEL_Pos)
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL(x)  (((x) << \
                                        MPU9250_GYRO_CONFIG_GYRO_FS_SEL_Pos) & \
                                        MPU9250_GYRO_CONFIG_GYRO_FS_SEL_Msk)
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_250_Val 0b00U
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_250 MPU9250_GYRO_CONFIG_GYRO_FS_SEL(\
                                        MPU9250_GYRO_CONFIG_GYRO_FS_SEL_250_Val)
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_500_Val 0b01U
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_500 MPU9250_GYRO_CONFIG_GYRO_FS_SEL(\
                                        MPU9250_GYRO_CONFIG_GYRO_FS_SEL_500_Val)
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_1000_Val    0b10U
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_1000 MPU9250_GYRO_CONFIG_GYRO_FS_SEL(\
                                    MPU9250_GYRO_CONFIG_GYRO_FS_SEL_1000_Val)
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_2000_Val    0b11U
#define MPU9250_GYRO_CONFIG_GYRO_FS_SEL_2000 MPU9250_GYRO_CONFIG_GYRO_FS_SEL(\
                                    MPU9250_GYRO_CONFIG_GYRO_FS_SEL_2000_Val)
// ZGYRO_Cten: Enable self test for gyroscope Z axis
#define MPU9250_GYRO_CONFIG_ZGYRO_CTEN_Pos  5
#define MPU9250_GYRO_CONFIG_ZGYRO_CTEN      (1U << \
                                             MPU9250_GYRO_CONFIG_ZGYRO_CTEN_Pos)
// YGYRO_Cten: Enable self test for gyroscope Y axis
#define MPU9250_GYRO_CONFIG_YGYRO_CTEN_Pos  6
#define MPU9250_GYRO_CONFIG_YGYRO_CTEN      (1U << \
                                             MPU9250_GYRO_CONFIG_YGYRO_CTEN_Pos)
// XGYRO_Cten: Enable self test for gyroscope X axis
#define MPU9250_GYRO_CONFIG_XGYRO_CTEN_Pos  7
#define MPU9250_GYRO_CONFIG_XGYRO_CTEN      (1U << \
                                             MPU9250_GYRO_CONFIG_XGYRO_CTEN_Pos)


//
//  0x1C - ACCEL_CONFIG
//  Accelerometer Configuration.
//
#define MPU9250_REG_ACCEL_CONFIG            0x1C

// ACCEL_HPF: Configure digital high pass filter (DHPF)
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_Pos      0
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_Msk      (0x7U << \
                                            MPU9250_ACCEL_CONFIG_ACCEL_HPF_Pos)
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF(x)       (((x) << \
                                    MPU9250_ACCEL_CONFIG_ACCEL_HPF_Pos) & \
                                    MPU9250_ACCEL_CONFIG_ACCEL_HPF_Msk)
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_RESET_Val    0U
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_RESET    MPU9250_ACCEL_CONFIG_ACCEL_HPF(\
                                    MPU9250_ACCEL_CONFIG_ACCEL_HPF_RESET_Val)
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_500_Val      1U
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_500      MPU9250_ACCEL_CONFIG_ACCEL_HPF(\
                                    MPU9250_ACCEL_CONFIG_ACCEL_HPF_500_Val)
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_250_Val      2U
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_250      MPU9250_ACCEL_CONFIG_ACCEL_HPF(\
                                    MPU9250_ACCEL_CONFIG_ACCEL_HPF_250_Val)
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_125_Val      3U
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_125      MPU9250_ACCEL_CONFIG_ACCEL_HPF(\
                                    MPU9250_ACCEL_CONFIG_ACCEL_HPF_125_Val)
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_063_Val      4U
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_063      MPU9250_ACCEL_CONFIG_ACCEL_HPF(\
                                    MPU9250_ACCEL_CONFIG_ACCEL_HPF_063_Val)
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_HOLD_Val     7U
#define MPU9250_ACCEL_CONFIG_ACCEL_HPF_HOLD     MPU9250_ACCEL_CONFIG_ACCEL_HPF(\
                                    MPU9250_ACCEL_CONFIG_ACCEL_HPF_HOLD_Val)
// ACCEL_FS_SEL: Select accelerometer full scale range
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_Pos   3
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_Msk   (0x3U << \
                                        MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_Pos)
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL(x)  (((x) << \
                                    MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_Pos) & \
                                    MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_Msk)
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_2_Val     0b00U
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_2 MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL(\
                                        MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_2_Val)
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_4_Val     0b01U
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_4 MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL(\
                                        MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_4_Val)
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_8_Val     0b10U
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_8 MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL(\
                                        MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_8_Val)
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_16_Val     0b11U
#define MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_16 MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL(\
                                    MPU9250_ACCEL_CONFIG_ACCEL_FS_SEL_16_Val)
// AZ_ST_EN: Enable self test for accelerometer Z axis
#define MPU9250_ACCEL_CONFIG_AZ_ST_EN_Pos       5
#define MPU9250_ACCEL_CONFIG_AZ_ST_EN  (1U << MPU9250_ACCEL_CONFIG_AZ_ST_EN_Pos)
// AY_ST_EN: Enable self test for accelerometer Y axis
#define MPU9250_ACCEL_CONFIG_AY_ST_EN_Pos       6
#define MPU9250_ACCEL_CONFIG_AY_ST_EN  (1U << MPU9250_ACCEL_CONFIG_AY_ST_EN_Pos)
// AX_ST_EN: Enable self test for accelerometer X axis
#define MPU9250_ACCEL_CONFIG_AX_ST_EN_Pos       7
#define MPU9250_ACCEL_CONFIG_AX_ST_EN  (1U << MPU9250_ACCEL_CONFIG_AX_ST_EN_Pos)


//
//  0x1D - ACCEL_CONFIG_2
//  Accelerometer Configuration 2.
//
#define MPU9250_REG_ACCEL_CONFIG_2          0x1D

// A_DLPFCFG: Accelerometer low pass filter setting
#define MPU9250_ACCEL_CONFIG_2_A_DLPFCFG_Pos    0
#define MPU9250_ACCEL_CONFIG_2_A_DLPFCFG_Msk    (0x7U << \
                                        MPU9250_ACCEL_CONFIG_2_A_DLPFCFG_Pos)
#define MPU9250_ACCEL_CONFIG_2_A_DLPFCFG(x) (((x) << \
                                        MPU9250_ACCEL_CONFIG_2_A_DLPFCFG_Pos) \
                                        & MPU9250_ACCEL_CONFIG_2_A_DLPFCFG_Msk)
// ACCEL_FCHOICE_B: Inverted accelerometer Fchoice
#define MPU9250_ACCEL_CONFIG_2_ACCEL_FCHOICE_B_Pos  3
#define MPU9250_ACCEL_CONFIG_2_ACCEL_FCHOICE_B      (1U << \
                                    MPU9250_ACCEL_CONFIG_2_ACCEL_FCHOICE_B_Pos)


//
//  0x1E - LP_ACCEL_ODR
//  Low Power Accelerometer ODR Control.
//
#define MPU9250_REG_LP_ACCEL_ODR            0x1E

// LPOSC_CLKSEL: Lower power accelerometer mode output data rate
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_Pos 0
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_Msk (0xfU << \
                                        MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_Pos)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(x)  (((x) << \
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_Pos) & \
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_Msk)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_24_Val    0U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_24 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_24_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_49_Val    1U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_49 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_49_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_98_Val    2U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_98 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_98_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_195_Val   3U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_195 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_195_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_391_Val   4U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_391 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_391_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_781_Val   5U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_781 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_781_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_1563_Val  6U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_1563 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_1563_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_3125_Val  7U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_3125 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_3125_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_6250_Val  8U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_6250 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_6250_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_12500_Val 9U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_12500 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_12500_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_25000_Val 10U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_25000 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_25000_Val)
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_50000_Val 11U
#define MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_50000 MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL(\
                                    MPU9250_LP_ACCEL_ODR_LPOSC_CLKSEL_50000_Val)


//
//  0x1F - WOM_THR
//  Wake-on Motion Threshold, LSB = 4mg.
//
#define MPU9250_REG_WOM_THR                 0x1F




//
//  0x23 - FIFO_EN
//  FIFO Enable.
//
#define MPU9250_REG_FIFO_EN                 0x23

// SLV_0: Write external sensor data from I2C slave 0 to FIFO at sample rate
#define MPU9250_FIFO_EN_SLV_0_Pos       0
#define MPU9250_FIFO_EN_SLV_0           (1U << MPU9250_FIFO_EN_SLV_0_Pos)
// SLV_1: Write external sensor data from I2C slave 1 to FIFO at sample rate
#define MPU9250_FIFO_EN_SLV_1_Pos       1
#define MPU9250_FIFO_EN_SLV_1           (1U << MPU9250_FIFO_EN_SLV_1_Pos)
// SLV_2: Write external sensor data from I2C slave 2 to FIFO at sample rate
#define MPU9250_FIFO_EN_SLV_2_Pos       2
#define MPU9250_FIFO_EN_SLV_2           (1U << MPU9250_FIFO_EN_SLV_2_Pos)
// ACCEL: Write acceleration data to FIFO at sample rate
#define MPU9250_FIFO_EN_ACCEL_Pos       3
#define MPU9250_FIFO_EN_ACCEL           (1U << MPU9250_FIFO_EN_ACCEL_Pos)
// GYRO_ZOUT: Write Z axis gyroscope data to FIFO at sample rate
#define MPU9250_FIFO_EN_GYRO_ZOUT_Pos   4
#define MPU9250_FIFO_EN_GYRO_ZOUT       (1U << MPU9250_FIFO_EN_GYRO_ZOUT_Pos)
// GYRO_YOUT: Write Y axis gyroscope data to FIFO at sample rate
#define MPU9250_FIFO_EN_GYRO_YOUT_Pos   5
#define MPU9250_FIFO_EN_GYRO_YOUT       (1U << MPU9250_FIFO_EN_GYRO_YOUT_Pos)
// GYRO_XOUT: Write X axis gyroscope data to FIFO at sample rate
#define MPU9250_FIFO_EN_GYRO_XOUT_Pos   6
#define MPU9250_FIFO_EN_GYRO_XOUT       (1U << MPU9250_FIFO_EN_GYRO_XOUT_Pos)
// TEMP_OUT: Write temperature data to FIFO at sample rate
#define MPU9250_FIFO_EN_TEMP_OUT_Pos    7
#define MPU9250_FIFO_EN_TEMP_OUT        (1U << MPU9250_FIFO_EN_TEMP_OUT_Pos)




//
//  0x24 - I2C_MST_CTRL
//  I2C Master Control.
//
#define MPU9250_REG_I2C_MST_CTRL            0x24

// I2C_MST_CLK: Configure I2C master clock divider
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_Pos    0
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_Msk    (0xfU << \
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_Pos)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK(x)     (((x) << \
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_Pos) & \
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_Msk)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_348_Val    0U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_348    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_348_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_333_Val    1U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_333    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_333_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_320_Val    2U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_320    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_320_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_308_Val    3U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_308    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_308_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_296_Val    4U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_296    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_296_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_286_Val    5U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_286    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_286_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_276_Val    6U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_276    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_276_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_267_Val    7U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_267    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_267_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_258_Val    8U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_258    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_258_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_500_Val    9U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_500    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_500_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_471_Val    10U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_471    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_471_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_444_Val    11U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_444    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_444_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_421_Val    12U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_421    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_421_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_400_Val    13U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_400    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_400_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_381_Val    14U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_381    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_381_Val)
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_364_Val    15U
#define MPU9250_I2C_MST_CTRL_I2C_MST_CLK_364    MPU9250_I2C_MST_CTRL_I2C_MST_CLK(\
                                    MPU9250_I2C_MST_CTRL_I2C_MST_CLK_364_Val)
// I2C_MST_P_NSR: Whether repeated start or a stop is used between reads
#define MPU9250_I2C_MST_CTRL_I2C_MST_P_NSR_Pos  4
#define MPU9250_I2C_MST_CTRL_I2C_MST_P_NSR_REPSTART (0U << \
                                        MPU9250_I2C_MST_CTRL_I2C_MST_P_NSR_Pos)
#define MPU9250_I2C_MST_CTRL_I2C_MST_P_NSR_STOP     (1U << \
                                        MPU9250_I2C_MST_CTRL_I2C_MST_P_NSR_Pos)
// SLV_3_FIFO_EN: Write sensor data from I2C slave 3 to FIFO at sample rate
#define MPU9250_I2C_MST_CTRL_SLV_3_FIFO_EN_Pos  5
#define MPU9250_I2C_MST_CTRL_SLV_3_FIFO_EN      (1U << \
                                        MPU9250_I2C_MST_CTRL_SLV_3_FIFO_EN_Pos)
// WAIT_FOR_ES: Delay data ready interrupt until external sensor data is read
#define MPU9250_I2C_MST_CTRL_WAIT_FOR_ES_Pos    6
#define MPU9250_I2C_MST_CTRL_WAIT_FOR_ES        (1U << \
                                        MPU9250_I2C_MST_CTRL_WAIT_FOR_ES_Pos)
// MULT_MST_EN: Enable multi-master support
#define MPU9250_I2C_MST_CTRL_MULT_MST_EN_Pos    7
#define MPU9250_I2C_MST_CTRL_MULT_MST_EN        (1U << \
                                        MPU9250_I2C_MST_CTRL_MULT_MST_EN_Pos)


//
//  0x25 - I2C_SLV0_ADDR
//  I2C Slave 0 Control - Address of I2C slave 0.
//
#define MPU9250_REG_I2C_SLV0_ADDR           0x25

// I2C_ID_0: Physical address of I2C slave
#define MPU9250_I2C_SLV0_ADDR_I2C_ID_0_Pos  0
#define MPU9250_I2C_SLV0_ADDR_I2C_ID_0_Msk  (0x7fU << \
                                            MPU9250_I2C_SLV0_ADDR_I2C_ID_0_Pos)
#define MPU9250_I2C_SLV0_ADDR_I2C_ID_0(x)   (((x) << \
                                        MPU9250_I2C_SLV0_ADDR_I2C_ID_0_Pos) & \
                                        MPU9250_I2C_SLV0_ADDR_I2C_ID_0_Msk)
// I2C_SLV0_RNW: Selects whether transfer is a read or write
#define MPU9250_I2C_SLV0_ADDR_I2C_SLV0_RNW_Pos  7
#define MPU9250_I2C_SLV0_ADDR_I2C_SLV0_RNW_WRITE    (0U << \
                                        MPU9250_I2C_SLV0_ADDR_I2C_SLV0_RNW_Pos)
#define MPU9250_I2C_SLV0_ADDR_I2C_SLV0_RNW_READ     (1U << \
                                        MPU9250_I2C_SLV0_ADDR_I2C_SLV0_RNW_Pos)


//
//  0x26 - I2C_SLV0_REG
//  I2C Slave 0 Control - Register address from which to begin data transfer.
//
#define MPU9250_REG_I2C_SLV0_REG            0x26


//
//  0x27 - I2C_SLV0_CTRL
//  I2C Slave 0 Control - Control.
//
#define MPU9250_REG_I2C_SLV0_CTRL           0x27

// I2C_SLV0_LENG: Number of bytes to be read
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_LENG_Pos 0
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_LENG_Msk (0xfU << \
                                        MPU9250_I2C_SLV0_CTRL_I2C_SLV0_LENG_Pos)
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_LENG(x)   (((x) << \
                                    MPU9250_I2C_SLV0_CTRL_I2C_SLV0_LENG_Pos) & \
                                    MPU9250_I2C_SLV0_CTRL_I2C_SLV0_LENG_Msk)
// I2C_SLV0_GRP: Indicates whether group registers are at even or off addresses
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_GRP_Pos  4
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_GRP_ODD  (0U << \
                                        MPU9250_I2C_SLV0_CTRL_I2C_SLV0_GRP_Pos)
#define MPU9250_I2C_SLV0_ADDR_I2C_SLV0_GRP_EVEN (1U << \
                                        MPU9250_I2C_SLV0_CTRL_I2C_SLV0_GRP_Pos)
// I2C_SLV0_REG_DIS: Disable writing of register address
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_REG_DIS_Pos  5
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_REG_DIS      (1U << \
                                    MPU9250_I2C_SLV0_CTRL_I2C_SLV0_REG_DIS_Pos)
// I2C_SLV0_BYTE_SW: Swap bytes in grouped registers
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_BYTE_SW_Pos  6
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_BYTE_SW      (1U << \
                                    MPU9250_I2C_SLV0_CTRL_I2C_SLV0_BYTE_SW_Pos)
// I2C_SLV0_EN: Enable reading from I2C slave
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_EN_Pos       7
#define MPU9250_I2C_SLV0_CTRL_I2C_SLV0_EN           (1U << \
                                        MPU9250_I2C_SLV0_CTRL_I2C_SLV0_EN_Pos)


//
//  0x28 - I2C_SLV1_ADDR
//  I2C Slave 1 Control - Address of I2C slave 1.
//
#define MPU9250_REG_I2C_SLV1_ADDR           0x28

// I2C_ID_1: Physical address of I2C slave
#define MPU9250_I2C_SLV1_ADDR_I2C_ID_1_Pos  0
#define MPU9250_I2C_SLV1_ADDR_I2C_ID_1_Msk  (0x7fU << \
                                            MPU9250_I2C_SLV1_ADDR_I2C_ID_1_Pos)
#define MPU9250_I2C_SLV1_ADDR_I2C_ID_1(x)   (((x) << \
                                        MPU9250_I2C_SLV1_ADDR_I2C_ID_1_Pos) & \
                                        MPU9250_I2C_SLV1_ADDR_I2C_ID_1_Msk)
// I2C_SLV1_RNW: Selects whether transfer is a read or write
#define MPU9250_I2C_SLV1_ADDR_I2C_SLV1_RNW_Pos  7
#define MPU9250_I2C_SLV1_ADDR_I2C_SLV1_RNW_WRITE    (0U << \
                                        MPU9250_I2C_SLV1_ADDR_I2C_SLV1_RNW_Pos)
#define MPU9250_I2C_SLV1_ADDR_I2C_SLV1_RNW_READ     (1U << \
                                        MPU9250_I2C_SLV1_ADDR_I2C_SLV1_RNW_Pos)


//
//  0x29 - I2C_SLV1_REG
//  I2C Slave 1 Control - Register address from which to begin data transfer.
//
#define MPU9250_REG_I2C_SLV1_REG            0x29


//
//  0x2A - I2C_SLV1_CTRL
//  I2C Slave 1 Control - Control.
//
#define MPU9250_REG_I2C_SLV1_CTRL           0x2A

// I2C_SLV1_LENG: Number of bytes to be read
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_LENG_Pos  0
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_LENG_Msk  (0xfU << \
                                        MPU9250_I2C_SLV1_CTRL_I2C_SLV1_LENG_Pos)
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_LENG(x)   (((x) << \
                                    MPU9250_I2C_SLV1_CTRL_I2C_SLV1_LENG_Pos) & \
                                    MPU9250_I2C_SLV1_CTRL_I2C_SLV1_LENG_Msk)
// I2C_SLV1_GRP: Indicates whether group registers are at even or off addresses
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_GRP_Pos  4
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_GRP_ODD  (0U << \
                                        MPU9250_I2C_SLV1_CTRL_I2C_SLV1_GRP_Pos)
#define MPU9250_I2C_SLV1_ADDR_I2C_SLV1_GRP_EVEN (1U << \
                                        MPU9250_I2C_SLV1_CTRL_I2C_SLV1_GRP_Pos)
// I2C_SLV1_REG_DIS: Disable writing of register address
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_REG_DIS_Pos  5
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_REG_DIS      (1U << \
                                    MPU9250_I2C_SLV1_CTRL_I2C_SLV1_REG_DIS_Pos)
// I2C_SLV1_BYTE_SW: Swap bytes in grouped registers
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_BYTE_SW_Pos  6
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_BYTE_SW      (1U << \
                                    MPU9250_I2C_SLV1_CTRL_I2C_SLV1_BYTE_SW_Pos)
// I2C_SLV1_EN: Enable reading from I2C slave
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_EN_Pos       7
#define MPU9250_I2C_SLV1_CTRL_I2C_SLV1_EN           (1U << \
                                        MPU9250_I2C_SLV1_CTRL_I2C_SLV1_EN_Pos)


//
//  0x2B - I2C_SLV2_ADDR
//  I2C Slave 2 Control - Address of I2C slave 2.
//
#define MPU9250_REG_I2C_SLV2_ADDR           0x2B

// I2C_ID_2: Physical address of I2C slave
#define MPU9250_I2C_SLV2_ADDR_I2C_ID_2_Pos  0
#define MPU9250_I2C_SLV2_ADDR_I2C_ID_2_Msk  (0x7fU << \
                                            MPU9250_I2C_SLV2_ADDR_I2C_ID_2_Pos)
#define MPU9250_I2C_SLV2_ADDR_I2C_ID_2(x)   (((x) << \
                                        MPU9250_I2C_SLV2_ADDR_I2C_ID_2_Pos) & \
                                        MPU9250_I2C_SLV2_ADDR_I2C_ID_2_Msk)
// I2C_SLV2_RNW: Selects whether transfer is a read or write
#define MPU9250_I2C_SLV2_ADDR_I2C_SLV2_RNW_Pos  7
#define MPU9250_I2C_SLV2_ADDR_I2C_SLV2_RNW_WRITE    (0U << \
                                        MPU9250_I2C_SLV2_ADDR_I2C_SLV2_RNW_Pos)
#define MPU9250_I2C_SLV2_ADDR_I2C_SLV2_RNW_READ     (1U << \
                                        MPU9250_I2C_SLV2_ADDR_I2C_SLV2_RNW_Pos)


//
//  0x2C - I2C_SLV2_REG
//  I2C Slave 2 Control - Register address from which to begin data transfer.
//
#define MPU9250_REG_I2C_SLV2_REG            0x2C


//
//  0x2D - I2C_SLV2_CTRL
//  I2C Slave 2 Control - Control.
//
#define MPU9250_REG_I2C_SLV2_CTRL           0x2D

// I2C_SLV2_LENG: Number of bytes to be read
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_LENG_Pos  0
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_LENG_Msk  (0xfU << \
                                        MPU9250_I2C_SLV2_CTRL_I2C_SLV2_LENG_Pos)
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_LENG(x)   (((x) << \
                                    MPU9250_I2C_SLV2_CTRL_I2C_SLV2_LENG_Pos) & \
                                    MPU9250_I2C_SLV2_CTRL_I2C_SLV2_LENG_Msk)
// I2C_SLV2_GRP: Indicates whether group registers are at even or off addresses
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_GRP_Pos  4
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_GRP_ODD  (0U << \
                                        MPU9250_I2C_SLV2_CTRL_I2C_SLV2_GRP_Pos)
#define MPU9250_I2C_SLV2_ADDR_I2C_SLV2_GRP_EVEN (1U << \
                                        MPU9250_I2C_SLV2_CTRL_I2C_SLV2_GRP_Pos)
// I2C_SLV2_REG_DIS: Disable writing of register address
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_REG_DIS_Pos  5
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_REG_DIS      (1U << \
                                    MPU9250_I2C_SLV2_CTRL_I2C_SLV2_REG_DIS_Pos)
// I2C_SLV2_BYTE_SW: Swap bytes in grouped registers
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_BYTE_SW_Pos  6
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_BYTE_SW      (1U << \
                                    MPU9250_I2C_SLV2_CTRL_I2C_SLV2_BYTE_SW_Pos)
// I2C_SLV2_EN: Enable reading from I2C slave
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_EN_Pos       7
#define MPU9250_I2C_SLV2_CTRL_I2C_SLV2_EN           (1U << \
                                        MPU9250_I2C_SLV2_CTRL_I2C_SLV2_EN_Pos)


//
//  0x2E - I2C_SLV3_ADDR
//  I2C Slave 3 Control - Address of I2C slave 3.
//
#define MPU9250_REG_I2C_SLV3_ADDR           0x2E

// I2C_ID_3: Physical address of I2C slave
#define MPU9250_I2C_SLV3_ADDR_I2C_ID_3_Pos  0
#define MPU9250_I2C_SLV3_ADDR_I2C_ID_3_Msk  (0x7fU << \
                                            MPU9250_I2C_SLV3_ADDR_I2C_ID_3_Pos)
#define MPU9250_I2C_SLV3_ADDR_I2C_ID_3(x)   (((x) << \
                                        MPU9250_I2C_SLV3_ADDR_I2C_ID_3_Pos) & \
                                        MPU9250_I2C_SLV3_ADDR_I2C_ID_3_Msk)
// I2C_SLV3_RNW: Selects whether transfer is a read or write
#define MPU9250_I2C_SLV3_ADDR_I2C_SLV3_RNW_Pos  7
#define MPU9250_I2C_SLV3_ADDR_I2C_SLV3_RNW_WRITE    (0U << \
                                        MPU9250_I2C_SLV3_ADDR_I2C_SLV3_RNW_Pos)
#define MPU9250_I2C_SLV3_ADDR_I2C_SLV3_RNW_READ     (1U << \
                                        MPU9250_I2C_SLV3_ADDR_I2C_SLV3_RNW_Pos)


//
//  0x2F - I2C_SLV3_REG
//  I2C Slave 3 Control - Register address from which to begin data transfer.
//
#define MPU9250_REG_I2C_SLV3_REG            0x2F


//
//  0x30 - I2C_SLV3_CTRL
//  I2C Slave 3 Control - Control.
//
#define MPU9250_REG_I2C_SLV3_CTRL           0x30

// I2C_SLV3_LENG: Number of bytes to be read
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_LENG_Pos  0
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_LENG_Msk  (0xfU << \
                                        MPU9250_I2C_SLV3_CTR¬_I2C_SLV3_LENG_Pos)
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_LENG(x)   (((x) << \
                                    MPU9250_I2C_SLV3_CTRL_I2C_SLV3_LENG_Pos) & \
                                    MPU9250_I2C_SLV3_CTRL_I2C_SLV3_LENG_Msk)
// I2C_SLV3_GRP: Indicates whether group registers are at even or off addresses
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_GRP_Pos  4
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_GRP_ODD  (0U << \
                                        MPU9250_I2C_SLV3_CTRL_I2C_SLV3_GRP_Pos)
#define MPU9250_I2C_SLV3_ADDR_I2C_SLV3_GRP_EVEN (1U << \
                                        MPU9250_I2C_SLV3_CTRL_I2C_SLV3_GRP_Pos)
// I2C_SLV3_REG_DIS: Disable writing of register address
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_REG_DIS_Pos  5
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_REG_DIS      (1U << \
                                    MPU9250_I2C_SLV3_CTRL_I2C_SLV3_REG_DIS_Pos)
// I2C_SLV3_BYTE_SW: Swap bytes in grouped registers
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_BYTE_SW_Pos  6
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_BYTE_SW      (1U << \
                                    MPU9250_I2C_SLV3_CTRL_I2C_SLV3_BYTE_SW_Pos)
// I2C_SLV3_EN: Enable reading from I2C slave
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_EN_Pos       7
#define MPU9250_I2C_SLV3_CTRL_I2C_SLV3_EN           (1U << \
                                        MPU9250_I2C_SLV3_CTRL_I2C_SLV3_EN_Pos)


//
//  0x31 - I2C_SLV4_ADDR
//  I2C Slave 4 Control - Address of I2C slave 4.
//
#define MPU9250_REG_I2C_SLV4_ADDR           0x31

// I2C_ID_4: Physical address of I2C slave
#define MPU9250_I2C_SLV4_ADDR_I2C_ID_4_Pos  0
#define MPU9250_I2C_SLV4_ADDR_I2C_ID_4_Msk  (0x7fU << \
                                            MPU9250_I2C_SLV4_ADDR_I2C_ID_4_Pos)
#define MPU9250_I2C_SLV4_ADDR_I2C_ID_4(x)   (((x) << \
                                        MPU9250_I2C_SLV4_ADDR_I2C_ID_4_Pos) & \
                                        MPU9250_I2C_SLV4_ADDR_I2C_ID_4_Pos)
// I2C_SLV4_RNW: Selects whether transfer is a read or write
#define MPU9250_I2C_SLV4_ADDR_I2C_SLV4_RNW_Pos  7
#define MPU9250_I2C_SLV4_ADDR_I2C_SLV4_RNW_WRITE    (0U << \
                                        MPU9250_I2C_SLV4_ADDR_I2C_SLV4_RNW_Pos)
#define MPU9250_I2C_SLV4_ADDR_I2C_SLV4_RNW_READ     (1U << \
                                        MPU9250_I2C_SLV4_ADDR_I2C_SLV4_RNW_Pos)


//
//  0x32 - I2C_SLV4_REG
//  I2C Slave 4 Control - Register address from which to begin data transfer.
//
#define MPU9250_REG_I2C_SLV4_REG            0x32


//
//  0x33 - I2C_SLV4_DO
//  I2C Slave 4 Control - Data to be written if enabled.
//
#define MPU9250_REG_I2C_SLV4_DO             0x33


//
//  0x34 - I2C_SLV4_CTRL
//  I2C Slave 4 Control - Control
//
#define MPU9250_REG_I2C_SLV4_CTRL           0x34

// I2C_MST_DLY: When enabled, slaves sampled every (1+I2C_MST_DLY) samples
#define MPU9250_I2C_SLV4_CTRL_I2C_MST_DLY_Pos   0
#define MPU9250_I2C_SLV4_CTRL_I2C_MST_DLY_Msk   (0x1fU << \
                                        MPU9250_I2C_SLV4_CTRL_I2C_MST_DLY_Pos)
#define MPU9250_I2C_SLV4_CTRL_I2C_MST_DLY       (((x) << \
                                    MPU9250_I2C_SLV4_CTRL_I2C_MST_DLY_Pos) & \
                                    MPU9250_I2C_SLV4_CTRL_I2C_MST_DLY_Msk)
// I2C_SLV4_REG_DIS: Disable writing of register address
#define MPU9250_I2C_SLV4_CTRL_I2C_SLV4_REG_DIS_Pos  5
#define MPU9250_I2C_SLV4_CTRL_I2C_SLV4_REG_DIS      (1U << \
                                    MPU9250_I2C_SLV4_CTRL_I2C_SLV4_REG_DIS_Pos)
// SLV4_DONE_INT_EN: Enables interrupt on completion of I2C slave 4 transfer
#define MPU9250_I2C_SLV4_CTRL_SLV4_DONE_INT_EN_Pos  6
#define MPU9250_I2C_SLV4_CTRL_SLV4_DONE_INT_EN      (1U << \
                                    MPU9250_I2C_SLV4_CTRL_SLV4_DONE_INT_EN_Pos)
// I2C_SLV4_EN: Enable I2C slave 4
#define MPU9250_I2C_SLV4_CTRL_I2C_SLV4_EN_Pos       7
#define MPU9250_I2C_SLV4_CTRL_I2C_SLV4_EN           (1U << \
                                    MPU9250_I2C_SLV4_CTRL_I2C_SLV4_EN_Pos)


//
//  0x35 - I2C_SLV4_DI
//  I2C Slave 4 Control - Data read.
//
#define MPU9250_REG_I2C_SLV4_DI             0x35


//
//  0x36 - I2C_MST_STATUS
//  I2C Master Status - A read of this reg clears all status bits in this reg.
//
#define MPU9250_REG_I2C_MST_STATUS          0x36

// I2C_SLV0_NACK: Asserted when slave 0 receives a nack
#define MPU9250_I2C_MST_STATUS_I2C_SLV0_NACK_Pos    0
#define MPU9250_I2C_MST_STATUS_I2C_SLV0_NACK        (1U << \
                                    MPU9250_I2C_MST_STATUS_I2C_SLV0_NACK_Pos)
// I2C_SLV1_NACK: Asserted when slave 1 receives a nack
#define MPU9250_I2C_MST_STATUS_I2C_SLV1_NACK_Pos    1
#define MPU9250_I2C_MST_STATUS_I2C_SLV1_NACK        (1U << \
                                    MPU9250_I2C_MST_STATUS_I2C_SLV1_NACK_Pos)
// I2C_SLV2_NACK: Asserted when slave 2 receives a nack
#define MPU9250_I2C_MST_STATUS_I2C_SLV2_NACK_Pos    2
#define MPU9250_I2C_MST_STATUS_I2C_SLV2_NACK        (1U << \
                                    MPU9250_I2C_MST_STATUS_I2C_SLV2_NACK_Pos)
// I2C_SLV3_NACK: Asserted when slave 3 receives a nack
#define MPU9250_I2C_MST_STATUS_I2C_SLV3_NACK_Pos    3
#define MPU9250_I2C_MST_STATUS_I2C_SLV3_NACK        (1U << \
                                    MPU9250_I2C_MST_STATUS_I2C_SLV3_NACK_Pos)
// I2C_SLV4_NACK: Asserted when slave 4 receives a nack
#define MPU9250_I2C_MST_STATUS_I2C_SLV4_NACK_Pos    4
#define MPU9250_I2C_MST_STATUS_I2C_SLV4_NACK        (1U << \
                                    MPU9250_I2C_MST_STATUS_I2C_SLV4_NACK_Pos)
// I2C_LOST_ARB: Asserted when I2C master looses arbitration of the I2C bus
#define MPU9250_I2C_MST_STATUS_I2C_LOST_ARB_Pos     5
#define MPU9250_I2C_MST_STATUS_I2C_LOST_ARB         (1U << \
                                    MPU9250_I2C_MST_STATUS_I2C_LOST_ARB_Pos)
// I2C_SLV4_DONE: Asserted when I2C slave 4’s transfer is complete
#define MPU9250_I2C_MST_STATUS_I2C_SLV4_DONE_Pos    6
#define MPU9250_I2C_MST_STATUS_I2C_SLV4_DONE        (1U << \
                                    MPU9250_I2C_MST_STATUS_I2C_SLV4_DONE_Pos)
// PASS_THROUGH: Status of FSYNC interrupt
#define MPU9250_I2C_MST_STATUS_PASS_THROUGH_Pos     7
#define MPU9250_I2C_MST_STATUS_PASS_THROUGH         (1U << \
                                    MPU9250_I2C_MST_STATUS_PASS_THROUGH_Pos)




//
//  0x37 - INT_PIN_CFG
//  Interrupt Pin and Bypass Enable Configuration
//
#define MPU9250_REG_INT_PIN_CFG             0x37

// BYPASS_EN: Put I2C master interface pins into bypass mode if master disabled
#define MPU9250_INT_PIN_CFG_BYPASS_EN_Pos           1
#define MPU9250_INT_PIN_CFG_BYPASS_EN  (1U << MPU9250_INT_PIN_CFG_BYPASS_EN_Pos)
// FSYNC_INT_MODE_EN: Use FSYNC pin as interrupt
#define MPU9250_INT_PIN_CFG_FSYNC_INT_MODE_EN_Pos   2
#define MPU9250_INT_PIN_CFG_FSYNC_INT_MODE_EN       (1U << \
                                    MPU9250_INT_PIN_CFG_FSYNC_INT_MODE_EN_Pos)
// ACTL_FSYNC: FSYNC pin as interrupt is active low if this bit is set
#define MPU9250_INT_PIN_CFG_ACTL_FSYNC_Pos          3
#define MPU9250_INT_PIN_CFG_ACTL_FSYNC              (1U << \
                                    MPU9250_INT_PIN_CFG_ACTL_FSYNC_Pos)
// INT_ANYRD_2CLEAR: If set any read operation clears the interrupt status flags
#define MPU9250_INT_PIN_CFG_ANYRD_2CLEAR_Pos        4
#define MPU9250_INT_PIN_CFG_ANYRD_2CLEAR            (1U << \
                                        MPU9250_INT_PIN_CFG_ANYRD_2CLEAR_Pos)
// LATCH_INT_EN: Latch interrupt pin level until interrupt status cleared
#define MPU9250_INT_PIN_CFG_LATCH_INT_EN_Pos        5
#define MPU9250_INT_PIN_CFG_LATCH_INT_EN            (1U << \
                                        MPU9250_INT_PIN_CFG_LATCH_INT_EN_Pos)
// OPEN: Configure interrupt pin as open drain
#define MPU9250_INT_PIN_CFG_OPEN_Pos                6
#define MPU9250_INT_PIN_CFG_OPEN            (1U << MPU9250_INT_PIN_CFG_OPEN_Pos)
// ACTL: If set interrupt pin is active low
#define MPU9250_INT_PIN_CFG_ACTL_Pos                7
#define MPU9250_INT_PIN_CFG_ACTL            (1U << MPU9250_INT_PIN_CFG_ACTL_Pos)


//
//  0x38 - INT_ENABLE
//
//
#define MPU9250_REG_INT_ENABLE              0x38

// RAW_RDY_EN: Enable raw sensor data ready interrupt
#define MPU9250_INT_ENABLE_RAW_RDY_EN_Pos           0
#define MPU9250_INT_ENABLE_RAW_RDY_EN  (1U << MPU9250_INT_ENABLE_RAW_RDY_EN_Pos)
// DMP_INT_EN: Enable DMP interrupt
#define MPU9250_INT_ENABLE_DMP_INT_EN_Pos           1
#define MPU9250_INT_ENABLE_DMP_INT_EN  (1U << MPU9250_INT_ENABLE_DMP_INT_EN_Pos)
// FSYNC_INT_EN: Enable FSYNC interrupt
#define MPU9250_INT_ENABLE_FSYNC_INT_EN_Pos         3
#define MPU9250_INT_ENABLE_FSYNC_INT_EN             (1U << \
                                            MPU9250_INT_ENABLE_FSYNC_INT_EN_Pos)
// FIFO_OVERFLOW_EN: Enable FIFO overflow interrupt
#define MPU9250_INT_ENABLE_FIFO_OVERFLOW_EN_Pos     4
#define MPU9250_INT_ENABLE_FIFO_OVERFLOW_EN         (1U << \
                                        MPU9250_INT_ENABLE_FIFO_OVERFLOW_EN_Pos)
// ZMOT_EN: Enable Zero Motion detection interrupt
#define MPU9250_INT_ENABLE_ZMOT_EN_Pos              5
#define MPU9250_INT_ENABLE_ZMOT_EN      (1U << MPU9250_INT_ENABLE_ZMOT_EN_Pos)
// WOM_EN: Enable Wake-on-Motion interrupt
#define MPU9250_INT_ENABLE_WOM_EN_Pos               6
#define MPU9250_INT_ENABLE_WOM_EN       (1U << MPU9250_INT_ENABLE_WOM_EN_Pos)
// FF_EN: Enable Free Fall detection interrupt
#define MPU9250_INT_ENABLE_FF_EN_Pos                7
#define MPU9250_INT_ENABLE_FF_EN        (1U << MPU9250_INT_ENABLE_FF_EN_Pos)


//
//  0x3A - INT_STATUS
//  Interrupt Status - Read register to clear flags.
//
#define MPU9250_REG_INT_STATUS              0x3A

// RAW_DATA_RDY_INT: Raw sensor data ready interrupt
#define MPU9250_INT_STATUS_RAW_DATA_RDY_INT_Pos     0
#define MPU9250_INT_STATUS_RAW_DATA_RDY_INT         (1U << \
                                        MPU9250_INT_STATUS_RAW_DATA_RDY_INT_Pos)
// DMP_INT: DMP interrupt
#define MPU9250_INT_STATUS_DMP_INT_Pos              1
#define MPU9250_INT_STATUS_DMP_INT      (1U << MPU9250_INT_STATUS_DMP_INT_Pos)
// FSYNC_INT: FSYNC interrupt
#define MPU9250_INT_STATUS_FSYNC_INT_Pos            3
#define MPU9250_INT_STATUS_FSYNC_INT    (1U << MPU9250_INT_STATUS_FSYNC_INT_Pos)
// FIFO_OVERFLOW_INT: FIFO overflow interrupt
#define MPU9250_INT_STATUS_FIFO_OVERFLOW_INT_Pos    4
#define MPU9250_INT_STATUS_FIFO_OVERFLOW_INT        (1U << \
                                    MPU9250_INT_STATUS_FIFO_OVERFLOW_INT_Pos)
// ZMOT_INT: Zero Motion Detection interrupt
#define MPU9250_INT_STATUS_ZMOT_INT_Pos             5
#define MPU9250_INT_STATUS_ZMOT_INT     (1U << MPU9250_INT_STATUS_ZMOT_INT_Pos)
// WOM_INT: Wake-on-Motion interrupt
#define MPU9250_INT_STATUS_WOM_INT_Pos              6
#define MPU9250_INT_STATUS_WOM_INT      (1U << MPU9250_INT_STATUS_WOM_INT_Pos)
// FF_INT: Free Fall Detection interrupt
#define MPU9250_INT_STATUS_FF_INT_Pos               7
#define MPU9250_INT_STATUS_FF_INT       (1U << MPU9250_INT_STATUS_FF_INT_Pos)




//
//  0x3B - ACCEL_XOUT_H
//  Accelerometer Output: X High
//
#define MPU9250_REG_ACCEL_XOUT_H            0x3B

//
//  0x3C - ACCEL_XOUT_L
//  Accelerometer Output: X Low
//
#define MPU9250_REG_ACCEL_XOUT_L            0x3C


//
//  0x3D - ACCEL_YOUT_H
//  Accelerometer Output: Y High
//
#define MPU9250_REG_ACCEL_YOUT_H            0x3D

//
//  0x3E - ACCEL_YOUT_L
//  Accelerometer Output: Y Low
//
#define MPU9250_REG_ACCEL_YOUT_L            0x3E


//
//  0x3F - ACCEL_ZOUT_H
//  Accelerometer Output: Z High
//
#define MPU9250_REG_ACCEL_ZOUT_H            0x3F

//
//  0x40 - ACCEL_ZOUT_L
//  Accelerometer Output: Z Low
//
#define MPU9250_REG_ACCEL_ZOUT_L            0x40




//
//  0x41 - TEMP_OUT_H
//  Temperature Output: High
//
#define MPU9250_REG_TEMP_OUT_H              0x41

//
//  0x42 - TEMP_OUT_L
//  Temperature Output: Low
//
#define MPU9250_REG_TEMP_OUT_L              0x42




//
//  0x43 - GYRO_XOUT_H
//  Gyroscope Output: X High
//
#define MPU9250_REG_GYRO_XOUT_H             0x43

//
//  0x44 - GYRO_XOUT_L
//  Gyroscope Output: X Low
//
#define MPU9250_REG_GYRO_XOUT_L             0x44


//
//  0x45 - GYRO_YOUT_H
//  Gyroscope Output: Y High
//
#define MPU9250_REG_GYRO_YOUT_H             0x45

//
//  0x46 - GYRO_YOUT_L
//  Gyroscope Output: Y Low
//
#define MPU9250_REG_GYRO_YOUT_L             0x46


//
//  0x47 - GYRO_ZOUT_H
//  Gyroscope Output: Z High
//
#define MPU9250_REG_GYRO_ZOUT_H             0x47

//
//  0x48 - GYRO_ZOUT_L
//  Gyroscope Output: Z Low
//
#define MPU9250_REG_GYRO_ZOUT_L             0x48




//
//  0x49 - EXT_SENS_DATA_00
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_00        0x49

//
//  0x4A - EXT_SENS_DATA_01
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_01        0x4A

//
//  0x4B - EXT_SENS_DATA_02
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_02        0x4B

//
//  0x4C - EXT_SENS_DATA_03
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_03        0x4C

//
//  0x4D - EXT_SENS_DATA_04
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_04        0x4D

//
//  0x4E - EXT_SENS_DATA_05
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_05        0x4E

//
//  0x4F - EXT_SENS_DATA_06
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_06        0x4F

//
//  0x50 - EXT_SENS_DATA_07
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_07        0x50

//
//  0x51 - EXT_SENS_DATA_08
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_08        0x51

//
//  0x52 - EXT_SENS_DATA_09
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_09        0x52

//
//  0x53 - EXT_SENS_DATA_10
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_10        0x53

//
//  0x54 - EXT_SENS_DATA_11
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_11        0x54

//
//  0x55 - EXT_SENS_DATA_12
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_12        0x55

//
//  0x56 - EXT_SENS_DATA_13
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_13        0x56

//
//  0x57 - EXT_SENS_DATA_14
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_14        0x57

//
//  0x58 - EXT_SENS_DATA_15
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_15        0x58

//
//  0x59 - EXT_SENS_DATA_16
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_16        0x59

//
//  0x5A - EXT_SENS_DATA_17
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_17        0x5A

//
//  0x5B - EXT_SENS_DATA_18
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_18        0x5B

//
//  0x5C - EXT_SENS_DATA_19
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_19        0x5C

//
//  0x5D - EXT_SENS_DATA_20
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_20        0x5D

//
//  0x5E - EXT_SENS_DATA_21
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_21        0x5E

//
//  0x5F - EXT_SENS_DATA_22
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_22        0x5F

//
//  0x60 - EXT_SENS_DATA_23
//  External Sensor Data
//
#define MPU9250_REG_EXT_SENS_DATA_23        0x60




//
//  0x63 - I2C_SLV0_DO
//  I2C Slave 0 Data Out
//
#define MPU9250_REG_I2C_SLV0_DO             0x63


//
//  0x64 - I2C_SLV0_D1
//  I2C Slave 1 Data Out
//
#define MPU9250_REG_I2C_SLV1_DO             0x64


//
//  0x65 - I2C_SLV0_D2
//  I2C Slave 2 Data Out
//
#define MPU9250_REG_I2C_SLV2_DO             0x65


//
//  0x66 - I2C_SLV0_D3
//  I2C Slave 3 Data Out
//
#define MPU9250_REG_I2C_SLV3_DO             0x66




//
//  0x67 - I2C_MST_DELAY_CTRL
//  I2C Master Delay Control
//

#define MPU9250_REG_I2C_MST_DELAY_CTRL      0x67

// I2C_SLV0_DLY_EN: If set slave 0 accessed every 1 + I2C_MST_DLY samples
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV0_DLY_EN_Pos  0
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV0_DLY_EN      (1U << \
                                MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV0_DLY_EN_Pos)
// I2C_SLV1_DLY_EN: If set slave 1 accessed every 1 + I2C_MST_DLY samples
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV1_DLY_EN_Pos  1
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV1_DLY_EN      (1U << \
                                MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV1_DLY_EN_Pos)
// I2C_SLV2_DLY_EN: If set slave 2 accessed every 1 + I2C_MST_DLY samples
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV2_DLY_EN_Pos  2
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV2_DLY_EN      (1U << \
                                MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV2_DLY_EN_Pos)
// I2C_SLV3_DLY_EN: If set slave 3 accessed every 1 + I2C_MST_DLY samples
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV3_DLY_EN_Pos  3
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV3_DLY_EN      (1U << \
                                MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV3_DLY_EN_Pos)
// I2C_SLV4_DLY_EN: If set slave 4 accessed every 1 + I2C_MST_DLY samples
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV4_DLY_EN_Pos  4
#define MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV4_DLY_EN      (1U << \
                                MPU9250_I2C_MST_DELAY_CTRL_I2C_SLV4_DLY_EN_Pos)
// DELAY_ES_SHADOW: Delay shadowing of ext sensor data until all data received
#define MPU9250_I2C_MST_DELAY_CTRL_DELAY_ES_SHADOW_Pos  7
#define MPU9250_I2C_MST_DELAY_CTRL_DELAY_ES_SHADOW      (1U << \
                                MPU9250_I2C_MST_DELAY_CTRL_DELAY_ES_SHADOW_Pos)


//
//  0x68 - SIGNAL_PATH_RESET
//  Signal Path Reset
//
#define MPU9250_REG_SIGNAL_PATH_RESET       0x68

// TEMP_RST: Reset temperature sensor digital signal path
#define MPU9250_SIGNAL_PATH_RESET_TEMP_RST_Pos  0
#define MPU9250_SIGNAL_PATH_RESET_TEMP_RST      (1U << \
                                        MPU9250_SIGNAL_PATH_RESET_TEMP_RST_Pos)
// ACCEL_RST: Reset accelerometer digital signal path
#define MPU9250_SIGNAL_PATH_RESET_ACCEL_RST_Pos 1
#define MPU9250_SIGNAL_PATH_RESET_ACCEL_RST     (1U << \
                                        MPU9250_SIGNAL_PATH_RESET_ACCEL_RST_Pos)
// GYRO_RST: Reset gyroscope digital signal path
#define MPU9250_SIGNAL_PATH_RESET_GYRO_RST_Pos  2
#define MPU9250_SIGNAL_PATH_RESET_GYRO_RST     (1U << \
                                        MPU9250_SIGNAL_PATH_RESET_GYRO_RST_Pos)


//
//  0x69 - ACCEL_INTEL_CTRL
//  Accelerometer Interrupt Control
//
#define MPU9250_REG_MOT_DETECT_CTRL         0x69

//  ACCEL_INTEL_MODE: If set current sample compared with the previous sample
#define MPU9250_MOT_DETECT_CTRL_ACCEL_INTEL_MODE_Pos    6
#define MPU9250_MOT_DETECT_CTRL_ACCEL_INTEL_MODE        (1U << \
                                MPU9250_MOT_DETECT_CTRL_ACCEL_INTEL_MODE_Pos)
// ACCEL_INTEL_EN Enable the Wake-on-Motion detection logic
#define MPU9250_MOT_DETECT_CTRL_ACCEL_INTEL_EN_Pos      7
#define MPU9250_MOT_DETECT_CTRL_ACCEL_INTEL_EN          (1U << \
                                MPU9250_MOT_DETECT_CTRL_ACCEL_INTEL_EN_Pos)


//
//  0x6A - USER_CTRL
//  User Control
//
#define MPU9250_REG_USER_CTRL               0x6A

// SIG_COND_RST: Reset accel, gyro and temp signal paths and registers
#define MPU9250_USER_CTRL_SIG_COND_RST_Pos      0
#define MPU9250_USER_CTRL_SIG_COND_RST          (1U << \
                                            MPU9250_USER_CTRL_SIG_COND_RST_Pos)
// I2C_MST_RST: Reset I2C Master module
#define MPU9250_USER_CTRL_I2C_MST_RST_Pos       1
#define MPU9250_USER_CTRL_I2C_MST_RST  (1U << MPU9250_USER_CTRL_I2C_MST_RST_Pos)
// FIFO_RST: Reset FIFO module
#define MPU9250_USER_CTRL_FIFO_RST_Pos          2
#define MPU9250_USER_CTRL_FIFO_RST      (1U << MPU9250_USER_CTRL_FIFO_RST_Pos)
// I2C_IF_DIS: Disable I2C Slave module, put serial interface in SPI mode only
#define MPU9250_USER_CTRL_I2C_IF_DIS_Pos        4
#define MPU9250_USER_CTRL_I2C_IF_DIS    (1U << MPU9250_USER_CTRL_I2C_IF_DIS_Pos)
// I2C_MST_EN: Enable the I2C Master interface module
#define MPU9250_USER_CTRL_I2C_MST_EN_Pos        5
#define MPU9250_USER_CTRL_I2C_MST_EN    (1U << MPU9250_USER_CTRL_I2C_MST_EN_Pos)
// FIFO_EN: Enable FIFO operation mode
#define MPU9250_USER_CTRL_FIFO_EN_Pos           6
#define MPU9250_USER_CTRL_FIFO_EN       (1U << MPU9250_USER_CTRL_FIFO_EN_Pos)
// DMP_EN: Enable Digital Motion Processor
#define MPU9250_USER_CTRL_DMP_EN_Pos            7
#define MPU9250_USER_CTRL_DMP_EN        (1U << MPU9250_USER_CTRL_DMP_EN_Pos)


//
//  0x6B - PWR_MGMT_1
//  Power Management 1
//
#define MPU9250_REG_PWR_MGMT_1              0x6B

// CLKSEL: Select internal clock source
#define MPU9250_PWR_MGMT_1_CLKSEL_Pos           0
#define MPU9250_PWR_MGMT_1_CLKSEL_Msk   (0x7U << MPU9250_PWR_MGMT_1_CLKSEL_Pos)
#define MPU9250_PWR_MGMT_1_CLKSEL(x)    (((x) << MPU9250_PWR_MGMT_1_CLKSEL_Pos)\
                                         & MPU9250_PWR_MGMT_1_CLKSEL_Msk)
#define MPU9250_PWR_MGMT_1_CLKSEL_20MHZ_Val     0U
#define MPU9250_PWR_MGMT_1_CLKSEL_20MHZ         MPU9250_PWR_MGMT_1_CLKSEL(\
                                        MPU9250_PWR_MGMT_1_CLKSEL_20MHZ_Val)
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO_Val      1U
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO          MPU9250_PWR_MGMT_1_CLKSEL(\
                                        MPU9250_PWR_MGMT_1_CLKSEL_AUTO_Val)
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO2_Val     2U
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO2         MPU9250_PWR_MGMT_1_CLKSEL(\
                                        MPU9250_PWR_MGMT_1_CLKSEL_AUTO2_Val)
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO3_Val     3U
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO3         MPU9250_PWR_MGMT_1_CLKSEL(\
                                        MPU9250_PWR_MGMT_1_CLKSEL_AUTO3_Val)
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO4_Val     4U
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO4         MPU9250_PWR_MGMT_1_CLKSEL(\
                                        MPU9250_PWR_MGMT_1_CLKSEL_AUTO4_Val)
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO5_Val     5U
#define MPU9250_PWR_MGMT_1_CLKSEL_AUTO5         MPU9250_PWR_MGMT_1_CLKSEL(\
                                        MPU9250_PWR_MGMT_1_CLKSEL_AUTO5_Val)
#define MPU9250_PWR_MGMT_1_CLKSEL_20MHZ6_Val    6U
#define MPU9250_PWR_MGMT_1_CLKSEL_20MHZ6        MPU9250_PWR_MGMT_1_CLKSEL(\
                                        MPU9250_PWR_MGMT_1_CLKSEL_20MHZ6_Val)
#define MPU9250_PWR_MGMT_1_CLKSEL_NONE_Val      7U
#define MPU9250_PWR_MGMT_1_CLKSEL_NONE          MPU9250_PWR_MGMT_1_CLKSEL(\
                                        MPU9250_PWR_MGMT_1_CLKSEL_NONE_Val)
// PD_PTAT: Power down internal PTAT voltage generator and PTAT ADC (TEMP_DIS)
#define MPU9250_PWR_MGMT_1_PD_PTAT_Pos          3
#define MPU9250_PWR_MGMT_1_PD_PTAT      (1U << MPU9250_PWR_MGMT_1_PD_PTAT_Pos)
// TEMP_DIS: Disable temperature sensor (PD_PTAT)
#define MPU9250_PWR_MGMT_1_TEMP_DIS_Pos         3
#define MPU9250_PWR_MGMT_1_TEMP_DIS     (1U << MPU9250_PWR_MGMT_1_TEMP_DIS_Pos)
// GYRO_STANDBY: Enable gyro drive and pll circuitry but not sense paths
#define MPU9250_PWR_MGMT_1_GYRO_STANDBY_Pos     4
#define MPU9250_PWR_MGMT_1_GYRO_STANDBY         (1U << \
                                        MPU9250_PWR_MGMT_1_GYRO_STANDBY_Pos)
// CYCLE: Cycle between sleep and taking a single sample
#define MPU9250_PWR_MGMT_1_CYCLE_Pos            5
#define MPU9250_PWR_MGMT_1_CYCLE        (1U << MPU9250_PWR_MGMT_1_CYCLE_Pos)
// SLEEP: Set chip into sleep mode
#define MPU9250_PWR_MGMT_1_SLEEP_Pos            6
#define MPU9250_PWR_MGMT_1_SLEEP        (1U << MPU9250_PWR_MGMT_1_SLEEP_Pos)
// H_RESET: Reset internal registers and restore default settings
#define MPU9250_PWR_MGMT_1_H_RESET_Pos          7
#define MPU9250_PWR_MGMT_1_H_RESET      (1U << MPU9250_PWR_MGMT_1_H_RESET_Pos)
// DEVICE_RESET: Reset internal registers and restore default settings
#define MPU9250_PWR_MGMT_1_DEVICE_RESET_Pos     7
#define MPU9250_PWR_MGMT_1_DEVICE_RESET (1U << \
                                        MPU9250_PWR_MGMT_1_DEVICE_RESET_Pos)


//
//  0x6C - PWR_MGMT_2
//  Power Management 2
//
#define MPU9250_REG_PWR_MGMT_2              0x6C

//  DISABLE_ZG: Disable gyroscope Z-axis
#define MPU9250_PWR_MGMT_2_DISABLE_ZG_Pos       0
#define MPU9250_PWR_MGMT_2_DISABLE_ZG (1U << MPU9250_PWR_MGMT_2_DISABLE_ZG_Pos)
//  DISABLE_YG: Disable gyroscope Y-axis
#define MPU9250_PWR_MGMT_2_DISABLE_YG_Pos       1
#define MPU9250_PWR_MGMT_2_DISABLE_YG (1U << MPU9250_PWR_MGMT_2_DISABLE_YG_Pos)
//  DISABLE_XG: Disable gyroscope X-axis
#define MPU9250_PWR_MGMT_2_DISABLE_XG_Pos       2
#define MPU9250_PWR_MGMT_2_DISABLE_XG (1U << MPU9250_PWR_MGMT_2_DISABLE_XG_Pos)
//  DISABLE_ZA: Disable accelerometer Z-axis
#define MPU9250_PWR_MGMT_2_DISABLE_ZA_Pos       3
#define MPU9250_PWR_MGMT_2_DISABLE_ZA (1U << MPU9250_PWR_MGMT_2_DISABLE_ZA_Pos)
//  DISABLE_YA: Disable accelerometer Y-axis
#define MPU9250_PWR_MGMT_2_DISABLE_YA_Pos       4
#define MPU9250_PWR_MGMT_2_DISABLE_YA (1U << MPU9250_PWR_MGMT_2_DISABLE_YA_Pos)
//  DISABLE_XA: Disable accelerometer X-axis
#define MPU9250_PWR_MGMT_2_DISABLE_XA_Pos       5
#define MPU9250_PWR_MGMT_2_DISABLE_XA (1U << MPU9250_PWR_MGMT_2_DISABLE_XA_Pos)
// LP_WAKE_CTRL: Frequency of wake-ups during Accelerometer Only Low Power Mode
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_Pos     6
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_Msk     (0x3U << \
                                        MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_Pos)
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL(x)    (((x) << \
                                        MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_Pos) &\
                                        MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_Msk)
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_0125_Val    0U
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_0125   MPU9250_PWR_MGMT_2_LP_WAKE_CTRL(\
                                    MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_0125_Val)
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_0500_Val    1U
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_0500   MPU9250_PWR_MGMT_2_LP_WAKE_CTRL(\
                                    MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_0500_Val)
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_2000_Val    2U
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_2000   MPU9250_PWR_MGMT_2_LP_WAKE_CTRL(\
                                    MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_2000_Val)
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_4000_Val    3U
#define MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_4000   MPU9250_PWR_MGMT_2_LP_WAKE_CTRL(\
                                    MPU9250_PWR_MGMT_2_LP_WAKE_CTRL_2000_Val)




//
//  0x6D - DMP_CTRL_1
//
//
#define MPU9250_REG_DMP_CTRL_1              0x6D


//
//  0x6E - DMP_CTRL_2
//
//
#define MPU9250_REG_DMP_CTRL_2              0x6E


//
//  0x6F - DMP_CTRL_3
//
//
#define MPU9250_REG_DMP_CTRL_3              0x6F


//
//  0x70 - FW_START_H
//  Firmware start value for DMP - High
//
#define MPU9250_REG_FW_START_H              0x70

//
//  0x71 - FW_START_L
//  Firmware start value for DMP - Low
//
#define MPU9250_REG_FW_START_L              0x71




//
//  0x72 - FIFO_COUNTH
//  FIFO Count - High
//
#define MPU9250_REG_FIFO_COUNTH             0x72

#define MPU9250_REG_FIFO_COUNTH_Msk         0x1fU

//
//  0x73 - FIFO_COUNTL
//  FIFO Count - Low
//
#define MPU9250_REG_FIFO_COUNTL             0x73


//
//  0x74 - FIFO_R_W
//  FIFO Read Write
//
#define MPU9250_REG_FIFO_R_W                0x74




//
//  0x75 - WHO_AM_I
//  Who Am I
//
#define MPU9250_REG_WHO_AM_I                0x75

#define MPU9250_WHO_AM_I_VAL            0x71




//
//  0x77 - XA_OFFSET_H
//  Accelerometer Offset Cancelation - X Axis High
//
#define MPU9250_REG_XA_OFFSET_H             0x77

//
//  0x78 - XA_OFFSET_L
//  Accelerometer Offset Cancelation - X Axis Low
//
#define MPU9250_REG_XA_OFFSET_L             0x78


//
//  0x7A - YA_OFFSET_H
//  Accelerometer Offset Cancelation - Y Axis High
//
#define MPU9250_REG_YA_OFFSET_H             0x7A

//
//  0x7B - YA_OFFSET_L
//  Accelerometer Offset Cancelation - Y Axis Lows
//
#define MPU9250_REG_YA_OFFSET_L             0x7B


//
//  0x7D - ZA_OFFSET_H
//  Accelerometer Offset Cancelation - Z Axis High
//
#define MPU9250_REG_ZA_OFFSET_H             0x7D

//
//  0x7E - ZA_OFFSET_L
//  Accelerometer Offset Cancelation - Z Axis Low
//
#define MPU9250_REG_ZA_OFFSET_L             0x7E


#endif /* mpu9250_registers_h */
