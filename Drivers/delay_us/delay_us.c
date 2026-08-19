/******************************************************************************
 * \file    delay_us.c
 * \brief   TIM6 microsecond delay driver for STM32H523
 ******************************************************************************/

#include "Drivers/delay_us/delay_us.h"

static uint16_t delay_us_timer_prescaler;


static uint32_t get_tim6_clock(void) {
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


void delay_us_init(void) {
    RCC->APB1LENR |= RCC_APB1LENR_TIM6EN;
    (void)RCC->APB1LENR;

    TIM6->CR1 = TIM_CR1_OPM;
    delay_us_timer_prescaler =
        (uint16_t)((get_tim6_clock() / 1000000UL) - 1UL);
}


static void start_timer(uint16_t ticks) {
    TIM6->CR1 &= ~TIM_CR1_CEN;
    TIM6->PSC = delay_us_timer_prescaler;
    TIM6->ARR = (ticks == 0U) ? 0U : (uint32_t)ticks - 1UL;
    TIM6->EGR = TIM_EGR_UG;
    TIM6->SR = 0U;
    TIM6->CNT = 0U;
    TIM6->CR1 |= TIM_CR1_CEN;
}


void delay_us(uint16_t us) {
    if (us == 0U) {
        return;
    }

    start_timer(us);

    while ((TIM6->SR & TIM_SR_UIF) == 0U) {
    }

    TIM6->SR = 0U;
}


void timeout_us_start(uint16_t us) {
    start_timer((us == 0U) ? 1U : us);
}


uint8_t timeout_us_get_status(void) {
    if ((TIM6->SR & TIM_SR_UIF) != 0U) {
        TIM6->CR1 &= ~TIM_CR1_CEN;
        TIM6->SR = 0U;
        return 1U;
    }

    return 0U;
}
