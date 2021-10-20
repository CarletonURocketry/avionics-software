/**
 * @file kx134-1211-registers.h
 * @desc Register definitions for KX134-1211 accelerometer
 * @author Samuel Dewan
 * @date 2021-07-09
 * Last Author:
 * Last Edited On:
 */

#ifndef kx134_1211_registers_h
#define kx134_1211_registers_h


// MSB of register address specifies read or write
#define KX134_1211_READ     (1 << 7)
#define KX134_1211_WRITE    (0 << 7)

#define KX134_1211_I2C_ADDR 0x1E


//
//  0x00 - MAN_ID
//  Manufacturer ID (4 byte string)
//
#define KX134_1211_REG_MAN_ID   0x00

#define KX134_1211_MAN_ID_VAL   "Kion"
#define KX134_1211_MAN_ID_LEN   4

//
//  0x01 - PART_ID
//  Part ID (first byte is WAI, second byte is silicon specific ID)
//
#define KX134_1211_REG_PART_ID  0x01




//
//  0x02 - XADP_L
//  Advanced Data Path Output: X Low
//
#define KX134_1211_REG_XADP_L   0x02

//
//  0x03 - XADP_H
//  Advanced Data Path Output: X High
//
#define KX134_1211_REG_XADP_H   0x03

//
//  0x04 - YADP_L
//  Advanced Data Path Output: Y Low
//
#define KX134_1211_REG_YADP_L   0x04

//
//  0x05 - YADP_H
//  Advanced Data Path Output: Y High
//
#define KX134_1211_REG_YADP_H   0x05

//
//  0x06 - ZADP_L
//  Advanced Data Path Output: Z Low
//
#define KX134_1211_REG_ZADP_L   0x06

//
//  0x07 - ZADP_H
//  Advanced Data Path Output: Z High
//
#define KX134_1211_REG_ZADP_H   0x07




//
//  0x08 - XOUT_L
//  Accelerometer Output: X Low
//
#define KX134_1211_REG_XOUT_L   0x08

//
//  0x09 - XOUT_H
//  Accelerometer Output: X High
//
#define KX134_1211_REG_XOUT_H   0x09

//
//  0x0a - YOUT_L
//  Accelerometer Output: Y Low
//
#define KX134_1211_REG_YOUT_L   0x0a

//
//  0x0b - YOUT_H
//  Accelerometer Output: Y High
//
#define KX134_1211_REG_YOUT_H   0x0b

//
//  0x0c - ZOUT_L
//  Accelerometer Output: Z Low
//
#define KX134_1211_REG_ZOUT_L   0x0c

//
//  0x0d - ZOUT_H
//  Accelerometer Output: Z High
//
#define KX134_1211_REG_ZOUT_H   0x0d




//
//  0x12 - COTR
//  Command Test Response
//
#define KX134_1211_REG_COTR     0x12

#define KX134_1211_COTR_DEFAULT_VAL 0x55
#define KX134_1211_COTR_TEST_VAL    0xAA


//
//  0x13 - WHO_AM_I
//  Who Am I
//
#define KX134_1211_REG_WHO_AM_I 0x13

#define KX134_1211_WHO_AM_I_VAL 0x46


//
//  0x14 - TSCP
//  Current Tilt Position
//
#define KX134_1211_REG_TSCP     0x14

// FU: Face-Up State (Z+)
#define KX134_1211_TSCP_FU_Pos  0
#define KX134_1211_TSCP_FU      (1 << KX134_1211_TSCP_FU_Pos)
// FD: Face-Down State (Z-)
#define KX134_1211_TSCP_FD_Pos  1
#define KX134_1211_TSCP_FD      (1 << KX134_1211_TSCP_FD_Pos)
// UP: Up State (Y+)
#define KX134_1211_TSCP_UP_Pos  2
#define KX134_1211_TSCP_UP      (1 << KX134_1211_TSCP_UP_Pos)
// DO: Down State (Y-)
#define KX134_1211_TSCP_DO_Pos  3
#define KX134_1211_TSCP_DO      (1 << KX134_1211_TSCP_DO_Pos)
// RI: Right State (X+)
#define KX134_1211_TSCP_RI_Pos  4
#define KX134_1211_TSCP_RI      (1 << KX134_1211_TSCP_RI_Pos)
// LE: Left State (X-)
#define KX134_1211_TSCP_LE_Pos  5
#define KX134_1211_TSCP_LE      (1 << KX134_1211_TSCP_LE_Pos)

//
//  0x15 - TSPP
//  Previous Tilt Position
//
#define KX134_1211_REG_TSPP     0x15

// FU: Face-Up State (Z+)
#define KX134_1211_TSPP_FU_Pos  0
#define KX134_1211_TSPP_FU      (1 << KX134_1211_TSPP_FU_Pos)
// FD: Face-Down State (Z-)
#define KX134_1211_TSPP_FD_Pos  1
#define KX134_1211_TSPP_FD      (1 << KX134_1211_TSPP_FD_Pos)
// UP: Up State (Y+)
#define KX134_1211_TSPP_UP_Pos  2
#define KX134_1211_TSPP_UP      (1 << KX134_1211_TSPP_UP_Pos)
// DO: Down State (Y-)
#define KX134_1211_TSPP_DO_Pos  3
#define KX134_1211_TSPP_DO      (1 << KX134_1211_TSPP_DO_Pos)
// RI: Right State (X+)
#define KX134_1211_TSPP_RI_Pos  4
#define KX134_1211_TSPP_RI      (1 << KX134_1211_TSPP_RI_Pos)
// LE: Left State (X-)
#define KX134_1211_TSPP_LE_Pos  5
#define KX134_1211_TSPP_LE      (1 << KX134_1211_TSPP_LE_Pos)




//
//  0x16 - INS1
//  Interrupt Source Register 1
//
#define KX134_1211_REG_INS1     0x16

// TFU: Z Positive (Z+) Reported
#define KX134_1211_INS1_TFU_Pos 0
#define KX134_1211_INS1_TFU     (1 << KX134_1211_INS1_TFU_Pos)
// TFD: Z Negative (Z-) Reported
#define KX134_1211_INS1_TFD_Pos 1
#define KX134_1211_INS1_TFD     (1 << KX134_1211_INS1_TFD_Pos)
// TUP: Y Positive (Y+) Reported
#define KX134_1211_INS1_TUP_Pos 2
#define KX134_1211_INS1_TUP     (1 << KX134_1211_INS1_TUP_Pos)
// TDO: Y Negative (Y-) Reported
#define KX134_1211_INS1_TDO_Pos 3
#define KX134_1211_INS1_TDO     (1 << KX134_1211_INS1_TDO_Pos)
// TRI: X Positive (X+) Reported
#define KX134_1211_INS1_TRI_Pos 4
#define KX134_1211_INS1_TRI     (1 << KX134_1211_INS1_TRI_Pos)
// TLE: X Negative (X-) Reported
#define KX134_1211_INS1_TLE_Pos 5
#define KX134_1211_INS1_TLE     (1 << KX134_1211_INS1_TLE_Pos)

//
//  0x17 - INS2
//  Interrupt Source Register 2
//
#define KX134_1211_REG_INS2     0x17

// TPS: Tilt Position Status
#define KX134_1211_INS2_TPS_Pos         0
#define KX134_1211_INS2_TPS             (1 << KX134_1211_INS2_TPS_Pos)
// TDTS: Tap/Double-Tap Status
#define KX134_1211_INS2_TDTS_Pos        2
#define KX134_1211_INS2_TDTS_Msk        (0x3 << KX134_1211_INS2_TDTS_Pos)
#define KX134_1211_INS2_TDTS(x)         (((x) << KX134_1211_INS2_TDTS_Pos) & \
                                            KX134_1211_INS2_TDTS_Msk)
#define KX134_1211_INS2_TDTS_NO_TAP_Val 0b00
#define KX134_1211_INS2_TDTS_NO_TAP     \
                        KX134_1211_INS2_TDTS(KX134_1211_INS2_TDTS_NO_TAP_Val)
#define KX134_1211_INS2_TDTS_SINGLE_TAP_Val 0b01
#define KX134_1211_INS2_TDTS_SINGLE_TAP \
                    KX134_1211_INS2_TDTS(KX134_1211_INS2_TDTS_SINGLE_TAP_Val)
#define KX134_1211_INS2_TDTS_DOUBLE_TAP_Val 0b10
#define KX134_1211_INS2_TDTS_DOUBLE_TAP \
                    KX134_1211_INS2_TDTS(KX134_1211_INS2_TDTS_DOUBLE_TAP_Val)
// DRDY: Data Ready
#define KX134_1211_INS2_DRDY_Pos        4
#define KX134_1211_INS2_DRDY            (1 << KX134_1211_INS2_DRDY_Pos)
// WMI: Watermark
#define KX134_1211_INS2_WMI_Pos         5
#define KX134_1211_INS2_WMI             (1 << KX134_1211_INS2_WMI_Pos)
// BFI: Buffer Full
#define KX134_1211_INS2_BFI_Pos         6
#define KX134_1211_INS2_BFI             (1 << KX134_1211_INS2_BFI_Pos)
// FFS: Free Fall Status
#define KX134_1211_INS2_FFS_Pos         7
#define KX134_1211_INS2_FFS             (1 << KX134_1211_INS2_FFS_Pos)

//
//  0x18 - INS3
//  Interrupt Source Register 3
//
#define KX134_1211_REG_INS3     0x18

// ZPWU: Z Positive (Z+) Reported
#define KX134_1211_INS3_ZPWU_Pos    0
#define KX134_1211_INS3_ZPWU        (1 << KX134_1211_INS3_ZPWU_Pos)
// ZNWU: Z Negative (Z-) Reported
#define KX134_1211_INS3_ZNWU_Pos    1
#define KX134_1211_INS3_ZNWU        (1 << KX134_1211_INS3_ZNWU_Pos)
// YPWU: Y Positive (Y+) Reported
#define KX134_1211_INS3_YPWU_Pos    2
#define KX134_1211_INS3_YPWU        (1 << KX134_1211_INS3_YPWU_Pos)
// YNWU: Y Negative (Y-) Reported
#define KX134_1211_INS3_YNWU_Pos    3
#define KX134_1211_INS3_YNWU        (1 << KX134_1211_INS3_YNWU_Pos)
// XPWU: X Positive (X+) Reported
#define KX134_1211_INS3_XPWU_Pos    4
#define KX134_1211_INS3_XPWU        (1 << KX134_1211_INS3_XPWU_Pos)
// XNWU: X Negative (X-) Reported
#define KX134_1211_INS3_XNWU_Pos    5
#define KX134_1211_INS3_XNWU        (1 << KX134_1211_INS3_XNWU_Pos)
// BTS: Back to Sleep
#define KX134_1211_INS3_BTS_Pos     6
#define KX134_1211_INS3_BTS         (1 << KX134_1211_INS3_BTS_Pos)
// WUFS: Wake Up from Sleep
#define KX134_1211_INS3_WUFS_Pos    7
#define KX134_1211_INS3_WUFS        (1 << KX134_1211_INS3_WUFS_Pos)




