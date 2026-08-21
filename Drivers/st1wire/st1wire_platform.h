/**
 ******************************************************************************
 * \file   st1wire_platform.h
 * \brief  STM32H533 ST1Wire platform abstraction
 ******************************************************************************
 */

#ifndef ST1WIRE_PLATFORM_H_
#define ST1WIRE_PLATFORM_H_

#include <stdint.h>

#ifdef USE_FREERTOS
#define ST1WIRE_START_CRITICAL_SECTION \
    vTaskSuspendAll();                 \
    __disable_irq();
#define ST1WIRE_END_CRITICAL_SECTION \
    xTaskResumeAll();                \
    __enable_irq();
#else
#define ST1WIRE_START_CRITICAL_SECTION __disable_irq();
#define ST1WIRE_END_CRITICAL_SECTION __enable_irq();
#endif /* USE_FREERTOS */

void st1wire_platform_init(void);
void st1wire_platform_deinit(void);
void st1wire_platform_io_set(uint8_t bus_addr);
void st1wire_platform_io_clear(uint8_t bus_addr);
uint8_t st1wire_platform_io_get(uint8_t bus_addr);
void st1wire_platform_io_in(uint8_t bus_addr);
void st1wire_platform_io_out(uint8_t bus_addr);
void st1wire_platform_wake(uint8_t bus_addr);
void st1wire_platform_delay(uint32_t delay);
void st1wire_platform_start_timeout(uint32_t timeout);
int8_t st1wire_platform_is_timeout_exceeded(void);

#endif /* ST1WIRE_PLATFORM_H_ */
