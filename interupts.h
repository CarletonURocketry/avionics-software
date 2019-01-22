/**
 * @file interupts.h
 * @desc Declerations of interupt service routines
 * @author Samuel Dewan
 * @date 2018-12-17
 * Last Author:
 * Last Edited On:
 */

#ifndef interupts_h
#define interupts_h

/**
 *  Handler for hard fault
 */
extern void hardfault_handler(void);

/**
 *  Handler for SysTick interupt
 */
extern void systick_handler(void);



/**
 *  Handler for the DMAC interupt
 */
extern void dmac_handler (void);



/**
 *  Handler for SERCOM0 interupt
 */
extern void sercom0_handler (void);

/**
 *  Handler for SERCOM1 interupt
 */
extern void sercom1_handler (void);

/**
 *  Handler for SERCOM2 interupt
 */
extern void sercom2_handler (void);

/**
 *  Handler for SERCOM3 interupt
 */
extern void sercom3_handler (void);

#ifdef ID_SERCOM4
/**
 *  Handler for SERCOM4 interupt
 */
extern void sercom4_handler (void);
#endif

#ifdef ID_SERCOM5
/**
 *  Handler for SERCOM5 interupt
 */
extern void sercom5_handler (void);
#endif

#endif /* interupts_h */