//
//  0x19 - STATUS_REG
//  Status Register
//
#define KX134_1211_REG_STATUS_REG   0x19

// WAKE: Wake/Back to Sleep State
#define KX134_1211_STATUS_REG_WAKE_Pos  0
#define KX134_1211_STATUS_REG_WAKE      (1 << KX134_1211_STATUS_REG_WAKE_Pos)
// INT: Combined (OR) Interrupt Information
#define KX134_1211_STATUS_REG_INT_Pos   4
#define KX134_1211_STATUS_REG_INT       (1 << KX134_1211_STATUS_REG_INT_Pos)




//
//  0x1a - INT_REL
//  Interrupt Latch Release
//
#define KX134_1211_REG_INT_REL  0x1a




//
//  0x1b - CNTL1
//  Control Register 1
//
#define KX134_1211_REG_CNTL1    0x1b
#define KX134_1211_REG_CNTL1_RST_VAL    0b00000000

// TPE: Tilt Position Engine (TPE) Enable
#define KX134_1211_CNTL1_TPW_Pos    0
#define KX134_1211_CNTL1_TPW        (1 << KX134_1211_CNTL1_TPW_Pos)
// TDTE: Tap/Double-Tap Engine (TDTE) Enable
#define KX134_1211_CNTL1_TDTE_Pos   2
#define KX134_1211_CNTL1_TDTE       (1 << KX134_1211_CNTL1_TDTE_Pos)
// GSEL: G-range Select
#define KX134_1211_CNTL1_GSEL_Pos        3
#define KX134_1211_CNTL1_GSEL_Msk        (0x3 << KX134_1211_CNTL1_GSEL_Pos)
#define KX134_1211_CNTL1_GSEL(x)         (((x) << KX134_1211_CNTL1_GSEL_Pos) \
                                            & KX134_1211_CNTL1_GSEL_Msk)
#define KX134_1211_CNTL1_GSEL_8G_Val    0b00
#define KX134_1211_CNTL1_GSEL_8G        KX134_1211_CNTL1_GSEL(KX134_1211_CNTL1_GSEL_8G_Val)
#define KX134_1211_CNTL1_GSEL_16G_Val   0b01
#define KX134_1211_CNTL1_GSEL_16G       KX134_1211_CNTL1_GSEL(KX134_1211_CNTL1_GSEL_16G_Val)
#define KX134_1211_CNTL1_GSEL_32G_Val   0b10
#define KX134_1211_CNTL1_GSEL_32G       KX134_1211_CNTL1_GSEL(KX134_1211_CNTL1_GSEL_32G_Val)
#define KX134_1211_CNTL1_GSEL_64G_Val   0b11
#define KX134_1211_CNTL1_GSEL_64G       KX134_1211_CNTL1_GSEL(KX134_1211_CNTL1_GSEL_64G_Val)
// DRDYE: Data Ready Engine Enable
#define KX134_1211_CNTL1_DRDYE_Pos  5
#define KX134_1211_CNTL1_DRDYE      (1 << KX134_1211_CNTL1_DRDYE_Pos)
// RES: Performance Mode
#define KX134_1211_CNTL1_RES_Pos    6
#define KX134_1211_CNTL1_RES        (1 << KX134_1211_CNTL1_RES_Pos)
#define KX134_1211_CNTL1_RES_HIGH   (1 << KX134_1211_CNTL1_RES_Pos)
#define KX134_1211_CNTL1_RES_LOW    (0 << KX134_1211_CNTL1_RES_Pos)
// PC1: Operating Mode
#define KX134_1211_CNTL1_PC1_Pos        7
#define KX134_1211_CNTL1_PC1            (1 << KX134_1211_CNTL1_PC1_Pos)
#define KX134_1211_CNTL1_PC1_ACTIVE     (1 << KX134_1211_CNTL1_PC1_Pos)
#define KX134_1211_CNTL1_PC1_STANDBY    (0 << KX134_1211_CNTL1_PC1_Pos)

//
//  0x1c - CNTL2
//  Control Register 2
//
#define KX134_1211_REG_CNTL2    0x1c
#define KX134_1211_REG_CNTL2_RST_VAL    0b00111111

// FUM: Face-Up state enable (Z+)
#define KX134_1211_CNTL2_FUM_Pos    0
#define KX134_1211_CNTL2_FUM        (1 << KX134_1211_CNTL2_FUM_Pos)
// FDM: Face-Down state enable (Z-)
#define KX134_1211_CNTL2_FDM_Pos    1
#define KX134_1211_CNTL2_FDM        (1 << KX134_1211_CNTL2_FDM_Pos)
// UPM: Up state enable (Y+)
#define KX134_1211_CNTL2_UPM_Pos    2
#define KX134_1211_CNTL2_UPM        (1 << KX134_1211_CNTL2_UPM_Pos)
// DOM: Down state enable (Y-)
#define KX134_1211_CNTL2_DOM_Pos    3
#define KX134_1211_CNTL2_DOM        (1 << KX134_1211_CNTL2_DOM_Pos)
// RIM: Right state enable (X+)
#define KX134_1211_CNTL2_RIM_Pos    4
#define KX134_1211_CNTL2_RIM        (1 << KX134_1211_CNTL2_RIM_Pos)
// LEM: Left state enable (X-)
#define KX134_1211_CNTL2_LEM_Pos    5
#define KX134_1211_CNTL2_LEM        (1 << KX134_1211_CNTL2_LEM_Pos)
// COTC: Command Test Control
#define KX134_1211_CNTL2_COTC_Pos   6
#define KX134_1211_CNTL2_COTC       (1 << KX134_1211_CNTL2_COTC_Pos)
// SRST: Software Reset
#define KX134_1211_CNTL2_SRST_Pos   7
#define KX134_1211_CNTL2_SRST       (1 << KX134_1211_CNTL2_SRST_Pos)

//
//  0x1d - CNTL3
//  Control Register 3
//
#define KX134_1211_REG_CNTL3    0x1d
#define KX134_1211_REG_CNTL3_RST_VAL    0b10101000

// OWUF: ODR for Wake-Up Function
#define KX134_1211_CNTL3_OWUF_Pos   0
#define KX134_1211_CNTL3_OWUF_Msk   (0x7 << KX134_1211_CNTL3_OWUF_Pos)
#define KX134_1211_CNTL3_OWUF(x)    (((x) << KX134_1211_CNTL3_OWUF_Pos) & \
                                        KX134_1211_CNTL3_OWUF_Msk)
#define KX134_1211_CNTL3_OWUF_0_781_Val 0b000
#define KX134_1211_CNTL3_OWUF_0_781 KX134_1211_CNTL3_OWUF(KX134_1211_CNTL3_OWUF_0_781_Val)
#define KX134_1211_CNTL3_OWUF_1_563_Val 0b001
#define KX134_1211_CNTL3_OWUF_1_563 KX134_1211_CNTL3_OWUF(KX134_1211_CNTL3_OWUF_1_563_Val)
#define KX134_1211_CNTL3_OWUF_3_125_Val 0b010
#define KX134_1211_CNTL3_OWUF_3_125 KX134_1211_CNTL3_OWUF(KX134_1211_CNTL3_OWUF_3_125_Val)
#define KX134_1211_CNTL3_OWUF_6_25_Val  0b011
#define KX134_1211_CNTL3_OWUF_6_25  KX134_1211_CNTL3_OWUF(KX134_1211_CNTL3_OWUF_6_25_Val)
#define KX134_1211_CNTL3_OWUF_12_5_Val  0b100
#define KX134_1211_CNTL3_OWUF_12_5  KX134_1211_CNTL3_OWUF(KX134_1211_CNTL3_OWUF_12_5_Val)
#define KX134_1211_CNTL3_OWUF_25_Val    0b101
#define KX134_1211_CNTL3_OWUF_25    KX134_1211_CNTL3_OWUF(KX134_1211_CNTL3_OWUF_25_Val)
#define KX134_1211_CNTL3_OWUF_50_Val    0b110
#define KX134_1211_CNTL3_OWUF_50    KX134_1211_CNTL3_OWUF(KX134_1211_CNTL3_OWUF_50_Val)
#define KX134_1211_CNTL3_OWUF_100_Val   0b111
#define KX134_1211_CNTL3_OWUF_100   KX134_1211_CNTL3_OWUF(KX134_1211_CNTL3_OWUF_100_Val)
// OTDT: ODR for Tap/Double-Tap
#define KX134_1211_CNTL3_OTDT_Pos   3
#define KX134_1211_CNTL3_OTDT_Msk   (0x7 << KX134_1211_CNTL3_OTDT_Pos)
#define KX134_1211_CNTL3_OTDT(x)    (((x) << KX134_1211_CNTL3_OTDT_Pos) & \
                                        KX134_1211_CNTL3_OTDT_Msk)
