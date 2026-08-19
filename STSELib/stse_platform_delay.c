/******************************************************************************
 * \file	stse_platform_crypto.c
 * \brief   STSecureElement cryptographic platform file
 * \author  STMicroelectronics - CS application team
 *
 ******************************************************************************
 * \attention
 *
 * <h2><center>&copy; COPYRIGHT 2022 STMicroelectronics</center></h2>
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "Drivers/st1wire/st1wire_timer.h"
#include "stse_conf.h"
#include "stselib.h"

stse_ReturnCode_t stse_platform_delay_init(void) {
    /* Initialize platform Drivers used by PAL */
    st1wire_timer_init();

    return STSE_OK;
}

void stse_platform_Delay_ms(PLAT_UI16 delay_val) {
    st1wire_timer_delay_us((uint32_t)delay_val * 1000UL);
}
