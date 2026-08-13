/**
 ******************************************************************************
 * \file    st1wire_phy.h
 * \brief   ST1Wire physical layer
 ******************************************************************************
 */

#ifndef ST1WIRE_PHY_H_
#define ST1WIRE_PHY_H_

#include "st1wire.h"

#define ST1WIRE_PHY_TX_EDGE_COUNT            18U
#define ST1WIRE_PHY_TX_CAPTURE_COUNT         20U
#define ST1WIRE_PHY_RX_CAPTURE_COUNT         18U

#define ST1WIRE_PHY_TIMEOUT_US               2000U

st1wire_ReturnCode_t st1wire_phy_init(void);

st1wire_ReturnCode_t st1wire_phy_deinit(void);

st1wire_ReturnCode_t st1wire_phy_send_start(
    uint8_t bus_addr,
    uint8_t speed);

st1wire_ReturnCode_t st1wire_phy_send_byte(
    uint8_t bus_addr,
    uint8_t speed,
    uint8_t byte);

st1wire_ReturnCode_t st1wire_phy_receive_byte(
    uint8_t bus_addr,
    uint8_t speed,
    uint8_t *byte);

void st1wire_phy_inter_byte_delay(
    uint8_t speed);

void st1wire_phy_receive_request_delay(
    uint8_t speed);

void st1wire_phy_frame_end_delay(
    uint8_t speed);

void st1wire_phy_wake(
    uint8_t bus_addr);

void st1wire_phy_recovery(
    uint8_t bus_addr,
    uint8_t speed);

#endif /* ST1WIRE_PHY_H_ */