#define KX134_1211_CNTL3_OTDT_12_5_Val  0b000
#define KX134_1211_CNTL3_OTDT_12_5  KX134_1211_CNTL3_OTDT(KX134_1211_CNTL3_OTDT_12_5_Val)
#define KX134_1211_CNTL3_OTDT_25_Val    0b001
#define KX134_1211_CNTL3_OTDT_25    KX134_1211_CNTL3_OTDT(KX134_1211_CNTL3_OTDT_25_Val)
#define KX134_1211_CNTL3_OTDT_50_Val    0b010
#define KX134_1211_CNTL3_OTDT_50    KX134_1211_CNTL3_OTDT(KX134_1211_CNTL3_OTDT_50_Val)
#define KX134_1211_CNTL3_OTDT_100_Val   0b011
#define KX134_1211_CNTL3_OTDT_100   KX134_1211_CNTL3_OTDT(KX134_1211_CNTL3_OTDT_100_Val)
#define KX134_1211_CNTL3_OTDT_200_Val   0b100
#define KX134_1211_CNTL3_OTDT_200   KX134_1211_CNTL3_OTDT(KX134_1211_CNTL3_OTDT_200_Val)
#define KX134_1211_CNTL3_OTDT_400_Val   0b101
#define KX134_1211_CNTL3_OTDT_400   KX134_1211_CNTL3_OTDT(KX134_1211_CNTL3_OTDT_400_Val)
#define KX134_1211_CNTL3_OTDT_800_Val   0b110
#define KX134_1211_CNTL3_OTDT_800   KX134_1211_CNTL3_OTDT(KX134_1211_CNTL3_OTDT_800_Val)
#define KX134_1211_CNTL3_OTDT_1600_Val  0b111
#define KX134_1211_CNTL3_OTDT_1600  KX134_1211_CNTL3_OTDT(KX134_1211_CNTL3_OTDT_1600_Val)
// OTP: ODR for Tilt Position
#define KX134_1211_CNTL3_OTP_Pos    6
#define KX134_1211_CNTL3_OTP_Msk    (0x3 << KX134_1211_CNTL3_OTP_Pos)
#define KX134_1211_CNTL3_OTP(x)     (((x) << KX134_1211_CNTL3_OTP_Pos) & \
                                        KX134_1211_CNTL3_OTP)
#define KX134_1211_CNTL3_OTP_1_563_Val  0b00
#define KX134_1211_CNTL3_OTP_1_563  KX134_1211_CNTL3_OTP(KX134_1211_CNTL3_OTP_1_563_Val)
#define KX134_1211_CNTL3_OTP_6_25_Val   0b01
#define KX134_1211_CNTL3_OTP_6_25   KX134_1211_CNTL3_OTP(KX134_1211_CNTL3_OTP_6_25_Val)
#define KX134_1211_CNTL3_OTP_12_5_Val   0b10
#define KX134_1211_CNTL3_OTP_12_5   KX134_1211_CNTL3_OTP(KX134_1211_CNTL3_OTP_12_5_Val)
#define KX134_1211_CNTL3_OTP_50_Val     0b11
#define KX134_1211_CNTL3_OTP_50     KX134_1211_CNTL3_OTP(KX134_1211_CNTL3_OTP_50_Val)

//
//  0x1e - CNTL4
//  Control Register 4
//
#define KX134_1211_REG_CNTL4    0x1e
#define KX134_1211_REG_CNTL4_RST_VAL    0b01000000

// OBTS: ODR for Back-to-Sleep (Motion Detection)
#define KX134_1211_CNTL4_OBTS_Pos    0
#define KX134_1211_CNTL4_OBTS_Msk    (0x7 << KX134_1211_CNTL4_OBTS_Pos)
#define KX134_1211_CNTL4_OBTS(x)     (((x) << KX134_1211_CNTL4_OBTS_Pos) & \
                                        KX134_1211_CNTL4_OBTS_Msk)
#define KX134_1211_CNTL4_OBTS_0_781_Val 0b000
#define KX134_1211_CNTL4_OBTS_0_781 KX134_1211_CNTL4_OBTS(KX134_1211_CNTL4_OBTS_0_781_Val)
#define KX134_1211_CNTL4_OBTS_1_563_Val 0b001
#define KX134_1211_CNTL4_OBTS_1_563 KX134_1211_CNTL4_OBTS(KX134_1211_CNTL4_OBTS_1_563_Val)
#define KX134_1211_CNTL4_OBTS_3_125_Val 0b010
#define KX134_1211_CNTL4_OBTS_3_125 KX134_1211_CNTL4_OBTS(KX134_1211_CNTL4_OBTS_3_125_Val)
#define KX134_1211_CNTL4_OBTS_6_25_Val  0b011
#define KX134_1211_CNTL4_OBTS_6_25  KX134_1211_CNTL4_OBTS(KX134_1211_CNTL4_OBTS_6_25_Val)
#define KX134_1211_CNTL4_OBTS_12_5_Val  0b100
#define KX134_1211_CNTL4_OBTS_12_5  KX134_1211_CNTL4_OBTS(KX134_1211_CNTL4_OBTS_12_5_Val)
#define KX134_1211_CNTL4_OBTS_25_Val    0b101
#define KX134_1211_CNTL4_OBTS_25    KX134_1211_CNTL4_OBTS(KX134_1211_CNTL4_OBTS_25_Val)
#define KX134_1211_CNTL4_OBTS_50_Val    0b110
#define KX134_1211_CNTL4_OBTS_50    KX134_1211_CNTL4_OBTS(KX134_1211_CNTL4_OBTS_50_Val)
#define KX134_1211_CNTL4_OBTS_100_Val   0b111
#define KX134_1211_CNTL4_OBTS_100   KX134_1211_CNTL4_OBTS(KX134_1211_CNTL4_OBTS_100_Val)
// PR_MODE: Pulse Reject Mode
#define KX134_1211_CNTL4_PR_MODE_Pos    3
#define KX134_1211_CNTL4_PR_MODE        (1 << KX134_1211_CNTL4_PR_MODE_Pos)
#define KX134_1211_CNTL4_PR_MODE_REJ_PULSE  (1 << KX134_1211_CNTL4_PR_MODE_Pos)
#define KX134_1211_CNTL4_PR_MODE_STANDARD   (0 << KX134_1211_CNTL4_PR_MODE_Pos)
// BTSE: Back-to-Sleep Engine Enable
#define KX134_1211_CNTL4_BTSE_Pos       4
#define KX134_1211_CNTL4_BTSE           (1 << KX134_1211_CNTL4_BTSE_Pos)
// WUFE: Wake-Up Function Engine Enable
#define KX134_1211_CNTL4_WUFE_Pos       5
#define KX134_1211_CNTL4_WUFE           (1 << KX134_1211_CNTL4_WUFE_Pos)
// TH_MODE: Wake/Back-to-Sleep Threshold Mode
#define KX134_1211_CNTL4_TH_MODE_Pos    6
#define KX134_1211_CNTL4_TH_MODE        (1 << KX134_1211_CNTL4_TH_MODE_Pos)
#define KX134_1211_CNTL4_TH_MODE_REL    (1 << KX134_1211_CNTL4_TH_MODE_Pos)
#define KX134_1211_CNTL4_TH_MODE_ABS    (0 << KX134_1211_CNTL4_TH_MODE_Pos)
// C_MODE: Wake-Up Function Engine Enable
#define KX134_1211_CNTL4_C_MODE_Pos     7
#define KX134_1211_CNTL4_C_MODE         (1 << KX134_1211_CNTL4_C_MODE_Pos)
#define KX134_1211_CNTL4_C_MODE_DEC     (1 << KX134_1211_CNTL4_C_MODE_Pos)
#define KX134_1211_CNTL4_C_MODE_RST     (0 << KX134_1211_CNTL4_C_MODE_Pos)

//
//  0x1f - CNTL5
//  Control Register 5
//
#define KX134_1211_REG_CNTL5    0x1f
#define KX134_1211_REG_CNTL5_RST_VAL    0b00000000

// MAN_SLEEP: Manual Wake-Sleep Engine Overwrite
#define KX134_1211_CNTL5_MAN_SLEEP_Pos  0
#define KX134_1211_CNTL5_MAN_SLEEP      (1 << KX134_1211_CNTL5_MAN_SLEEP_Pos)
// MAN_WAKE: Manual Wake-Sleep Engine Overwrite
#define KX134_1211_CNTL5_MAN_WAKE_Pos   1
#define KX134_1211_CNTL5_MAN_WAKE       (1 << KX134_1211_CNTL5_MAN_WAKE_Pos)
// ADPE: Advanced Data Path (ADP) Enable
#define KX134_1211_CNTL5_ADPE_Pos       4
#define KX134_1211_CNTL5_ADPE           (1 << KX134_1211_CNTL5_ADPE_Pos)

//
//  0x20 - CNTL6
//  Control Register 6
//
#define KX134_1211_REG_CNTL6    0x20
#define KX134_1211_REG_CNTL6_RST_VAL    0b00000000

// I2C_ALC:  I2C Auto Release Function Counter Select
#define KX134_1211_CNTL6_I2C_ALC_Pos    0
#define KX134_1211_CNTL6_I2C_ALC_Msk    (0x3 << KX134_1211_CNTL6_I2C_ALC_Pos)
#define KX134_1211_CNTL6_I2C_ALC(x)     (((x) << KX134_1211_CNTL6_I2C_ALC_Pos) \
                                            & KX134_1211_CNTL6_I2C_ALC_Msk)
#define KX134_1211_CNTL6_I2C_ALC_0_5S_Val   0b00
#define KX134_1211_CNTL6_I2C_ALC_0_5S  \
                    KX134_1211_CNTL6_I2C_ALC(KX134_1211_CNTL6_I2C_ALC_0_5S_Val)
#define KX134_1211_CNTL6_I2C_ALC_1S_Val     0b01
#define KX134_1211_CNTL6_I2C_ALC_1S KX134_1211_CNTL6_I2C_ALC(KX134_1211_CNTL6_I2C_ALC_1S_Val)
#define KX134_1211_CNTL6_I2C_ALC_2S_Val     0b10
#define KX134_1211_CNTL6_I2C_ALC_2S KX134_1211_CNTL6_I2C_ALC(KX134_1211_CNTL6_I2C_ALC_2S_Val)
#define KX134_1211_CNTL6_I2C_ALC_4S_Val     0b11
#define KX134_1211_CNTL6_I2C_ALC_4S KX134_1211_CNTL6_I2C_ALC(KX134_1211_CNTL6_I2C_ALC_4S_Val)
// I2C_ALE: I2C Auto Release Function Enable
#define KX134_1211_CNTL6_I2C_ALE_Pos    7
#define KX134_1211_CNTL6_I2C_ALE        (1 << KX134_1211_CNTL6_I2C_ALE_Pos)




//
//  0x21 - ODCNTL
//  Output Data Control Register
//
#define KX134_1211_REG_ODCNTL   0x21
#define KX134_1211_REG_ODCNTL_RST_VAL   0b00000110

