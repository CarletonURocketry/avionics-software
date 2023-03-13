/**
 * @file ak8963-registers.h
 * @desc Register definitions for AK8963 magnetometer
 * @author Samuel Dewan
 * @date 2021-09-29
 * Last Author:
 * Last Edited On:
 */

#ifndef ak8963_registers_h
#define ak8963_registers_h

// MSB of register address specifies read or write for SPI interface
#define AK8963_READ         (1U << 7)
#define AK8963_WRITE        (0U << 7)

#define AK8963_I2C_ADDR     0b0001100U


// Sensitivities in microH per LSB
#define AK8963_SENSITIVITY_14BIT    600
#define AK8963_SENSITIVITY_16BIT    150



//
//  0x00 - WIA
//  Who Am I
//
#define AK8963_REG_WIA                      0x00

#define AK8963_WHO_AM_I_VAL             0x48


//
//  0x01 - INFO
//  Information
//
#define AK8963_REG_INFO                     0x01




//
//  0x02 - ST1
//  Status 1
//
#define AK8963_REG_ST1                      0x02

// DRDY: Data Ready
#define AK8963_ST1_DRDY_Pos             0
#define AK8963_ST1_DRDY                 (1U << AK8963_ST1_DRDY_Pos)
// DOR: Data Overrun
#define AK8963_ST1_DOR_Pos              1
#define AK8963_ST1_DOR                  (1U << AK8963_ST1_DOR_Pos)




//
//  0x03 - HXL
//  X axis data - Low
//
#define AK8963_REG_HXL                      0x03

//
//  0x04 - HXH
//  X axis data - High
//
#define AK8963_REG_HXH                      0x04


//
//  0x05 - HYL
//  Y axis data - Low
//
#define AK8963_REG_HYL                      0x05

//
//  0x06 - HYH
//  Y axis data - High
//
#define AK8963_REG_HYH                      0x06


//
//  0x07 - HZL
//  Z axis data - Low
//
#define AK8963_REG_HZL                      0x07

//
//  0x08 - HZH
//  Z axis data - High
//
#define AK8963_REG_HZH                      0x08




//
//  0x09 - ST2
//  Status 2
//
#define AK8963_REG_ST2                      0x09

// HOFL: Magnetic sensor overflow
#define AK8963_ST2_HOFL_Pos             3
#define AK8963_ST2_HOFL                 (1U << AK8963_ST2_HOFL_Pos)
// BITM: Output bit setting (mirror)
#define AK8963_ST2_BITM_Pos             4
#define AK8963_ST2_BITM                 (1U << AK8963_ST2_BITM_Pos)





//
//  0x0A - CNTL1
//  Control 1
//
#define AK8963_REG_CNTL1                    0x0A

// MODE: Operation mode setting
#define AK8963_CNTL1_MODE_Pos           0
#define AK8963_CNTL1_MODE_Msk           (0xfU << AK8963_CNTL1_MODE_Pos)
#define AK8963_CNTL1_MODE(x)            (((x) << AK8963_CNTL1_MODE_Pos) & \
                                         AK8963_CNTL1_MODE_Msk)
#define AK8963_CNTL1_MODE_POWER_DOWN_Val        0b0000U
#define AK8963_CNTL1_MODE_POWER_DOWN            AK8963_CNTL1_MODE(\
                                        AK8963_CNTL1_MODE_POWER_DOWN_Val)
#define AK8963_CNTL1_MODE_SINGLE_Val            0b0001U
#define AK8963_CNTL1_MODE_SINGLE                AK8963_CNTL1_MODE(\
                                        AK8963_CNTL1_MODE_SINGLE_Val)
#define AK8963_CNTL1_MODE_CONTINUOUS1_Val       0b0010U
#define AK8963_CNTL1_MODE_CONTINUOUS1           AK8963_CNTL1_MODE(\
                                        AK8963_CNTL1_MODE_CONTINUOUS1_Val)
#define AK8963_CNTL1_MODE_CONTINUOUS2_Val       0b0110U
#define AK8963_CNTL1_MODE_CONTINUOUS2           AK8963_CNTL1_MODE(\
                                        AK8963_CNTL1_MODE_CONTINUOUS2_Val)
#define AK8963_CNTL1_MODE_EXT_TRIGGER_Val       0b0100U
#define AK8963_CNTL1_MODE_EXT_TRIGGER           AK8963_CNTL1_MODE(\
                                        AK8963_CNTL1_MODE_EXT_TRIGGER_Val)
#define AK8963_CNTL1_MODE_SELF_TEST_Val         0b1000U
#define AK8963_CNTL1_MODE_SELF_TEST             AK8963_CNTL1_MODE(\
                                        AK8963_CNTL1_MODE_SELF_TEST_Val)
#define AK8963_CNTL1_MODE_FUSE_ROM_ACCESS_Val   0b1000U
#define AK8963_CNTL1_MODE_FUSE_ROM_ACCESS       AK8963_CNTL1_MODE(\
                                        AK8963_CNTL1_MODE_FUSE_ROM_ACCESS_Val)
// BIT: Output resolution setting
#define AK8963_CNTL1_BIT_Pos            4
#define AK8963_CNTL1_BIT_Msk            (1U << AK8963_CNTL1_BIT_Pos)
#define AK8963_CNTL1_BIT_14             (0U << AK8963_CNTL1_BIT_Pos)
#define AK8963_CNTL1_BIT_16             (1U << AK8963_CNTL1_BIT_Pos)


//
//  0x0B - CNTL2
//  Control 2
//
#define AK8963_REG_CNTL2                    0x0B

// SRST: Soft reset
#define AK8963_CNTL2_SRST_Pos           0
#define AK8963_CNTL2_SRST               (1U << AK8963_CNTL2_SRST_Pos)



//
//  0x0C - ASTC
//  Self Test
//
#define AK8963_REG_ASTC                     0x0C

// SELF: Self test control
#define AK8963_ASTC_SELF_Pos            6
#define AK8963_ASTC_SELF                (1U << AK8963_ASTC_SELF_Pos)




//
//  0x0D - TS1
//  Test 1 - Do Not Access
//
#define AK8963_REG_TS1                      0x0D


//
//  0x0E - TS2
//  Test 2 - Do Not Access
//
#define AK8963_REG_TS2                      0x0E




//
//  0x0F - I2CDIS
//  I2C Disable
//
#define AK8963_REG_I2CDIS                   0x0F

#define AK8963_I2CDIS_MAGIC                 0b00011011




//
//  0x10 - ASAX
//  X-axis sensitivity adjustment value (Fuse ROM)
//
#define AK8963_REG_ASAX                     0x10


//
//  0x11 - ASAY
//  X-axis sensitivity adjustment value (Fuse ROM)
//
#define AK8963_REG_ASAY                     0x11


//
//  0x12 - ASAZ
//  X-axis sensitivity adjustment value (Fuse ROM)
//
#define AK8963_REG_ASAZ                     0x12


#endif /* ak8963_registers_h */
