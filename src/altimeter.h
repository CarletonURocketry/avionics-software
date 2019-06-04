/**
 * @file altimeter.h
 * @desc The main altimeter file.
 * @author Maxim Zakirov
 * @date 2019-03-25
 * Last Author: Nobody
 * Last Edited On: 2019-04-20
 */

#ifndef altimeter_h
#define altimeter_h

#include "global.h"
#include "config.h"
#include <math.h>

//Definitions

#define DEV_ADDRESS 0b1110110

uint32_t altitude_data;

//Function prototypes

/* Start-up sequence for the altimeter */
void init_altimeter(void);

/* Get the Altitude from the pressure sensor */
float get_altitude(void);



#endif /* altimeter_h */
