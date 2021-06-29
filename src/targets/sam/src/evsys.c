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
    enable_bus_clock(PERPH_BUS_CLK_EVSYS_APB);
    /** Reset EVSYS */
#if defined(SAMD2x)
    EVSYS->CTRL.bit.SWRST = 1;
#elif defined(SAMx5x)
    EVSYS->CTRLA.bit.SWRST = 1;
#endif
}

void evsys_configure_user_mux (uint8_t user, uint8_t channel)
{
    /* Configure event user mux */
#if defined(SAMD2x)
    EVSYS->USER.reg = (EVSYS_USER_USER(user) |
                       EVSYS_USER_CHANNEL((channel == EVSYS_CHANNEL_DISABLED) ?
                                          0 : (channel + 1)));
#elif defined(SAMx5x)
    EVSYS->USER[user].bit.CHANNEL = ((channel == EVSYS_CHANNEL_DISABLED) ?
                                     0 : (channel + 1));
#endif
}

void evsys_configure_channel (uint8_t channel, uint8_t generator,
                              uint32_t clock_mask, enum evsys_path path,
                              enum evsys_edge edge)
{
#if defined(SAMD2x)
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
#elif defined(SAMx5x)
    if (channel < EVSYS_SYNCH_NUM) {
        /* Select the Generic Clock Generator for the EVSYS channel */
        do {
            GCLK->PCHCTRL[evsys_clk_ids[channel]].reg = (GCLK_PCHCTRL_CHEN |
                                                         clock_mask);
        } while(!GCLK->PCHCTRL[evsys_clk_ids[channel]].bit.CHEN);
    }
    /* Configure channel */
    EVSYS->Channel[channel].CHANNEL.reg = (EVSYS_CHANNEL_EVGEN(generator) |
                                           EVSYS_CHANNEL_PATH(path) |
                                           EVSYS_CHANNEL_EDGSEL(edge));
#endif
}

void evsys_software_event (uint8_t channel)
{
#if defined(SAMD2x)
    EVSYS->CHANNEL.bit.CHANNEL = channel;
    EVSYS->CHANNEL.reg |= EVSYS_CHANNEL_SWEVT;
#elif defined(SAMx5x)
    EVSYS->SWEVT.reg = (1 << channel);
#endif
}