// OSA: ODR for Accelerometer
#define KX134_1211_ODCNTL_OSA_Pos 0
#define KX134_1211_ODCNTL_OSA_Msk (0xf << KX134_1211_ODCNTL_OSA_Pos)
#define KX134_1211_ODCNTL_OSA(x)  (((x) << KX134_1211_ODCNTL_OSA_Pos) & \
                                            KX134_1211_ODCNTL_OSA_Msk)
#define KX134_1211_ODCNTL_OSA_0_781_Val 0b0000
#define KX134_1211_ODCNTL_OSA_0_781     KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_0_781_Val)
#define KX134_1211_ODCNTL_OSA_1_563_Val 0b0001
#define KX134_1211_ODCNTL_OSA_1_563     KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_1_563_Val)
#define KX134_1211_ODCNTL_OSA_3_125_Val 0b0010
#define KX134_1211_ODCNTL_OSA_3_125     KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_3_125_Val)
#define KX134_1211_ODCNTL_OSA_6_25_Val  0b0011
#define KX134_1211_ODCNTL_OSA_6_25      KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_6_25_Val)
#define KX134_1211_ODCNTL_OSA_12_5_Val  0b0100
#define KX134_1211_ODCNTL_OSA_12_5      KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_12_5_Val)
#define KX134_1211_ODCNTL_OSA_25_Val    0b0101
#define KX134_1211_ODCNTL_OSA_25        KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_25_Val)
#define KX134_1211_ODCNTL_OSA_50_Val    0b0110
#define KX134_1211_ODCNTL_OSA_50        KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_50_Val)
#define KX134_1211_ODCNTL_OSA_100_Val   0b0111
#define KX134_1211_ODCNTL_OSA_100       KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_100_Val)
#define KX134_1211_ODCNTL_OSA_200_Val   0b1000
#define KX134_1211_ODCNTL_OSA_200       KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_200_Val)
#define KX134_1211_ODCNTL_OSA_400_Val   0b1001
#define KX134_1211_ODCNTL_OSA_400       KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_400_Val)
#define KX134_1211_ODCNTL_OSA_800_Val   0b1010
#define KX134_1211_ODCNTL_OSA_800       KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_800_Val)
#define KX134_1211_ODCNTL_OSA_1600_Val  0b1011
#define KX134_1211_ODCNTL_OSA_1600      KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_1600_Val)
#define KX134_1211_ODCNTL_OSA_3200_Val  0b1100
#define KX134_1211_ODCNTL_OSA_3200      KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_3200_Val)
#define KX134_1211_ODCNTL_OSA_6400_Val  0b1101
#define KX134_1211_ODCNTL_OSA_6400      KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_6400_Val)
#define KX134_1211_ODCNTL_OSA_12800_Val 0b1110
#define KX134_1211_ODCNTL_OSA_12800     KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_12800_Val)
#define KX134_1211_ODCNTL_OSA_25600_Val 0b1111
#define KX134_1211_ODCNTL_OSA_25600     KX134_1211_ODCNTL_OSA(KX134_1211_ODCNTL_OSA_25600_Val)
// FSTUP: Fast Start Up Enable
#define KX134_1211_ODCNTL_FSTUP_Pos     5
#define KX134_1211_ODCNTL_FSTUP         (1 << KX134_1211_ODCNTL_FSTUP_Pos)
// LPRO: Low-Pass filter Roll-Off ontrol
#define KX134_1211_ODCNTL_LPRO_Pos      6
#define KX134_1211_ODCNTL_LPRO          (1 << KX134_1211_ODCNTL_LPRO_Pos)
#define KX134_1211_ODCNTL_LPRO_IR_CFF_ODR_2     (1 << KX134_1211_ODCNTL_LPRO_Pos)
#define KX134_1211_ODCNTL_LPRO_IR_CFF_ODR_9     (0 << KX134_1211_ODCNTL_LPRO_Pos)




//
//  0x22 - INC1
//  Interrupt Control Register 1 (settings for interrupt pin 1)
//
#define KX134_1211_REG_INC1     0x22
#define KX134_1211_REG_INC1_RST_VAL     0b00010000

// SPI3E: 3-wire SPI Interface Enable
#define KX134_1211_INC1_SPI3E_Pos   0
#define KX134_1211_INC1_SPI3E       (1 << KX134_1211_INC1_SPI3E_Pos)
// STPOL: Self Test Polarity
#define KX134_1211_INC1_STPOL_Pos   1
#define KX134_1211_INC1_STPOL       (1 << KX134_1211_INC1_STPOL_Pos)
#define KX134_1211_INC1_STPOL_NEGATIVE  (1 << KX134_1211_INC1_STPOL_Pos)
#define KX134_1211_INC1_STPOL_POSITIVE  (0 << KX134_1211_INC1_STPOL_Pos)
// IEL1:  Interrupt Latch Control for Interrupt Pin 1
#define KX134_1211_INC1_IEL1_Pos    3
#define KX134_1211_INC1_IEL1        (1 << KX134_1211_INC1_IEL1_Pos)
#define KX134_1211_INC1_IEL1_PULSED     (1 << KX134_1211_INC1_IEL1_Pos)
#define KX134_1211_INC1_IEL1_LATCHED    (0 << KX134_1211_INC1_IEL1_Pos)
// IEA1: Interrupt Active Level Control for Interrupt Pin 1
#define KX134_1211_INC1_IEA1_Pos    4
#define KX134_1211_INC1_IEA1        (1 << KX134_1211_INC1_IEA1_Pos)
#define KX134_1211_INC1_IEA1_HIGH   (1 << KX134_1211_INC1_IEA1_Pos)
#define KX134_1211_INC1_IEA1_LOW    (0 << KX134_1211_INC1_IEA1_Pos)
// IEN1: Interrupt Pin 1 Enable
#define KX134_1211_INC1_IEN1_Pos    5
#define KX134_1211_INC1_IEN1        (1 << KX134_1211_INC1_IEN1_Pos)
// PW1: Interrupt Pin 1 Pulse Width Configuration
#define KX134_1211_INC1_PW1_Pos     6
#define KX134_1211_INC1_PW1_Msk     (0x3 << KX134_1211_INC1_PW1_Pos)
#define KX134_1211_INC1_PW1(x)      (((x) << KX134_1211_INC1_PW1_Pos) & \
                                        KX134_1211_INC1_PW1_Msk)
#define KX134_1211_INC1_PW1_50U_Val     0b00
#define KX134_1211_INC1_PW1_50U         KX134_1211_INC1_PW1(KX134_1211_INC1_PW1_50U_Val)
#define KX134_1211_INC1_PW1_1OSA_Val    0b01
#define KX134_1211_INC1_PW1_1OSA        KX134_1211_INC1_PW1(KX134_1211_INC1_PW1_1OSA_Val)
#define KX134_1211_INC1_PW1_2OSA_Val    0b10
#define KX134_1211_INC1_PW1_2OSA        KX134_1211_INC1_PW1(KX134_1211_INC1_PW1_2OSA_Val)
#define KX134_1211_INC1_PW1_RT_Val      0b10
#define KX134_1211_INC1_PW1_RT          KX134_1211_INC1_PW1(KX134_1211_INC1_PW1_RT_Val)

//
//  0x23 - INC2
//  Interrupt Control Register 2 (interrupt behavior for WUF and BTS engines)
//
#define KX134_1211_REG_INC2     0x23
#define KX134_1211_REG_INC2_RST_VAL     0b00111111

// ZPWUE: Z positive (Z+) mask for WUF and BTS
#define KX134_1211_INC2_ZPWUE_Pos   0
#define KX134_1211_INC2_ZPWUE       (1 << KX134_1211_INC2_ZPWUE_Pos)
// ZNWUE: Z negative (Z-) mask for WUF and BTS
#define KX134_1211_INC2_ZNWUE_Pos   1
#define KX134_1211_INC2_ZNWUE       (1 << KX134_1211_INC2_ZNWUE_Pos)
// YPWUE: Y positive (Y+) mask for WUF and BTS
#define KX134_1211_INC2_YPWUE_Pos   2
#define KX134_1211_INC2_YPWUE       (1 << KX134_1211_INC2_YPWUE_Pos)
// YNWUE: Y negative (Y-) mask for WUF and BTS
#define KX134_1211_INC2_YNWUE_Pos   3
#define KX134_1211_INC2_YNWUE       (1 << KX134_1211_INC2_YNWUE_Pos)
// XPWUE: X positive (X+) mask for WUF and BTS
#define KX134_1211_INC2_XPWUE_Pos   4
#define KX134_1211_INC2_XPWUE       (1 << KX134_1211_INC2_XPWUE_Pos)
// XNWUE: X negative (X-) mask for WUF and BTS
#define KX134_1211_INC2_XNWUE_Pos   5
#define KX134_1211_INC2_XNWUE       (1 << KX134_1211_INC2_XNWUE_Pos)
// AOI: AND-OR Configuration for Motion Detection Axes
#define KX134_1211_INC2_AOI_Pos     6
#define KX134_1211_INC2_AOI         (1 << KX134_1211_INC2_AOI_Pos)
#define KX134_1211_INC2_AOI_AND     (1 << KX134_1211_INC2_AOI_Pos)
#define KX134_1211_INC2_AOI_OR      (0 << KX134_1211_INC2_AOI_Pos)

//
//  0x24 - INC3
//  Interrupt Control Register 3 (tap/double interrupt control)
//
#define KX134_1211_REG_INC3     0x24
#define KX134_1211_REG_INC3_RST_VAL     0b00111111

// TFUM: Tap Face Up (Z+) State Mask
#define KX134_1211_INC3_TFUM_Pos    0
#define KX134_1211_INC3_TFUM        (1 << KX134_1211_INC3_TFUM_Pos)
// TFDM: Tap Face Down (Z-) State Mask
#define KX134_1211_INC3_TFDM_Pos    1
#define KX134_1211_INC3_TFDM        (1 << KX134_1211_INC3_TFDM_Pos)
// TUPM: Tap Up (Y+) State Mask
#define KX134_1211_INC3_TUPM_Pos    2
#define KX134_1211_INC3_TUPM        (1 << KX134_1211_INC3_TUPM_Pos)
// TDOM: Tap Down (Y-) State Mask
#define KX134_1211_INC3_TDOM_Pos    3
#define KX134_1211_INC3_TDOM        (1 << KX134_1211_INC3_TDOM_Pos)
// TRIM: Tap Right (X+) state mask
#define KX134_1211_INC3_TRIM_Pos    4
#define KX134_1211_INC3_TRIM        (1 << KX134_1211_INC3_TRIM_Pos)
// TLEM: Tap Left (X-) state mask
#define KX134_1211_INC3_TLEM_Pos    5
#define KX134_1211_INC3_TLEM        (1 << KX134_1211_INC3_TLEM_Pos)
// TMEN: Alternate Tap Masking Scheme Enable
#define KX134_1211_INC3_TMEN_Pos    6
#define KX134_1211_INC3_TMEN        (1 << KX134_1211_INC3_TMEN_Pos)

