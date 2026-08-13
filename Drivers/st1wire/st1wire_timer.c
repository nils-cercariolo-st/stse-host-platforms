/**
 ******************************************************************************
 * \file    st1wire_timer.c
 * \brief   STM32L452 ST1Wire timer backend
 ******************************************************************************
 */

#include "st1wire_timer.h"
#include "stm32l4xx.h"


static uint32_t get_tim1_clock(void)
{
    uint32_t ppre2;
    uint32_t divider;
    uint32_t pclk2;

    ppre2 =
        (RCC->CFGR & RCC_CFGR_PPRE2_Msk)
        >> RCC_CFGR_PPRE2_Pos;

    if (ppre2 < 4U)
    {
        return SystemCoreClock;
    }

    divider =
        1UL << (ppre2 - 3U);

    pclk2 =
        SystemCoreClock / divider;

    return pclk2 * 2UL;
}


static void configure_pin(void)
{
    GPIOA->OTYPER |=
        (1UL << 9U);

    GPIOA->OSPEEDR &=
        ~(3UL << GPIO_OSPEEDR_OSPEED9_Pos);

    GPIOA->OSPEEDR |=
        (3UL << GPIO_OSPEEDR_OSPEED9_Pos);

    GPIOA->PUPDR &=
        ~(3UL << GPIO_PUPDR_PUPD9_Pos);

    GPIOA->AFR[1] &=
        ~(0xFUL << 4U);

    GPIOA->AFR[1] |=
        (0x1UL << 4U);

    GPIOA->MODER &=
        ~(3UL << GPIO_MODER_MODE9_Pos);

    GPIOA->MODER |=
        (2UL << GPIO_MODER_MODE9_Pos);
}


void st1wire_timer_init(void)
{
    RCC->AHB2ENR |=
        RCC_AHB2ENR_GPIOAEN;

    RCC->APB2ENR |=
        RCC_APB2ENR_TIM1EN;

    (void)RCC->AHB2ENR;
    (void)RCC->APB2ENR;

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

    timer_clock =
        get_tim1_clock();

    TIM1->PSC =
        (timer_clock /
         ST1WIRE_TIMER_FREQUENCY_HZ) - 1UL;

    TIM1->ARR =
        0xFFFFU;

    TIM1->CNT =
        0U;

    TIM1->CR1 =
        0U;

    TIM1->CR2 =
        0U;

    TIM1->SMCR =
        0U;

    /* CH1 captures TI2, CH2 is output-compare toggle */
    TIM1->CCMR1 =
        (2UL << TIM_CCMR1_CC1S_Pos) |
        (3UL << TIM_CCMR1_OC2M_Pos);

    TIM1->CCER =
        TIM_CCER_CC1E |
        TIM_CCER_CC1P |
        TIM_CCER_CC1NP |
        TIM_CCER_CC2E |
        TIM_CCER_CC2P;

    TIM1->BDTR |=
        TIM_BDTR_MOE;

    TIM1->EGR =
        TIM_EGR_UG;

    TIM1->SR =
        0U;
}


void st1wire_timer_stop(void)
{
    TIM1->CR1 &=
        ~TIM_CR1_CEN;

    TIM1->DIER =
        0U;

    TIM1->CCER =
        0U;

    TIM1->BDTR &=
        ~TIM_BDTR_MOE;

    TIM1->SR =
        0U;
}


void st1wire_timer_release_pin(void)
{
    GPIOA->MODER &=
        ~GPIO_MODER_MODE9_Msk;
}


void st1wire_timer_set_compare(
    uint16_t compare)
{
    TIM1->CCR2 =
        compare;
}


void st1wire_timer_enable_dma_requests(void)
{
    TIM1->DIER =
        TIM_DIER_CC1DE |
        TIM_DIER_CC2DE;
}


void st1wire_timer_start(void)
{
    TIM1->CNT =
        0U;

    TIM1->SR =
        0U;

    TIM1->CR1 |=
        TIM_CR1_CEN;
}


uint16_t st1wire_timer_get_counter(void)
{
    return (uint16_t)TIM1->CNT;
}