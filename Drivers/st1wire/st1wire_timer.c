/**
 ******************************************************************************
 * \file    st1wire_timer.c
 * \brief   STM32H523 ST1Wire TIM4 backend (PB8, CH3 output, CH4 capture)
 ******************************************************************************
 */

#include "st1wire_timer.h"
#include "stm32h5xx.h"


static uint32_t get_tim4_clock(void)
{
    uint32_t ppre1;
    uint32_t divider;
    uint32_t pclk1;

    ppre1 =
        (RCC->CFGR2 & RCC_CFGR2_PPRE1_Msk) >>
        RCC_CFGR2_PPRE1_Pos;

    if (ppre1 < 4U)
    {
        return SystemCoreClock;
    }

    divider = 1UL << (ppre1 - 3U);
    pclk1 = SystemCoreClock / divider;

    /* TIMPRE=0 gives 2 x PCLK when the APB prescaler is not one. */
    if ((RCC->CFGR1 & RCC_CFGR1_TIMPRE) == 0U)
    {
        return pclk1 * 2UL;
    }

    /* TIMPRE=1 gives HCLK up to APB /4, otherwise 4 x PCLK. */
    return (divider <= 4UL)
        ? SystemCoreClock
        : pclk1 * 4UL;
}


static void configure_pin(void)
{
    /* Release the open-drain line before assigning PB8 to TIM4_CH3. */
    GPIOB->BSRR = GPIO_BSRR_BS8;

    GPIOB->OTYPER |= GPIO_OTYPER_OT8;

    GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED8_Msk;
    GPIOB->OSPEEDR |= (3UL << GPIO_OSPEEDR_OSPEED8_Pos);

    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD8_Msk;

    GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL8_Msk;
    GPIOB->AFR[1] |= (2UL << GPIO_AFRH_AFSEL8_Pos);

    GPIOB->MODER &= ~GPIO_MODER_MODE8_Msk;
    GPIOB->MODER |= (2UL << GPIO_MODER_MODE8_Pos);
}


void st1wire_timer_init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->APB1LENR |= RCC_APB1LENR_TIM4EN;

    (void)RCC->AHB2ENR;
    (void)RCC->APB1LENR;

    st1wire_timer_stop();
}


void st1wire_timer_deinit(void)
{
    st1wire_timer_stop();
    st1wire_timer_release_pin();
}


void st1wire_timer_prepare(void)
{
    uint32_t timer_clock;

    st1wire_timer_stop();
    configure_pin();

    timer_clock = get_tim4_clock();

    TIM4->PSC =
        (timer_clock / ST1WIRE_TIMER_FREQUENCY_HZ) - 1UL;

    TIM4->ARR = 0xFFFFU;
    TIM4->CNT = 0U;
    TIM4->CR1 = 0U;
    TIM4->CR2 = 0U;
    TIM4->SMCR = 0U;
    TIM4->CCMR1 = 0U;

    /* CH3 toggles PB8. CH4 indirectly captures TI3 on both edges. */
    TIM4->CCMR2 =
        (3UL << TIM_CCMR2_OC3M_Pos) |
        (2UL << TIM_CCMR2_CC4S_Pos);

    TIM4->CCER =
        TIM_CCER_CC3E |
        TIM_CCER_CC3P |
        TIM_CCER_CC4E |
        TIM_CCER_CC4P |
        TIM_CCER_CC4NP;

    TIM4->EGR = TIM_EGR_UG;
    TIM4->SR = 0U;
}


void st1wire_timer_stop(void)
{
    TIM4->CR1 &= ~TIM_CR1_CEN;
    TIM4->DIER = 0U;
    TIM4->CCER = 0U;
    TIM4->SR = 0U;
}


void st1wire_timer_release_pin(void)
{
    /* Input mode releases the external pull-up controlled ST1Wire bus. */
    GPIOB->MODER &= ~GPIO_MODER_MODE8_Msk;
}


void st1wire_timer_set_compare(
    uint16_t compare)
{
    TIM4->CCR3 = compare;
}


void st1wire_timer_enable_dma_requests(void)
{
    TIM4->DIER =
        TIM_DIER_CC3DE |
        TIM_DIER_CC4DE;
}


void st1wire_timer_start(void)
{
    TIM4->CNT = 0U;
    TIM4->SR = 0U;
    TIM4->CR1 |= TIM_CR1_CEN;
}


uint16_t st1wire_timer_get_counter(void)
{
    return (uint16_t)TIM4->CNT;
}
