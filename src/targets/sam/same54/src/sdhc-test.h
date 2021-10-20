//
//  sdhc-test.h
//  index
//
//  Created by Samuel Dewan on 2021-07-28.
//  Copyright © 2021 Samuel Dewan. All rights reserved.
//

#ifndef sdhc_test_h
#define sdhc_test_h

#include "global.h"

extern void init_sdhc_test(void);

extern int sdhc_test_init_card(void);

extern int sdhc_test_read(uint32_t address, uint16_t num_blocks,
                          uint8_t *buffer);

extern int sdhc_test_write(uint32_t address, uint16_t num_blocks,
                           uint8_t *buffer);

#endif /* sdhc_test_h */
