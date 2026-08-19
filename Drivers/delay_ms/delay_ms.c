/******************************************************************************
 * \file    delay_ms.c
 * \brief   TIM2 millisecond delay driver for STM32H523
 ******************************************************************************/

#include "Drivers/delay_ms/delay_ms.h"

static uint16_t delay_ms_timer_prescaler;


static uint32_t get_tim2_clock(void) {
    uint32_t ppre1 =
        (RCC->CFGR2 & RCC_CFGR2_PPRE1_Msk) >> RCC_CFGR2_PPRE1_Pos;
    uint32_t divider;
    uint32_t pclk1;

    if (ppre1 < 4U) {
        return SystemCoreClock;
    }

    divider = 1UL << (ppre1 - 3U);
    pclk1 = SystemCoreClock / divider;

    if ((RCC->CFGR1 & RCC_CFGR1_TIMPRE) == 0U) {
        return pclk1 * 2UL;
    }

    return (divider <= 4UL) ? SystemCoreClock : pclk1 * 4UL;
}


void delay_ms_init(void) {
    RCC->APB1LENR |= RCC_APB1LENR_TIM2EN;
    (void)RCC->APB1LENR;

    TIM2->CR1 = TIM_CR1_OPM;
    delay_ms_timer_prescaler =
        (uint16_t)((get_tim2_clock() / 1000000UL) - 1UL);
}


static void start_timer(uint16_t ticks) {
    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->PSC = delay_ms_timer_prescaler;
    TIM2->ARR = (ticks == 0U)
        ? 0U
        : ((uint32_t)ticks * 1000UL) - 1UL;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = 0U;
    TIM2->CNT = 0U;
    TIM2->CR1 |= TIM_CR1_CEN;
}


void delay_ms(uint16_t ms) {
    if (ms == 0U) {
        return;
    }

    start_timer(ms);

    while ((TIM2->SR & TIM_SR_UIF) == 0U) {
    }

    TIM2->SR = 0U;
}


void timeout_ms_start(uint16_t ms) {
    start_timer((ms == 0U) ? 1U : ms);
}


uint8_t timeout_ms_get_status(void) {
    if ((TIM2->SR & TIM_SR_UIF) != 0U) {
        TIM2->CR1 &= ~TIM_CR1_CEN;
        TIM2->SR = 0U;
        return 1U;
    }

    return 0U;
}
