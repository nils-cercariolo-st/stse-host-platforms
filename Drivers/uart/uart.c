/******************************************************************************
 * \file    uart.c
 * \brief   USART2 polling driver for STM32H523 (PA2/PA3)
 ******************************************************************************/

#include "Drivers/uart/uart.h"

#define STM32H523_HSI_HZ                    64000000UL


static uint32_t get_hsi_clock(void) {
    uint32_t divider =
        (RCC->CR & RCC_CR_HSIDIV_Msk) >> RCC_CR_HSIDIV_Pos;

    return STM32H523_HSI_HZ >> divider;
}


void uart_init(uint32_t baudrate) {
    RCC->CR |= RCC_CR_HSION | RCC_CR_HSIKERON;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0U) {
    }

    /* Select HSI as USART2 kernel clock and enable USART2/GPIOA. */
    RCC->CCIPR1 &= ~RCC_CCIPR1_USART2SEL_Msk;
    RCC->CCIPR1 |= (3UL << RCC_CCIPR1_USART2SEL_Pos);
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1LENR |= RCC_APB1LENR_USART2EN;
    (void)RCC->APB1LENR;

    /* PA2=USART2_TX and PA3=USART2_RX, AF7. */
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk);
    GPIOA->AFR[0] |=
        (7UL << GPIO_AFRL_AFSEL2_Pos) |
        (7UL << GPIO_AFRL_AFSEL3_Pos);
    GPIOA->MODER &= ~(GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
    GPIOA->MODER |=
        (2UL << GPIO_MODER_MODE2_Pos) |
        (2UL << GPIO_MODER_MODE3_Pos);

    USART2->CR1 = 0U;
    USART2->CR2 = 0U;
    USART2->CR3 = USART_CR3_OVRDIS;
    USART2->PRESC = 0U;
    USART2->BRR = get_hsi_clock() / baudrate;
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}


void uart_putc(uint8_t c) {
    while ((USART2->ISR & USART_ISR_TXE_TXFNF) == 0U) {
    }

    USART2->TDR = c;
}


uint8_t uart_getc(void) {
    while ((USART2->ISR & USART_ISR_RXNE_RXFNE) == 0U) {
    }

    return (uint8_t)USART2->RDR;
}