//
//  0x25 - INC4
//  Interrupt Control Register 4 (interrupt routing for pin 1)
//
#define KX134_1211_REG_INC4     0x25
#define KX134_1211_REG_INC4_RST_VAL     0b00000000

// TPI1: Tilt Position Interrupt Reported on Interrupt Pin 1
#define KX134_1211_INC4_TPI1_Pos    0
#define KX134_1211_INC4_TPI1        (1 << KX134_1211_INC4_TPI1_Pos)
// WUFI1: Wake-Up (motion detect) Interrupt Reported on Interrupt Pin 1
#define KX134_1211_INC4_WUFI1_Pos   1
#define KX134_1211_INC4_WUFI1       (1 << KX134_1211_INC4_WUFI1_Pos)
// TDTI1: Tap/Double Tap Interrupt Reported on Interrupt Pin 1
#define KX134_1211_INC4_TDTI1_Pos   2
#define KX134_1211_INC4_TDTI1       (1 << KX134_1211_INC4_TDTI1_Pos)
// BTSI1: Back to Sleep Interrupt Reported on Interrupt Pin 1
#define KX134_1211_INC4_BTSI1_Pos   3
#define KX134_1211_INC4_BTSI1       (1 << KX134_1211_INC4_BTSI1_Pos)
// DRDYI1: Data Ready Interrupt Reported on Interrupt Pin 1
#define KX134_1211_INC4_DRDYI1_Pos  4
#define KX134_1211_INC4_DRDYI1      (1 << KX134_1211_INC4_DRDYI1_Pos)
// WMI1: Watermark Interrupt Reported on Interrupt Pin 1
#define KX134_1211_INC4_WMI1_Pos    5
#define KX134_1211_INC4_WMI1        (1 << KX134_1211_INC4_WMI1_Pos)
// BFI1: Buffer Full Interrupt Reported on Interrupt Pin 1
#define KX134_1211_INC4_BFI1_Pos    6
#define KX134_1211_INC4_BFI1        (1 << KX134_1211_INC4_BFI1_Pos)
// FFI1: Free Fall Interrupt Reported on Interrupt Pin 1
#define KX134_1211_INC4_FFI1_Pos    7
#define KX134_1211_INC4_FFI1        (1 << KX134_1211_INC4_FFI1_Pos)

//
//  0x26 - INC5
//  Interrupt Control Register 5 (settings for interrupt pin 2)
//
#define KX134_1211_REG_INC5     0x26
#define KX134_1211_REG_INC5_RST_VAL     0b00010000

// ACLR1: Auto Clear INS1-INS3 at Pulse Interrupt 1 Trailing Edge
#define KX134_1211_INC5_ACLR1_Pos   0
#define KX134_1211_INC5_ACLR1       (1 << KX134_1211_INC5_ACLR1_Pos)
// ACLR2: Auto Clear INS1-INS3 at Pulse Interrupt 2 Trailing Edge
#define KX134_1211_INC5_ACLR2_Pos   1
#define KX134_1211_INC5_ACLR2       (1 << KX134_1211_INC5_ACLR2_Pos)
// IEL2:  Interrupt Latch Control for Interrupt Pin 2
#define KX134_1211_INC5_IEL2_Pos    3
#define KX134_1211_INC5_IEL2        (1 << KX134_1211_INC5_IEL2_Pos)
#define KX134_1211_INC5_IEL2_PULSED     (1 << KX134_1211_INC5_IEL2_Pos)
#define KX134_1211_INC5_IEL2_LATCHED    (0 << KX134_1211_INC5_IEL2_Pos)
// IEA2: Interrupt Active Level Control for Interrupt Pin 2
#define KX134_1211_INC5_IEA2_Pos    4
#define KX134_1211_INC5_IEA2        (1 << KX134_1211_INC5_IEA2_Pos)
#define KX134_1211_INC5_IEA2_HIGH   (1 << KX134_1211_INC5_IEA2_Pos)
#define KX134_1211_INC5_IEA2_LOW    (0 << KX134_1211_INC5_IEA2_Pos)
// IEN2: Interrupt Pin 2 Enable
#define KX134_1211_INC5_IEN2_Pos    5
#define KX134_1211_INC5_IEN2        (1 << KX134_1211_INC5_IEN2_Pos)
// PW2: Interrupt Pin 2 Pulse Width Configuration
#define KX134_1211_INC5_PW2_Pos     6
#define KX134_1211_INC5_PW2_Msk     (0x3 << KX134_1211_INC5_PW2_Pos)
#define KX134_1211_INC5_PW2(x)      (((x) << KX134_1211_INC5_PW2_Pos) & \
                                            KX134_1211_INC5_PW2_Msk)
#define KX134_1211_INC5_PW2_50U_Val     0b00
#define KX134_1211_INC5_PW2_50U         KX134_1211_INC5_PW2(KX134_1211_INC1_PW2_50U_Val)
#define KX134_1211_INC5_PW2_1OSA_Val    0b01
#define KX134_1211_INC5_PW2_1OSA        KX134_1211_INC5_PW2(KX134_1211_INC1_PW2_1OSA_Val)
#define KX134_1211_INC5_PW2_2OSA_Val    0b10
#define KX134_1211_INC5_PW2_2OSA        KX134_1211_INC5_PW2(KX134_1211_INC1_PW2_2OSA_Val)
#define KX134_1211_INC5_PW2_RT_Val      0b10
#define KX134_1211_INC5_PW2_RT          KX134_1211_INC5_PW2(KX134_1211_INC1_PW2_RT_Val)

//
//  0x27 - INC6
//  Interrupt Control Register 6 (interrupt routing for pin 2)
//
#define KX134_1211_REG_INC6     0x27
#define KX134_1211_REG_INC6_RST_VAL     0b00000000

// TPI2: Tilt Position Interrupt Reported on Interrupt Pin 2
#define KX134_1211_INC6_TPI2_Pos    0
#define KX134_1211_INC6_TPI2        (1 << KX134_1211_INC4_TPI2_Pos)
// WUFI2: Wake-Up (motion detect) Interrupt Reported on Interrupt Pin 2
#define KX134_1211_INC6_WUFI2_Pos   1
#define KX134_1211_INC6_WUFI2       (1 << KX134_1211_INC4_WUFI2_Pos)
// TDTI2: Tap/Double Tap Interrupt Reported on Interrupt Pin 2
#define KX134_1211_INC6_TDTI2_Pos   2
#define KX134_1211_INC6_TDTI2       (1 << KX134_1211_INC4_TDTI2_Pos)
// BTSI2: Back to Sleep Interrupt Reported on Interrupt Pin 2
#define KX134_1211_INC6_BTSI2_Pos   3
#define KX134_1211_INC6_BTSI2       (1 << KX134_1211_INC4_BTSI2_Pos)
// DRDYI2: Data Ready Interrupt Reported on Interrupt Pin 2
#define KX134_1211_INC6_DRDYI2_Pos  4
#define KX134_1211_INC6_DRDYI2      (1 << KX134_1211_INC4_DRDYI2_Pos)
// WMI2: Watermark Interrupt Reported on Interrupt Pin 2
#define KX134_1211_INC6_WMI2_Pos    5
#define KX134_1211_INC6_WMI2        (1 << KX134_1211_INC4_WMI2_Pos)
// BFI2: Buffer Full Interrupt Reported on Interrupt Pin 2
#define KX134_1211_INC6_BFI2_Pos    6
#define KX134_1211_INC6_BFI2        (1 << KX134_1211_INC4_BFI2_Pos)
// FFI2: Free Fall Interrupt Reported on Interrupt Pin 2
#define KX134_1211_INC6_FFI2_Pos    7
#define KX134_1211_INC6_FFI2        (1 << KX134_1211_INC4_FFI2_Pos)




//
//  0x29 - TILT_TIMER
//  Initial Count for Tilt Position State Timer
//
#define KX134_1211_REG_TILT_TIMER   0x29
#define KX134_1211_REG_TILT_TIMER_RST_VAL   0b00000000




//
//  0x2a - TDTRC
//  Tap/Double-Tap Report Control
//
#define KX134_1211_REG_TDTRC    0x2a
#define KX134_1211_REG_TDTRC_RST_VAL    0b00000011

// STRE: Single Tap Interrupt Enable
#define KX134_1211_TDTRC_STRE_Pos   0
#define KX134_1211_TDTRC_STRE       (1 << KX134_1211_TDTRC_STRE_Pos)
// DTRE: Double Tap Interrupt Enable
#define KX134_1211_TDTRC_DTRE_Pos   1
#define KX134_1211_TDTRC_DTRE       (1 << KX134_1211_TDTRC_DTRE_Pos)

//
//  0x2b - TDTC
//  Double Tap Event Detection Count
//
#define KX134_1211_REG_TDTC     0x2b
#define KX134_1211_REG_TDTC_RST_VAL     0b01111000

//
//  0x2c - TTH
//  Tap Threshold High
//
#define KX134_1211_REG_TTH      0x2c
#define KX134_1211_REG_TTH_RST_VAL      0b00110011

//
//  0x2d - TTL
//  Tap Threshold Low
//
#define KX134_1211_REG_TTL      0x2d
#define KX134_1211_REG_TTL_RST_VAL      0b00000111

//
//  0x2e - FTD
//  Tap Detection Count
//
#define KX134_1211_REG_FTD      0x2e
#define KX134_1211_REG_FTD_RST_VAL      0b10100010

//
//  0x2f - STD
//  Counter Information for Detection of Double Tap Event
//
#define KX134_1211_REG_STD      0x2f
#define KX134_1211_REG_STD_RST_VAL      0b00100100

