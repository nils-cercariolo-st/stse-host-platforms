/**
 ******************************************************************************
 * \file    st1wire_timer.c
 * \brief   STM32H523 ST1Wire TIM4 backend (CH1 timebase, CH3 TX, CH4 RX)
 ******************************************************************************
 */

#include "st1wire_timer.h"
#include "stm32h5xx.h"

static uint16_t timeout_start_count;
static uint16_t timeout_ticks;
static uint8_t timeout_active;


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


static void configure_timebase(void)
{
    uint32_t timer_clock = get_tim4_clock();

    TIM4->CR1 = 0U;
    TIM4->PSC =
        (timer_clock / ST1WIRE_TIMER_FREQUENCY_HZ) - 1UL;
    TIM4->ARR = 0xFFFFU;
    TIM4->CNT = 0U;
    TIM4->EGR = TIM_EGR_UG;
    TIM4->SR = 0U;
}


static void clear_channel_1_flag(void)
{
    TIM4->SR &= ~TIM_SR_CC1IF;
}


static void wait_channel_1(uint16_t ticks)
{
    uint16_t start;

    clear_channel_1_flag();
    start = (uint16_t)TIM4->CNT;
    TIM4->CCR1 = (uint16_t)(start + ticks);

    /* The elapsed check also covers an exceptionally late CCR1 write. */
    while (((TIM4->SR & TIM_SR_CC1IF) == 0U) &&
           ((uint16_t)((uint16_t)TIM4->CNT - start) < ticks))
    {
    }

    clear_channel_1_flag();
}


void st1wire_timer_init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->APB1LENR |= RCC_APB1LENR_TIM4EN;

    (void)RCC->AHB2ENR;
    (void)RCC->APB1LENR;

    st1wire_timer_stop();
    configure_timebase();
}


void st1wire_timer_deinit(void)
{
    st1wire_timer_stop();
    st1wire_timer_release_pin();
}


void st1wire_timer_prepare(void)
{
    st1wire_timer_stop();
    configure_pin();
    configure_timebase();
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
    timeout_active = 0U;
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


void st1wire_timer_delay_us(uint32_t delay)
{
    uint8_t started_here;

    if (delay == 0U)
    {
        return;
    }

    /* CH1 is shared by blocking delays and the single outstanding timeout. */
    timeout_active = 0U;
    started_here = ((TIM4->CR1 & TIM_CR1_CEN) == 0U) ? 1U : 0U;

    if (started_here != 0U)
    {
        TIM4->CR1 |= TIM_CR1_CEN;
    }

    while (delay > UINT16_MAX)
    {
        wait_channel_1(UINT16_MAX);
        delay -= UINT16_MAX;
    }

    wait_channel_1((uint16_t)delay);

    if (started_here != 0U)
    {
        TIM4->CR1 &= ~TIM_CR1_CEN;
    }
}


void st1wire_timer_timeout_start(uint16_t timeout)
{
    timeout_ticks = (timeout == 0U) ? 1U : timeout;
    timeout_active = 1U;

    if ((TIM4->CR1 & TIM_CR1_CEN) == 0U)
    {
        TIM4->CR1 |= TIM_CR1_CEN;
    }

    clear_channel_1_flag();
    timeout_start_count = (uint16_t)TIM4->CNT;
    TIM4->CCR1 = (uint16_t)(timeout_start_count + timeout_ticks);
}


int8_t st1wire_timer_timeout_expired(void)
{
    uint16_t elapsed;

    if (timeout_active == 0U)
    {
        return 0;
    }

    elapsed = (uint16_t)((uint16_t)TIM4->CNT - timeout_start_count);

    if (((TIM4->SR & TIM_SR_CC1IF) != 0U) || (elapsed >= timeout_ticks))
    {
        timeout_active = 0U;
        clear_channel_1_flag();
        return 1;
    }

    return 0;
}
