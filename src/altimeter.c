/**
 * @file altimeter.c
 * @desc The main altimeter file.
 * @author Maxim Zakirov
 * @date 2019-03-25
 * Last Author: Me
 * Last Edited On: 2019-04-21
 */

#include "altimeter.h"

#include "sercom-i2c.h"


struct sercom_i2c_desc_t descriptor;

void init_altimeter(void)
{
   
    uint8_t i2c_t; //transaction id

    //Altimeter in STANDBY during config
    
    sercom_i2c_start_reg_write(&i2c_g,
                               &i2c_t,
                               DEV_ADDRESS,
                               0x26, //CTRL_REG1
                               (uint8_t*)0b10111000,// Altimeter mode and oversample 111 (128 ratio)
                               2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    
    sercom_i2c_start_reg_write(&i2c_g,
                               &i2c_t,
                               DEV_ADDRESS,
                               0x13, //PT_DATA_CFG
                               (uint8_t*)0b00000111, //Data Flags Enabled
                               2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);

    
    //Alitmeter is set to ACTIVE mode
    
    sercom_i2c_start_reg_write(&i2c_g,
                               &i2c_t,
                               DEV_ADDRESS,
                               0x26, //CTRL_REG1
                               (uint8_t*)0b10111001,
                               2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    sercom_i2c_clear_transaction(&i2c_g, i2c_t);
}



float get_altitude(void)
{
    
    uint8_t i2c_t; //transaction id
    uint8_t status;
    uint8_t offset; //altitude offset value
    
    uint8_t MSB;
    uint8_t CSB;
    uint8_t LSB;
    
    //Need to get offset altitude value
    sercom_i2c_start_reg_read(&i2c_g,
                              &i2c_t,
                              DEV_ADDRESS,
                              0x2D, //Altitude Data User Offset Register (OFF_H)
                              (uint8_t*)&offset,
                              2);
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    
    sercom_i2c_start_reg_read(&i2c_g, &i2c_t, DEV_ADDRESS, 0b00000000, (uint8_t*)&status, 2);
    
    while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
    
    if((status & 0x08) == true) {
        sercom_i2c_start_reg_read(&i2c_g,
                                  &i2c_t,
                                  DEV_ADDRESS,
                                  0x01, //OUT_P_MSB register
                                  (uint8_t*)&MSB,
                                  2);
        while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
        altitude_data |= (MSB << 16);
        
        sercom_i2c_start_reg_read(&i2c_g,
                                  &i2c_t,
                                  DEV_ADDRESS,
                                  0x02, //OUT_P_CSB register
                                  (uint8_t*)&CSB,
                                  2);
        while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
        altitude_data |= (CSB << 8);
        sercom_i2c_start_reg_read(&i2c_g,
                                  &i2c_t,
                                  DEV_ADDRESS,
                                  0x3, //OUT_P_LSB register
                                  (uint8_t*)&LSB,
                                  2);
        while (!sercom_i2c_transaction_done(&i2c_g, i2c_t)) sercom_i2c_service(&i2c_g);
        altitude_data |= (LSB);
        sercom_i2c_clear_transaction(&i2c_g, i2c_t);
    }
   
    //Calculate altitude from the pressure
    float temp_altitude_data = (float)altitude_data/101326;
    float temp2_altitude_data = 1 - powf(temp_altitude_data, 0.1902632f);
    float new_altitude_data = ((temp2_altitude_data)*44330.77f)+(float)offset;
    
    return new_altitude_data;
}