//
//  0x30 - TLT
//  Counter Information for Detection of Tap Event
//
#define KX134_1211_REG_TLT      0x30
#define KX134_1211_REG_TLT_RST_VAL      0b00101000

//
//  0x31 - TWS
//  Counter Information for Detection of Single and Double Taps
//
#define KX134_1211_REG_TWS      0x31
#define KX134_1211_REG_TWS_RST_VAL      0b10100000




//
//  0x32 - FFTH
//  Free Fall Threshold
//
#define KX134_1211_REG_FFTH     0x32
#define KX134_1211_REG_FFTH_RST_VAL     0b00000000

//
//  0x33 - FFC
//  Free Fall Counter
//
#define KX134_1211_REG_FFC      0x33
#define KX134_1211_REG_FFC_RST_VAL      0b00000000

//
//  0x34 - FFCNTL
//  Free Fall Control
//
#define KX134_1211_REG_FFCNTL   0x34
#define KX134_1211_REG_FFCNTL_RST_VAL   0b00000000

// OFFI: ODR for Free Fall Engine
#define KX134_1211_FFCNTL_OFFI_Pos  0
#define KX134_1211_FFCNTL_OFFI_Msk  (0x7 << KX134_1211_FFCNTL_OFFI_Pos)
#define KX134_1211_FFCNTL_OFFI(x)   (((x) << KX134_1211_FFCNTL_OFFI_Pos) & \
                                        KX134_1211_FFCNTL_OFFI_Msk)
#define KX134_1211_FFCNTL_OFFI_12_5_Val 0b000
#define KX134_1211_FFCNTL_OFFI_12_5 KX134_1211_FFCNTL_OFFI(KX134_1211_FFCNTL_OFFI_12_5_Val)
#define KX134_1211_FFCNTL_OFFI_25_Val   0b001
#define KX134_1211_FFCNTL_OFFI_25   KX134_1211_FFCNTL_OFFI(KX134_1211_FFCNTL_OFFI_25_Val)
#define KX134_1211_FFCNTL_OFFI_50_Val   0b010
#define KX134_1211_FFCNTL_OFFI_50   KX134_1211_FFCNTL_OFFI(KX134_1211_FFCNTL_OFFI_50_Val)
#define KX134_1211_FFCNTL_OFFI_100_Val  0b011
#define KX134_1211_FFCNTL_OFFI_100  KX134_1211_FFCNTL_OFFI(KX134_1211_FFCNTL_OFFI_100_Val)
#define KX134_1211_FFCNTL_OFFI_200_Val  0b100
#define KX134_1211_FFCNTL_OFFI_200  KX134_1211_FFCNTL_OFFI(KX134_1211_FFCNTL_OFFI_200_Val)
#define KX134_1211_FFCNTL_OFFI_400_Val  0b101
#define KX134_1211_FFCNTL_OFFI_400  KX134_1211_FFCNTL_OFFI(KX134_1211_FFCNTL_OFFI_400_Val)
#define KX134_1211_FFCNTL_OFFI_800_Val  0b110
#define KX134_1211_FFCNTL_OFFI_800  KX134_1211_FFCNTL_OFFI(KX134_1211_FFCNTL_OFFI_800_Val)
#define KX134_1211_FFCNTL_OFFI_1600_Val 0b111
#define KX134_1211_FFCNTL_OFFI_1600 KX134_1211_FFCNTL_OFFI(KX134_1211_FFCNTL_OFFI_1600_Val)
// DCRM: Debounce Methodology
#define KX134_1211_FFCNTL_DCRM_Pos  3
#define KX134_1211_FFCNTL_DCRM      (1 << KX134_1211_FFCNTL_DCRM_Pos)
#define KX134_1211_FFCNTL_DCRM_CNT_UP_RST   (1 << KX134_1211_FFCNTL_DCRM_Pos)
#define KX134_1211_FFCNTL_DCRM_CNT_UP_DWN   (0 << KX134_1211_FFCNTL_DCRM_Pos)
// FFDC: Free Fall Interrupt Delayed Clear Duration for Unlatched Mode
#define KX134_1211_FFCNTL_FFDC_Pos  4
#define KX134_1211_FFCNTL_FFDC_Msk  (0x3 << KX134_1211_FFCNTL_FFDC_Pos)
#define KX134_1211_FFCNTL_FFDC(x)   (((x) << KX134_1211_FFCNTL_FFDC_Pos) & \
                                        KX134_1211_FFCNTL_FFDC_Msk)
#define KX134_1211_FFCNTL_FFDC_0S_Val   0b00
#define KX134_1211_FFCNTL_FFDC_0S   KX134_1211_FFCNTL_FFDC(KX134_1211_FFCNTL_FFDC_0S_Val)
#define KX134_1211_FFCNTL_FFDC_1S_Val   0b01
#define KX134_1211_FFCNTL_FFDC_1S   KX134_1211_FFCNTL_FFDC(KX134_1211_FFCNTL_FFDC_1S_Val)
#define KX134_1211_FFCNTL_FFDC_2S_Val   0b10
#define KX134_1211_FFCNTL_FFDC_2S   KX134_1211_FFCNTL_FFDC(KX134_1211_FFCNTL_FFDC_2S_Val)
#define KX134_1211_FFCNTL_FFDC_4S_Val   0b11
#define KX134_1211_FFCNTL_FFDC_4S   KX134_1211_FFCNTL_FFDC(KX134_1211_FFCNTL_FFDC_4S_Val)
// ULMODE: Free Fall Interrupt Latch Control
#define KX134_1211_FFCNTL_ULMODE_Pos    6
#define KX134_1211_FFCNTL_ULMODE        (1 << KX134_1211_FFCNTL_ULMODE_Pos)
#define KX134_1211_FFCNTL_ULMODE_UNLATCHED  (1 << KX134_1211_FFCNTL_ULMODE_Pos)
#define KX134_1211_FFCNTL_ULMODE_LATCHED    (0 << KX134_1211_FFCNTL_ULMODE_Pos)
// FFIE: Free Fall Engine Enable
#define KX134_1211_FFCNTL_FFIE_Pos  6
#define KX134_1211_FFCNTL_FFIE      (1 << KX134_1211_FFCNTL_FFIE_Pos)




//
//  0x37 - TILT_ANGLE_LL
//  Tilt Angle Low Limit
//
#define KX134_1211_REG_TILT_ANGLE_LL    0x37
#define KX134_1211_REG_TILT_ANGLE_LL_RST_VAL    0b00000011

//
//  0x38 - TILT_ANGLE_HL
//  Tilt Angle High Limit
//
#define KX134_1211_REG_TILT_ANGLE_HL    0x38
#define KX134_1211_REG_TILT_ANGLE_HL_RST_VAL    0b00001011

//
//  0x39 - HYST_SET
//  Tilt Angle Hysteresis Setting
//
#define KX134_1211_REG_HYST_SET         0x39
#define KX134_1211_REG_HYST_SET_RST_VA          0b00010100




//
//  0x3a - LP_CNTL1
//  Low Power Control 1
//
#define KX134_1211_REG_LP_CNTL1     0x3a
#define KX134_1211_REG_LP_CNTL1_RST_VAL 0b01000011

// AVC: Averaging Filter Control
#define KX134_1211_LP_CNTL1_AVC_Pos 4
#define KX134_1211_LP_CNTL1_AVC_Msk (0x7 << KX134_1211_LP_CNTL1_AVC_Pos)
#define KX134_1211_LP_CNTL1_AVC(x)  (((x) << KX134_1211_LP_CNTL1_AVC_Pos) & \
                                        KX134_1211_LP_CNTL1_AVC_Msk)
#define KX134_1211_LP_CNTL1_AVC_NONE_Val    0b000
#define KX134_1211_LP_CNTL1_AVC_NONE    \
                    KX134_1211_LP_CNTL1_AVC(KX134_1211_LP_CNTL1_AVC_NONE_Val)
#define KX134_1211_LP_CNTL1_AVC_2S_Val    0b001
#define KX134_1211_LP_CNTL1_AVC_2S    KX134_1211_LP_CNTL1_AVC(KX134_1211_LP_CNTL1_AVC_2S_Val)
#define KX134_1211_LP_CNTL1_AVC_4S_Val    0b010
#define KX134_1211_LP_CNTL1_AVC_4S    KX134_1211_LP_CNTL1_AVC(KX134_1211_LP_CNTL1_AVC_4S_Val)
#define KX134_1211_LP_CNTL1_AVC_8S_Val    0b011
#define KX134_1211_LP_CNTL1_AVC_8S    KX134_1211_LP_CNTL1_AVC(KX134_1211_LP_CNTL1_AVC_8S_Val)
#define KX134_1211_LP_CNTL1_AVC_16S_Val   0b100
#define KX134_1211_LP_CNTL1_AVC_16S   KX134_1211_LP_CNTL1_AVC(KX134_1211_LP_CNTL1_AVC_16S_Val)
#define KX134_1211_LP_CNTL1_AVC_32S_Val   0b101
#define KX134_1211_LP_CNTL1_AVC_32S   KX134_1211_LP_CNTL1_AVC(KX134_1211_LP_CNTL1_AVC_32S_Val)
#define KX134_1211_LP_CNTL1_AVC_64S_Val   0b110
#define KX134_1211_LP_CNTL1_AVC_64S   KX134_1211_LP_CNTL1_AVC(KX134_1211_LP_CNTL1_AVC_64S_Val)
#define KX134_1211_LP_CNTL1_AVC_128S_Val   0b111
#define KX134_1211_LP_CNTL1_AVC_128S    \
                    KX134_1211_LP_CNTL1_AVC(KX134_1211_LP_CNTL1_AVC_128S_Val)

//
//  0x3b - LP_CNTL2
//  Low Power Control 2
//
#define KX134_1211_REG_LP_CNTL2     0x3b
#define KX134_1211_REG_LP_CNTL2_RST_VAL 0b10011010

// LPSTPSEL: Digital Power Shut-off Enable
#define KX134_1211_LP_CNTL2_LPSTPSEL_Pos    0
#define KX134_1211_LP_CNTL2_LPSTPSEL        (1 << KX134_1211_LP_CNTL2_LPSTPSEL_Pos)




//
//  0x49 - WUFTH
//  Low Bits of Wake Up Threshold Value
//
#define KX134_1211_REG_WUFTH        0x49
#define KX134_1211_REG_WUFTH_RST_VAL    0b00100000

