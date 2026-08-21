/**
 ******************************************************************************
 * \brief  STM32H533 ST1Wire platform abstraction for PB8
 * \author STMicroelectronics SMD Application Team
 *****************************************************************************/

#include "Drivers/st1wire/st1wire_timer.h"
#include "stm32h5xx.h"

extern uint32_t SystemCoreClock;
volatile uint32_t st1wire_ref_cpu_cycles = 0;

/* ---------- Static Platform Abstraction layer Declarations ---------- */

void st1wire_platform_init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    (void)RCC->AHB2ENR;

    /* Initialize PB8 (ST1Wire line) as released open-drain output. */
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD8_Msk;
    GPIOB->OTYPER |= GPIO_OTYPER_OT8;
    GPIOB->BSRR = GPIO_BSRR_BS8;
    GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED8_Msk;
    GPIOB->OSPEEDR |= (3UL << GPIO_OSPEEDR_OSPEED8_Pos);
    GPIOB->MODER &= ~GPIO_MODER_MODE8_Msk;
    GPIOB->MODER |= (1UL << GPIO_MODER_MODE8_Pos);

}

void st1wire_platform_deinit(void) {
    /* Do Nothing */
}

void st1wire_platform_io_set(uint8_t bus_addr) {
    (void)bus_addr;
    GPIOB->BSRR = GPIO_BSRR_BS8;
}

void st1wire_platform_io_clear(uint8_t bus_addr) {
    (void)bus_addr;
    GPIOB->BSRR = GPIO_BSRR_BR8;
}

uint8_t st1wire_platform_io_get(uint8_t bus_addr) {
    (void)bus_addr;
    if ((GPIOB->IDR & GPIO_IDR_ID8) != 0U) {
        return 1;
    } else {
        return 0;
    }
}

void st1wire_platform_io_in(uint8_t bus_addr) {
    (void)bus_addr;
    GPIOB->MODER &= ~GPIO_MODER_MODE8_Msk;
}

void st1wire_platform_io_out(uint8_t bus_addr) {
    (void)bus_addr;
    GPIOB->MODER &= ~GPIO_MODER_MODE8_Msk;
    GPIOB->MODER |= (1UL << GPIO_MODER_MODE8_Pos);
}

void st1wire_platform_delay(uint32_t delay) {
    st1wire_timer_delay_us(delay);
}

void st1wire_platform_wake(uint8_t bus_addr) {
    st1wire_platform_io_clear(bus_addr);
    st1wire_platform_delay(1000);
    st1wire_platform_io_set(bus_addr);
    st1wire_platform_delay(8000);
}

void st1wire_platform_start_timeout(uint32_t timeout) {
    st1wire_timer_timeout_start((uint16_t)timeout);
}

int8_t st1wire_platform_is_timeout_exceeded(void) {
    return st1wire_timer_timeout_expired();
}
