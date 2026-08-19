# STM32H523CC host platform

Bare-metal host-platform drivers for the STM32H523CC. This branch ports the
STM32L452 timer/DMA ST1Wire implementation to the STM32H5 GPDMA architecture.
The ST1Wire timing path is intended for 3-contact (3C) mode; the retained 2C
protocol branches are not a supported or validated target.

## Pin and peripheral assignment

| Function | STM32H523CC resource | Notes |
| --- | --- | --- |
| ST1Wire | PB8, AF2 | Open-drain, external pull-up required |
| ST1Wire transmit | TIM4_CH3 | Output-compare toggle |
| ST1Wire receive | TIM4_CH4 | Indirect TI3 capture on both edges |
| ST1Wire capture DMA | GPDMA1 channel 0, request 86 | TIM4_CH4 to memory |
| ST1Wire output DMA | GPDMA1 channel 1, request 85 | Memory to TIM4_CH3 |
| I2C1 | PB6/PB7, AF4 | Moved from PB8/PB9 to avoid ST1Wire conflict |
| USART2 | PA2/PA3, AF7 | Polling driver |
| Microsecond delay/timeout | TIM6 | 16-bit timer at 1 MHz |
| Millisecond delay/timeout | TIM2 | 32-bit timer at 1 MHz |
| Power controls | PB0, PC0, PC1 | Open-drain outputs |

TIM4 runs with a 1 MHz counter, so the existing 3C values remain expressed in
microseconds. The DMA implementation programs H5 GPDMA block lengths in bytes
and converts the remaining byte count back to captured halfwords for the PHY.

## CMSIS and memory

The branch includes STM32H523 CMSIS device headers and startup files for GCC,
Arm/Keil, and IAR. GCC and IAR linker files target the CC density:

- 256 KiB flash at `0x08000000`
- 272 KiB SRAM at `0x20000000`
- TrustZone disabled memory aliases
- HSI at its reset frequency of 64 MHz

For GCC, include `Core/CMSIS/Include` and
`Core/CMSIS/Device/ST/STM32H5xx/Include`, and use
`Core/CMSIS/Device/ST/STM32H5xx/Source/gcc/startup_stm32h523xx.s` plus
`Core/CMSIS/Device/ST/STM32H5xx/Source/system_stm32h5xx.c`.

## Hardware validation

This port is compile-checked only because target hardware is not yet available.
First-board testing should scope PB8 and verify the SOF, sync/data duty cycles,
ACK capture count, and open-drain release level before running long STSELib
transactions.

Primary references:

- [STM32H523CC datasheet](https://www.st.com/resource/en/datasheet/stm32h523cc.pdf)
- [STM32H523/533 reference manual RM0481](https://www.st.com/resource/en/reference_manual/rm0481-stm32h533-stm32h563-stm32h573-and-stm32h562-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STSAFE-L010 datasheet](https://www.st.com/resource/en/datasheet/stsafe-l010.pdf)
- [STM32H5 CMSIS device package](https://github.com/STMicroelectronics/cmsis-device-h5)
- [STM32H5 HAL/LL DMA request definitions](https://github.com/STMicroelectronics/stm32h5xx-hal-driver)