//
//  0x4a - BTSWUFTH
//  High Bits of Wake Up Threshold Value and Back to Sleep Threshold Value
//
#define KX134_1211_REG_BTSWUFTH     0x4a
#define KX134_1211_REG_BTSWUFTH_RST_VAL 0b00000000

//
//  0x4b - BTSTH
//  Low Bits of Back to Sleep Threshold Value
//
#define KX134_1211_REG_BTSTH        0x4b
#define KX134_1211_REG_BTSTH_RST_VAL    0b00100000

//
//  0x4c - BTSC
//  Debounce Counter for Back to Sleep Engine
//
#define KX134_1211_REG_BTSC     0x4c
#define KX134_1211_REG_BTSC_RST_VAL     0b00000000

//
//  0x4d - WUFC
//  Debounce Counter for Wake Up Engine
//
#define KX134_1211_REG_WUFC     0x4d
#define KX134_1211_REG_WUFC_RST_VAL     0b00000000




//
//  0x5d - SELF_TEST
//  Self Test Enable
//
#define KX134_1211_REG_SELF_TEST    0x5d
#define KX134_1211_REG_SELF_TEST_RST_VAL    0b00000000

#define KX134_1211_REG_SELF_TEST_ENABLE_VAL 0xCA




//
//  0x5e - BUF_CNTL1
//  Buffer Sample Threshold
//
#define KX134_1211_REG_BUF_CNTL1    0x5e
#define KX134_1211_REG_BUF_CNTL1_RST_VAL    0b00000000

//
//  0x5f - BUF_CNTL2
//  Buffer Operation Settings
//
#define KX134_1211_REG_BUF_CNTL2    0x5f
#define KX134_1211_REG_BUF_CNTL2_RST_VAL    0b00000000

// BM: Buffer Mode
#define KX134_1211_BUF_CNTL2_BM_Pos     0
#define KX134_1211_BUF_CNTL2_BM_Msk     (0x3 << KX134_1211_BUF_CNTL2_BM_Pos)
#define KX134_1211_BUF_CNTL2_BM(x)      (((x) << KX134_1211_BUF_CNTL2_BM_Pos) \
                                            & KX134_1211_BUF_CNTL2_BM_Msk)
#define KX134_1211_BUF_CNTL2_BM_FIFO_Val    0b00
#define KX134_1211_BUF_CNTL2_BM_FIFO        \
                    KX134_1211_BUF_CNTL2_BM(KX134_1211_BUF_CNTL2_BM_FIFO_Val)
#define KX134_1211_BUF_CNTL2_BM_STREAM_Val  0b01
#define KX134_1211_BUF_CNTL2_BM_STREAM  \
                    KX134_1211_BUF_CNTL2_BM(KX134_1211_BUF_CNTL2_BM_STREAM_Val)
#define KX134_1211_BUF_CNTL2_BM_TRIGGER_Val 0b10
#define KX134_1211_BUF_CNTL2_BM_TRIGGER  \
                    KX134_1211_BUF_CNTL2_BM(KX134_1211_BUF_CNTL2_BM_TRIGGER_Val)
// BFIE: Buffer Full Interrupt Enable
#define KX134_1211_BUF_CNTL2_BFIE_Pos   5
#define KX134_1211_BUF_CNTL2_BFIE       (1 << KX134_1211_BUF_CNTL2_BFIE_Pos)
// BRES: Sample Resolution Control
#define KX134_1211_BUF_CNTL2_BRES_Pos   6
#define KX134_1211_BUF_CNTL2_BRES       (1 << KX134_1211_BUF_CNTL2_BRES_Pos)
#define KX134_1211_BUF_CNTL2_BRES_16    (1 << KX134_1211_BUF_CNTL2_BRES_Pos)
#define KX134_1211_BUF_CNTL2_BRES_8     (0 << KX134_1211_BUF_CNTL2_BRES_Pos)
// BUFE: Sample Buffer Enable
#define KX134_1211_BUF_CNTL2_BUFE_Pos   7
#define KX134_1211_BUF_CNTL2_BUFE       (1 << KX134_1211_BUF_CNTL2_BUFE_Pos)

//
//  0x60 - BUF_STATUS_1
//  Sample Buffer Status Register 1 (sample level [7:0])
//
#define KX134_1211_REG_BUF_STATUS_1     0x60
#define KX134_1211_REG_BUF_STATUS_1_RST_VAL 0b00000000

//
//  0x61 - BUF_STATUS_2
//  Sample Buffer Status Register 2
//
#define KX134_1211_REG_BUF_STATUS_2     0x61
#define KX134_1211_REG_BUF_STATUS_2_RST_VAL 0b00000000

// SMP_LEV[9:8]: High Bits of Sample Level
#define KX134_1211_BUF_STATUS_2_SMP_LEV_Pos     0
#define KX134_1211_BUF_STATUS_2_SMP_LEV_Msk     (0x3 << KX134_1211_BUF_STATUS_2_SMP_LEV_Pos)
// BUF_TRIG: Buffer Trigger Status
#define KX134_1211_BUF_STATUS_2_BUF_TRIG_Pos    7
#define KX134_1211_BUF_STATUS_2_BUF_TRIG        (1 << KX124_1211_BUF_STATUS_2_BUF_TRIG_Pos)

//
//  0x62 - BUF_CLEAR
//  Buffer Clear
//
#define KX134_1211_REG_BUF_CLEAR        0x62
#define KX134_1211_REG_BUF_CLEAR_RST_VAL    0b00000000

//
//  0x63 - BUF_READ
//  Buffer Read
//
#define KX134_1211_REG_BUF_READ         0x63
#define KX134_1211_REG_BUF_READ_RST_VAL     0b00000000




//
//  0x64 - ADP_CNTL1
//  Advanced Data Path Control Register 1
//
#define KX134_1211_REG_ADP_CNTL1        0x64
#define KX134_1211_REG_ADP_CNTL1_RST_VAL    0b00000000

// OADP: ODR for Advanced Data Path
#define KX134_1211_ADP_CNTL1_OADP_Pos   0
#define KX134_1211_ADP_CNTL1_OADP_Msk   (0xf << KX134_1211_ADP_CNTL1_OADP_Pos)
#define KX134_1211_ADP_CNTL1_OADP(x)    (((x) << KX134_1211_ADP_CNTL1_OADP_Pos)\
                                            & KX134_1211_ADP_CNTL1_OADP_Msk)
#define KX134_1211_ADP_CNTL1_OADP_0_781_Val 0b0000
#define KX134_1211_ADP_CNTL1_OADP_0_781     \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_0_781_Val)
#define KX134_1211_ADP_CNTL1_OADP_1_563_Val 0b0001
#define KX134_1211_ADP_CNTL1_OADP_1_563     \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_1_563_Val)
#define KX134_1211_ADP_CNTL1_OADP_3_125_Val 0b0010
#define KX134_1211_ADP_CNTL1_OADP_3_125     \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_3_125_Val)
#define KX134_1211_ADP_CNTL1_OADP_6_25_Val  0b0011
#define KX134_1211_ADP_CNTL1_OADP_6_25      \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_6_25_Val)
#define KX134_1211_ADP_CNTL1_OADP_12_5_Val  0b0100
#define KX134_1211_ADP_CNTL1_OADP_12_5      \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_12_5_Val)
#define KX134_1211_ADP_CNTL1_OADP_25_Val    0b0101
#define KX134_1211_ADP_CNTL1_OADP_25        \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_25_Val)
#define KX134_1211_ADP_CNTL1_OADP_50_Val    0b0110
#define KX134_1211_ADP_CNTL1_OADP_50        \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_50_Val)
#define KX134_1211_ADP_CNTL1_OADP_100_Val   0b0111
#define KX134_1211_ADP_CNTL1_OADP_100       \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_100_Val)
#define KX134_1211_ADP_CNTL1_OADP_200_Val   0b1000
#define KX134_1211_ADP_CNTL1_OADP_200       \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_200_Val)
#define KX134_1211_ADP_CNTL1_OADP_400_Val   0b1001
#define KX134_1211_ADP_CNTL1_OADP_400       \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_400_Val)
#define KX134_1211_ADP_CNTL1_OADP_800_Val   0b1010
#define KX134_1211_ADP_CNTL1_OADP_800       \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_800_Val)
#define KX134_1211_ADP_CNTL1_OADP_1600_Val  0b1011
#define KX134_1211_ADP_CNTL1_OADP_1600      \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_1600_Val)
#define KX134_1211_ADP_CNTL1_OADP_3200_Val  0b1100
#define KX134_1211_ADP_CNTL1_OADP_3200      \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_3200_Val)
#define KX134_1211_ADP_CNTL1_OADP_6400_Val  0b1101
#define KX134_1211_ADP_CNTL1_OADP_6400      \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_6400_Val)
#define KX134_1211_ADP_CNTL1_OADP_12800_Val 0b1110
#define KX134_1211_ADP_CNTL1_OADP_12800     \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_12800_Val)
#define KX134_1211_ADP_CNTL1_OADP_25600_Val 0b1111
#define KX134_1211_ADP_CNTL1_OADP_25600     \
                    KX134_1211_ODCNTL_OSA(KX134_1211_ADP_CNTL1_OADP_25600_Val)
// RMS_AVC: Number of Samples Used to Calculate RMS Output
#define KX134_1211_ADP_CNTL1_RMS_AVC_Pos    4
#define KX134_1211_ADP_CNTL1_RMS_AVC_Msk    (0x7 << KX134_1211_ADP_CNTL1_RMS_AVC_Pos)
#define KX134_1211_ADP_CNTL1_RMS_AVC(x)     (((x) << KX134_1211_ADP_CNTL1_RMS_AVC_Pos)\
                                                & KX134_1211_ADP_CNTL1_RMS_AVC_Msk)
#define KX134_1211_ADP_CNTL1_RMS_AVC_2_Val  0b000
#define KX134_1211_ADP_CNTL1_RMS_AVC_2      \
            KX134_1211_ADP_CNTL1_RMS_AVC(KX134_1211_ADP_CNTL1_RMS_AVC_2_Val)
#define KX134_1211_ADP_CNTL1_RMS_AVC_4_Val  0b001
#define KX134_1211_ADP_CNTL1_RMS_AVC_4      \
            KX134_1211_ADP_CNTL1_RMS_AVC(KX134_1211_ADP_CNTL1_RMS_AVC_4_Val)
