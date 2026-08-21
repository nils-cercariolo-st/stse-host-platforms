/******************************************************************************
 * \file	rng.c
 * \brief   Random Number Generator driver for STM32H533
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

#include "Drivers/rng/rng.h"

void rng_start(void) {
    RCC->CR |= RCC_CR_HSI48ON;
    while ((RCC->CR & RCC_CR_HSI48RDY) == 0U) {
    }

    RCC->CCIPR5 &= ~RCC_CCIPR5_RNGSEL_Msk;
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
    (void)RCC->AHB2ENR;

    RNG->CR |= RNG_CR_CED | RNG_CR_RNGEN;
}

uint32_t rng_generate_random_number(void) {
    while (!(RNG->SR & RNG_SR_DRDY))
        ;
    return RNG->DR;
}

void rng_stop(void) {
    RNG->CR &= ~(RNG_CR_RNGEN);
}
