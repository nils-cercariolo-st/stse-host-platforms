/**
 ******************************************************************************
 * \file    st1wire_dma.c
 * \brief   STM32L452 ST1Wire DMA backend
 ******************************************************************************
 */

#include "st1wire_dma.h"
#include "stm32l4xx.h"


void st1wire_dma_init(void)
{
    RCC->AHB1ENR |=
        RCC_AHB1ENR_DMA1EN;

    (void)RCC->AHB1ENR;

    st1wire_dma_stop();
}


void st1wire_dma_deinit(void)
{
    st1wire_dma_stop();
}


void st1wire_dma_stop(void)
{
    DMA1_Channel2->CCR &=
        ~DMA_CCR_EN;

    DMA1_Channel3->CCR &=
        ~DMA_CCR_EN;

    while ((DMA1_Channel2->CCR &
            DMA_CCR_EN) != 0U)
    {
    }

    while ((DMA1_Channel3->CCR &
            DMA_CCR_EN) != 0U)
    {
    }

    DMA1->IFCR =
        DMA_IFCR_CGIF2 |
        DMA_IFCR_CGIF3;
}


void st1wire_dma_config_capture(
    volatile uint16_t *buffer,
    uint16_t count)
{
    DMA1_Channel2->CCR &=
        ~DMA_CCR_EN;

    DMA1->IFCR =
        DMA_IFCR_CGIF2;

    /* DMA1 CH2 request 7 = TIM1_CH1 */
    DMA1_CSELR->CSELR &=
        ~(0xFUL << 4U);

    DMA1_CSELR->CSELR |=
        (ST1WIRE_DMA_CAPTURE_REQUEST << 4U);

    DMA1_Channel2->CPAR =
        (uint32_t)&TIM1->CCR1;

    DMA1_Channel2->CMAR =
        (uint32_t)buffer;

    DMA1_Channel2->CNDTR =
        count;

    DMA1_Channel2->CCR =
        DMA_CCR_MINC |
        DMA_CCR_PSIZE_0 |
        DMA_CCR_MSIZE_0 |
        DMA_CCR_PL_1;
}


void st1wire_dma_config_output(
    const uint16_t *buffer,
    uint16_t count)
{
    DMA1_Channel3->CCR &=
        ~DMA_CCR_EN;

    DMA1->IFCR =
        DMA_IFCR_CGIF3;

    /* DMA1 CH3 request 7 = TIM1_CH2 */
    DMA1_CSELR->CSELR &=
        ~(0xFUL << 8U);

    DMA1_CSELR->CSELR |=
        (ST1WIRE_DMA_OUTPUT_REQUEST << 8U);

    DMA1_Channel3->CPAR =
        (uint32_t)&TIM1->CCR2;

    DMA1_Channel3->CMAR =
        (uint32_t)buffer;

    DMA1_Channel3->CNDTR =
        count;

    DMA1_Channel3->CCR =
        DMA_CCR_DIR |
        DMA_CCR_MINC |
        DMA_CCR_PSIZE_0 |
        DMA_CCR_MSIZE_0 |
        DMA_CCR_PL_1 |
        DMA_CCR_PL_0;
}


void st1wire_dma_start_capture(void)
{
    DMA1_Channel2->CCR |=
        DMA_CCR_EN;
}


void st1wire_dma_start_output(void)
{
    DMA1_Channel3->CCR |=
        DMA_CCR_EN;
}


uint16_t st1wire_dma_capture_remaining(void)
{
    return (uint16_t)
        DMA1_Channel2->CNDTR;
}