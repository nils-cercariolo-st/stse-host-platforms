/**
 ******************************************************************************
 * \file    st1wire_dma.c
 * \brief   STM32H533 ST1Wire GPDMA backend
 ******************************************************************************
 */

#include "st1wire_dma.h"
#include "stm32h5xx.h"


#define ST1WIRE_DMA_CAPTURE_CHANNEL         GPDMA1_Channel0
#define ST1WIRE_DMA_OUTPUT_CHANNEL          GPDMA1_Channel1

#define ST1WIRE_DMA_CLEAR_FLAGS             \
    (DMA_CFCR_TCF | DMA_CFCR_HTF | DMA_CFCR_DTEF | \
     DMA_CFCR_ULEF | DMA_CFCR_USEF | DMA_CFCR_SUSPF | DMA_CFCR_TOF)


static void stop_channel(
    DMA_Channel_TypeDef *channel)
{
    if ((channel->CCR & DMA_CCR_EN) != 0U)
    {
        channel->CCR |= DMA_CCR_SUSP;

        while (((channel->CSR & DMA_CSR_SUSPF) == 0U) &&
               ((channel->CCR & DMA_CCR_EN) != 0U))
        {
        }

        channel->CCR |= DMA_CCR_RESET;

        while ((channel->CCR & DMA_CCR_EN) != 0U)
        {
        }
    }

    channel->CFCR = ST1WIRE_DMA_CLEAR_FLAGS;
}


void st1wire_dma_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPDMA1EN;
    (void)RCC->AHB1ENR;

    st1wire_dma_stop();
}


void st1wire_dma_deinit(void)
{
    st1wire_dma_stop();
}


void st1wire_dma_stop(void)
{
    stop_channel(ST1WIRE_DMA_CAPTURE_CHANNEL);
    stop_channel(ST1WIRE_DMA_OUTPUT_CHANNEL);
}


void st1wire_dma_config_capture(
    volatile uint16_t *buffer,
    uint16_t count)
{
    DMA_Channel_TypeDef *channel = ST1WIRE_DMA_CAPTURE_CHANNEL;

    stop_channel(channel);

    channel->CCR = DMA_CCR_PRIO_1;
    channel->CTR1 =
        DMA_CTR1_SDW_LOG2_0 |
        DMA_CTR1_DDW_LOG2_0 |
        DMA_CTR1_DINC;
    channel->CTR2 =
        (ST1WIRE_DMA_CAPTURE_REQUEST << DMA_CTR2_REQSEL_Pos);
    channel->CBR1 =
        ((uint32_t)count * sizeof(uint16_t)) & DMA_CBR1_BNDT;
    channel->CSAR = (uint32_t)(uintptr_t)&TIM4->CCR4;
    channel->CDAR = (uint32_t)(uintptr_t)buffer;
    channel->CLLR = 0U;
}


void st1wire_dma_config_output(
    const uint16_t *buffer,
    uint16_t count)
{
    DMA_Channel_TypeDef *channel = ST1WIRE_DMA_OUTPUT_CHANNEL;

    stop_channel(channel);

    channel->CCR = DMA_CCR_PRIO;
    channel->CTR1 =
        DMA_CTR1_SDW_LOG2_0 |
        DMA_CTR1_SINC |
        DMA_CTR1_DDW_LOG2_0;
    channel->CTR2 =
        (ST1WIRE_DMA_OUTPUT_REQUEST << DMA_CTR2_REQSEL_Pos) |
        DMA_CTR2_DREQ;
    channel->CBR1 =
        ((uint32_t)count * sizeof(uint16_t)) & DMA_CBR1_BNDT;
    channel->CSAR = (uint32_t)(uintptr_t)buffer;
    channel->CDAR = (uint32_t)(uintptr_t)&TIM4->CCR3;
    channel->CLLR = 0U;
}


void st1wire_dma_start_capture(void)
{
    ST1WIRE_DMA_CAPTURE_CHANNEL->CCR |= DMA_CCR_EN;
}


void st1wire_dma_start_output(void)
{
    ST1WIRE_DMA_OUTPUT_CHANNEL->CCR |= DMA_CCR_EN;
}


uint16_t st1wire_dma_capture_remaining(void)
{
    return (uint16_t)
        ((ST1WIRE_DMA_CAPTURE_CHANNEL->CBR1 & DMA_CBR1_BNDT) /
         sizeof(uint16_t));
}
