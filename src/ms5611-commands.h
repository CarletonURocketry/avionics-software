/**
 * @file ms5611-commands.h
 * @desc Command descriptions for MS5611 barometric pressure sensor
 * @author Samuel Dewan
 * @date 2019-06-04
 * Last Author:
 * Last Edited On:
 */

#ifndef ms5611_commands_h
#define ms5611_commands_h

#define MS5611_ADDR             0b11101100
#define MS5611_ADDR_CSB_Pos     1

#define MS5611_CMD_RESET        0x1E
#define MS5611_CMD_D1           0x40
#define MS5611_CMD_D2           0x50
#define MS5611_CMD_ADC_READ     0x00
#define MS5611_CMD_PROM_READ    0xA0

#define MS5611_OSR_256_Val      0b000
#define MS5611_OSR_512_Val      0b001
#define MS5611_OSR_1024_Val     0b010
#define MS5611_OSR_2048_Val     0b011
#define MS5611_OSR_4096_Val     0b100

#define MS5611_OSR_Pos          1
#define MS5611_OSR_256          (MS5611_OSR_256_Val << MS5611_OSR_Pos)
#define MS5611_OSR_512          (MS5611_OSR_512_Val << MS5611_OSR_Pos)
#define MS5611_OSR_1024         (MS5611_OSR_1024_Val << MS5611_OSR_Pos)
#define MS5611_OSR_2048         (MS5611_OSR_2048_Val << MS5611_OSR_Pos)
#define MS5611_OSR_4096         (MS5611_OSR_4096_Val << MS5611_OSR_Pos)

#define MS5611_PROM_MAN_Val     0
#define MS5611_PROM_C1_Val      1
#define MS5611_PROM_C2_Val      2
#define MS5611_PROM_C3_Val      3
#define MS5611_PROM_C4_Val      4
#define MS5611_PROM_C5_Val      5
#define MS5611_PROM_C6_Val      6
#define MS5611_PROM_CRC_Val     7

#define MS5611_PROM_ADDR_Pos    1
#define MS5611_PROM_MAN         (MS5611_PROM_MAN_Val << MS5611_PROM_ADDR_Pos)
#define MS5611_PROM_C1          (MS5611_PROM_C1_Val << MS5611_PROM_ADDR_Pos)
#define MS5611_PROM_C2          (MS5611_PROM_C2_Val << MS5611_PROM_ADDR_Pos)
#define MS5611_PROM_C3          (MS5611_PROM_C3_Val << MS5611_PROM_ADDR_Pos)
#define MS5611_PROM_C4          (MS5611_PROM_C4_Val << MS5611_PROM_ADDR_Pos)
#define MS5611_PROM_C5          (MS5611_PROM_C5_Val << MS5611_PROM_ADDR_Pos)
#define MS5611_PROM_C6          (MS5611_PROM_C6_Val << MS5611_PROM_ADDR_Pos)
#define MS5611_PROM_CRC         (MS5611_PROM_CRC_Val << MS5611_PROM_ADDR_Pos)


#endif /* ms5611_commands_h */
