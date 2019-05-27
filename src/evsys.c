/**
 * @file evsys.c
 * @desc Event System Driver
 * @author Samuel Dewan
 * @date 2019-04-27
 * Last Author:
 * Last Edited On:
 */

#include "evsys.h"

static const uint8_t evsys_clk_ids[] = {
    EVSYS_GCLK_ID_0,
    EVSYS_GCLK_ID_1,
    EVSYS_GCLK_ID_2,
    EVSYS_GCLK_ID_3,
    EVSYS_GCLK_ID_4,
    EVSYS_GCLK_ID_5,
#ifdef EVSYS_GCLK_ID_6
    EVSYS_GCLK_ID_6,
#endif
#ifdef EVSYS_GCLK_ID_7
    EVSYS_GCLK_ID_7,
#endif
#ifdef EVSYS_GCLK_ID_8
    EVSYS_GCLK_ID_8,
#endif
#ifdef EVSYS_GCLK_ID_9
    EVSYS_GCLK_ID_9,
#endif
#ifdef EVSYS_GCLK_ID_10
    EVSYS_GCLK_ID_10,
#endif
#ifdef EVSYS_GCLK_ID_11
    EVSYS_GCLK_ID_11
#endif
};


void init_evsys (void)
{
    /** Enable EVSYS interface clock */
    PM->APBCMASK.reg |= PM_APBCMASK_EVSYS;
    /** Reset EVSYS */
    EVSYS->CTRL.bit.SWRST = 1;
}

void evsys_configure_user_mux (uint8_t user, uint8_t channel)
{
    /* Configure event user mux */
    EVSYS->USER.reg = (EVSYS_USER_USER(user) |
                       EVSYS_USER_CHANNEL((channel == EVSYS_CHANNEL_DISABLED) ?
                                          0 : (channel + 1)));
}

void evsys_configure_channel (uint8_t channel, uint8_t generator,
                              uint32_t clock_mask, enum evsys_path path,
                              enum evsys_edge edge)
{
    /* Select the Generic Clock Generator for the EVSYS channel */
    GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_CLKEN | clock_mask |
                         GCLK_CLKCTRL_ID(evsys_clk_ids[channel]));
    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY);
    /* Configure channel */
    EVSYS->CHANNEL.reg = (EVSYS_CHANNEL_CHANNEL(channel) |
                          EVSYS_CHANNEL_EVGEN(generator) |
                          EVSYS_CHANNEL_PATH(path) |
                          EVSYS_CHANNEL_EDGSEL(edge));
}

void evsys_software_event (uint8_t channel)
{
    EVSYS->CHANNEL.reg = (uint16_t)(EVSYS_CHANNEL_CHANNEL(channel) |
                                    EVSYS_CHANNEL_SWEVT);
}
