/**
 ******************************************************************************
 * \file    st1wire_dma.h
 * \brief   STM32L452 ST1Wire DMA backend
 ******************************************************************************
 */

#ifndef ST1WIRE_DMA_H_
#define ST1WIRE_DMA_H_

#include <stdint.h>

#define ST1WIRE_DMA_CAPTURE_REQUEST          7U
#define ST1WIRE_DMA_OUTPUT_REQUEST           7U

void st1wire_dma_init(void);

void st1wire_dma_deinit(void);

void st1wire_dma_stop(void);

void st1wire_dma_config_capture(
    volatile uint16_t *buffer,
    uint16_t count);

void st1wire_dma_config_output(
    const uint16_t *buffer,
    uint16_t count);

void st1wire_dma_start_capture(void);

void st1wire_dma_start_output(void);

uint16_t st1wire_dma_capture_remaining(void);

#endif /* ST1WIRE_DMA_H_ */