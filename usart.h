//
//  usart.h
//  index
//
//  Created by Samuel Dewan on 2018-12-26.
//  Copyright © 2018 Samuel Dewan. All rights reserved.
//

#ifndef usart_h
#define usart_h

#define DONT_USE_CMSIS_INIT
#include "global.h"

/**
 *  Initilize a sercom module as a usart
 */
void init_sercom_usart (Sercom *inst, uint32_t baudrate);

extern void sercom_usart_send (Sercom *const inst, const char *data);

#endif /* usart_h */
