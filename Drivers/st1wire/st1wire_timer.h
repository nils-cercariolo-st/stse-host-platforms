/**
 ******************************************************************************
 * \file    st1wire_timer.h
 * \brief   STM32L452 ST1Wire timer backend
 ******************************************************************************
 */

#ifndef ST1WIRE_TIMER_H_
#define ST1WIRE_TIMER_H_

#include <stdint.h>

#define ST1WIRE_TIMER_FREQUENCY_HZ           1000000UL

void st1wire_timer_init(void);

void st1wire_timer_deinit(void);

void st1wire_timer_prepare(void);

void st1wire_timer_stop(void);

void st1wire_timer_release_pin(void);

void st1wire_timer_set_compare(
    uint16_t compare);

void st1wire_timer_enable_dma_requests(void);

void st1wire_timer_start(void);

uint16_t st1wire_timer_get_counter(void);

#endif /* ST1WIRE_TIMER_H_ */