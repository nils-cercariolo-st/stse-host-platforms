/**
 ******************************************************************************
 * \file    st1wire.h
 * \brief   ST1Wire protocol driver
 ******************************************************************************
 */

#ifndef ST1WIRE_H_
#define ST1WIRE_H_

#include <stdint.h>

/* General */
#define ST1WIRE_IDLE                        100U
#define ST1WIRE_RECEIVE_TIMEOUT             34464U

/* 2C timings */
#define ST1WIRE_2C_LONG_PULSE               14U
#define ST1WIRE_2C_SHORT_PULSE              4U
#define ST1WIRE_2C_WAIT_ACK                 4U
#define ST1WIRE_2C_ACK_PULSE                14U

#define ST1WIRE_2C_BIT_PERIOD               \
    (ST1WIRE_2C_LONG_PULSE + ST1WIRE_2C_SHORT_PULSE)

#define ST1WIRE_2C_START_PULSE              \
    (4U * ST1WIRE_2C_BIT_PERIOD)

#define ST1WIRE_2C_INTER_BYTE_DELAY         \
    (8U * ST1WIRE_2C_BIT_PERIOD)

#define ST1WIRE_2C_INTER_FRAME_DELAY        1000U

/* 3C timings */
#define ST1WIRE_3C_LONG_PULSE               13U
#define ST1WIRE_3C_SHORT_PULSE              7U
#define ST1WIRE_3C_ACK_PULSE                2U

#define ST1WIRE_3C_BIT_PERIOD               \
    (ST1WIRE_3C_LONG_PULSE + ST1WIRE_3C_SHORT_PULSE)

#define ST1WIRE_3C_START_PULSE              \
    (4U * ST1WIRE_2C_BIT_PERIOD)

#define ST1WIRE_3C_INTER_BYTE_DELAY         10U

//#define ST1WIRE_NO_LEN_FIX

/** \defgroup st1wire ST1Wire Layer
 *  \brief ST1Wire TIM + DMA interface
 *  @{
*/

typedef enum
{
    ST1WIRE_OK = 0x00,
    ST1WIRE_BUS_ARBITRATION_FAULT,
    ST1WIRE_BUS_ACK_ERROR,
    ST1WIRE_BUS_RECEIVE_TIMEOUT

} st1wire_ReturnCode_t;

/*!
 * \brief	Initialize ST1Wire bus
 * \result  ST1WIRE_OK on success ; st1wire_ReturnCode_t error code otherwise
 */
st1wire_ReturnCode_t st1wire_init(void);

/*!
 * \brief	De-initialise ST1Wire bus
 * \result  ST1WIRE_OK on success ; st1wire_ReturnCode_t error code otherwise
 */
st1wire_ReturnCode_t st1wire_deinit(void);

/*!
 * \brief					Send frame on ST1Wire bus
 * \param[in] bus_addr		Index of the ST1Wire bus
 * \param[in] speed			Communication speed (0 : slow	1: fast)
 * \param[in] *frame		Pointer to the applicative transmit buffer
 * \parame[in] frame_length	Length of the Frame to be sent
 */
st1wire_ReturnCode_t st1wire_SendFrame(
    uint8_t bus_addr,
    uint8_t dev_addr,
    uint8_t speed,
    uint8_t *frame,
    uint16_t frame_length);

    /*!
 * \brief					Receive frame on ST1Wire bus
 * \param[in] bus_addr		Index of the ST1Wire bus
 * \param[in] speed			Communication speed (0 : slow	1: fast)
 * \param[in] *frame		Pointer to the applicative receive buffer
 * \parame[in] frame_length	Pointer to the applicative receive frame length variable
 */
st1wire_ReturnCode_t st1wire_ReceiveFrame(
    uint8_t bus_addr,
    uint8_t dev_addr,
    uint8_t speed,
    uint8_t *frame,
    uint16_t *pframe_length);

void st1wire_wake(uint8_t bus_addr);

/*!
 * \brief					Recover ST1Wire device
 * \param[in] bus_addr		Index of the ST1Wire bus
 * \param[in] speed			Communication speed (0 : slow	1: fast)
 */
void st1wire_recovery(
    uint8_t bus_addr,
    uint8_t speed);

/*! @}*/

#endif /* ST1WIRE_H_ */