#define KX134_1211_ADP_CNTL1_RMS_AVC_8_Val  0b010
#define KX134_1211_ADP_CNTL1_RMS_AVC_8      \
            KX134_1211_ADP_CNTL1_RMS_AVC(KX134_1211_ADP_CNTL1_RMS_AVC_8_Val)
#define KX134_1211_ADP_CNTL1_RMS_AVC_16_Val  0b011
#define KX134_1211_ADP_CNTL1_RMS_AVC_16     \
KX134_1211_ADP_CNTL1_RMS_AVC(KX134_1211_ADP_CNTL1_RMS_AVC_16_Val)
#define KX134_1211_ADP_CNTL1_RMS_AVC_32_Val  0b100
#define KX134_1211_ADP_CNTL1_RMS_AVC_32     \
            KX134_1211_ADP_CNTL1_RMS_AVC(KX134_1211_ADP_CNTL1_RMS_AVC_32_Val)
#define KX134_1211_ADP_CNTL1_RMS_AVC_64_Val  0b101
#define KX134_1211_ADP_CNTL1_RMS_AVC_64     \
            KX134_1211_ADP_CNTL1_RMS_AVC(KX134_1211_ADP_CNTL1_RMS_AVC_64_Val)
#define KX134_1211_ADP_CNTL1_RMS_AVC_128_Val 0b110
#define KX134_1211_ADP_CNTL1_RMS_AVC_128    \
            KX134_1211_ADP_CNTL1_RMS_AVC(KX134_1211_ADP_CNTL1_RMS_AVC_128_Val)
#define KX134_1211_ADP_CNTL1_RMS_AVC_256_Val 0b111
#define KX134_1211_ADP_CNTL1_RMS_AVC_256    \
            KX134_1211_ADP_CNTL1_RMS_AVC(KX134_1211_ADP_CNTL1_RMS_AVC_256_Val)

//
//  0x65 - ADP_CNTL2
//  Advanced Data Path Control Register 2
//
#define KX134_1211_REG_ADP_CNTL2        0x65
#define KX134_1211_REG_ADP_CNTL2_RST_VAL    0b00000010

// ADP_F2_HP: Filter 2 High Pass Enable
#define KX134_1211_ADP_CNTL2_ADP_F2_HP_Pos  0
#define KX134_1211_ADP_CNTL2_ADP_F2_HP      (1 << KX134_1211_ADP_CNTL2_ADP_F2_HP_Pos)
// ADP_RMS_OSEL: RMS Output Enable
#define KX134_1211_ADP_CNTL2_ADP_RMS_OSEL_Pos   1
#define KX134_1211_ADP_CNTL2_ADP_RMS_OSEL       (1 << KX134_1211_ADP_CNTL2_ADP_RMS_OSEL_Pos)
// ADP_FLT1_BYP: ADP Filter 1 Bypass Enable
#define KX134_1211_ADP_CNTL2_ADP_FLT1_BYP_Pos   3
#define KX134_1211_ADP_CNTL2_ADP_FLT1_BYP       (1 << KX134_1211_ADP_CNTL2_ADP_FLT1_BYP_Pos)
// ADP_FLT2_BYP: ADP Filter 2 Bypass Enable
#define KX134_1211_ADP_CNTL2_ADP_FLT2_BYP_Pos   4
#define KX134_1211_ADP_CNTL2_ADP_FLT2_BYP       (1 << KX134_1211_ADP_CNTL2_ADP_FLT2_BYP_Pos)
// RMS_WB_OSEL: Use RMS Data for Wakup/Back-to-Sleep
#define KX134_1211_ADP_CNTL2_RMS_WB_OSEL_Pos    5
#define KX134_1211_ADP_CNTL2_RMS_WB_OSEL        (1 << KX134_1211_ADP_CNTL2_RMS_WB_OSEL_Pos)
// ADP_WB_ISEL: Use ADP Data for Wakup/Back-to-Sleep
#define KX134_1211_ADP_CNTL2_ADP_WB_ISEL_Pos    6
#define KX134_1211_ADP_CNTL2_ADP_WB_ISEL        (1 << KX134_1211_ADP_CNTL2_ADP_WB_ISEL_Pos)
// ADP_BUF_SEL: Route ADP Data to Sample Buffer
#define KX134_1211_ADP_CNTL2_ADP_BUF_SEL_Pos    7
#define KX134_1211_ADP_CNTL2_ADP_BUF_SEL        (1 << KX134_1211_ADP_CNTL2_ADP_BUF_SEL_Pos)

//
//  0x66 - ADP_CNTL3
//  ADP Register 3 (ADP filter-1 coefficient (1/A))
//
#define KX134_1211_REG_ADP_CNTL3        0x66
#define KX134_1211_REG_ADP_CNTL3_RST_VAL    0b00000000

//
//  0x67 - ADP_CNTL4
//  ADP Register43 (ADP filter-1 coefficient (B/A)[7:0])
//
#define KX134_1211_REG_ADP_CNTL4        0x67
#define KX134_1211_REG_ADP_CNTL4_RST_VAL    0b00000000

//
//  0x68 - ADP_CNTL5
//  ADP Register 5 (ADP filter-1 coefficient (B/A)[15:8])
//
#define KX134_1211_REG_ADP_CNTL5        0x68
#define KX134_1211_REG_ADP_CNTL5_RST_VAL    0b00000000

//
//  0x69 - ADP_CNTL6
//  ADP Register 6 (ADP filter-1 coefficient (B/A)[22:16])
//
#define KX134_1211_REG_ADP_CNTL6        0x69
#define KX134_1211_REG_ADP_CNTL6_RST_VAL    0b00000000

//
//  0x6a - ADP_CNTL7
//  ADP Control Register 7 (ADP filter-1 coefficient (C/A)[7:0])
//
#define KX134_1211_REG_ADP_CNTL7        0x6a
#define KX134_1211_REG_ADP_CNTL7_RST_VAL    0b00000000

//
//  0x6b - ADP_CNTL8
//  ADP Control Register 8 (ADP filter-1 coefficient (C/A)[15:8])
//
#define KX134_1211_REG_ADP_CNTL8        0x6b
#define KX134_1211_REG_ADP_CNTL8_RST_VAL    0b00000000

//
//  0x6c - ADP_CNTL9
//  ADP Control Register 9 (ADP filter-1 coefficient (C/A)[22:16])
//
#define KX134_1211_REG_ADP_CNTL9        0x6c
#define KX134_1211_REG_ADP_CNTL9_RST_VAL    0b00000000

//
//  0x6d - ADP_CNTL10
//  ADP Control Register 10 (ADP filter-1 input scale shift value)
//
#define KX134_1211_REG_ADP_CNTL10       0x6d
#define KX134_1211_REG_ADP_CNTL10_RST_VAL   0b00000000

//
//  0x6e - ADP_CNTL11
//  ADP Control Register 11
//
#define KX134_1211_REG_ADP_CNTL11       0x6e
#define KX134_1211_REG_ADP_CNTL11_RST_VAL   0b00000000

// ADP_F2_1A: ADP filter-2 Coefficient (1/A)
#define KX134_1211_ADP_CNTL11_ADP_F2_1A_Pos 0
#define KX134_1211_ADP_CNTL11_ADP_F2_1A_Msk (0x7f << KX134_1211_ADP_CNTL11_ADP_F2_1A_Pos)
#define KX134_1211_ADP_CNTL11_ADP_F2_1A(x)  (((x) << KX134_1211_ADP_CNTL11_ADP_F2_1A_Pos \
                                                & KX134_1211_ADP_CNTL11_ADP_F2_1A_Msk))
// ADP_F1_OSH: ADP filter-1 Output Scale Shift Value
#define KX134_1211_ADP_CNTL11_ADP_F1_OSH_Pos    7
#define KX134_1211_ADP_CNTL11_ADP_F1_OSH    (1 << KX134_1211_ADP_CNTL11_ADP_F1_OSH_Pos)

//
//  0x6f - ADP_CNTL12
//  ADP Control Register 12 (ADP filter-2 coefficient (B/A)[7:0])
//
#define KX134_1211_REG_ADP_CNTL12       0x6f
#define KX134_1211_REG_ADP_CNTL12_RST_VAL   0b00000000

//
//  0x70 - ADP_CNTL13
//  ADP Control Register 13 (ADP filter-2 coefficient (B/A)[14:8])
//
#define KX134_1211_REG_ADP_CNTL13       0x70
#define KX134_1211_REG_ADP_CNTL13_RST_VAL   0b00000000

//
//  0x71 - ADP_CNTL14
//  ADP Control Register 14 (set to 0)
//
#define KX134_1211_REG_ADP_CNTL14       0x71
#define KX134_1211_REG_ADP_CNTL14_RST_VAL   0b00000000

//
//  0x72 - ADP_CNTL15
//  ADP Control Register 15 (set to 0)
//
#define KX134_1211_REG_ADP_CNTL15       0x72
#define KX134_1211_REG_ADP_CNTL15_RST_VAL   0b00000000

//
//  0x73 - ADP_CNTL16
//  ADP Control Register 16 (set to 0)
//
#define KX134_1211_REG_ADP_CNTL16       0x73
#define KX134_1211_REG_ADP_CNTL16_RST_VAL   0b00000000

//
//  0x74 - ADP_CNTL17
//  ADP Control Register 17 (set to 0)
//
#define KX134_1211_REG_ADP_CNTL17       0x74
#define KX134_1211_REG_ADP_CNTL17_RST_VAL   0b00000000

//
//  0x75 - ADP_CNTL18
//  ADP Control Register 18 (ADP filter-2 input scale shift value)
//
#define KX134_1211_REG_ADP_CNTL18       0x75
#define KX134_1211_REG_ADP_CNTL18_RST_VAL   0b00000000

//
//  0x76 - ADP_CNTL19
//  ADP Control Register 19 (ADP filter-2 output scale shift value)
//
#define KX134_1211_REG_ADP_CNTL19       0x76
#define KX134_1211_REG_ADP_CNTL19_RST_VAL   0b00000000




//
//  0x7f - MYSTERY_RST
//  Undocumented Register Used in Reset Process
//
#define KX134_1211_REG_MYSTERY_RST      0x7f


#endif /* kx134_1211_registers_h */
