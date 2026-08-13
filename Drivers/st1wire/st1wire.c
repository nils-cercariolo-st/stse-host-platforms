/**
 ******************************************************************************
 * \file    st1wire.c
 * \brief   ST1Wire protocol driver
 ******************************************************************************
 */

#include "st1wire.h"
#include "st1wire_phy.h"

static int8_t _st1wire_SendByte(
    uint8_t bus_addr,
    uint8_t speed,
    uint8_t byte);

static int8_t _st1wire_ReceiveByte(
    uint8_t bus_addr,
    uint8_t speed,
    uint8_t *rcv_byte);

static int8_t _st1wire_SendStart(
    uint8_t bus_addr,
    uint8_t speed);


static int8_t _st1wire_SendStart(
    uint8_t bus_addr,
    uint8_t speed)
{
    return (int8_t)st1wire_phy_send_start(
        bus_addr,
        speed);
}


static int8_t _st1wire_SendByte(
    uint8_t bus_addr,
    uint8_t speed,
    uint8_t byte)
{
    return (int8_t)st1wire_phy_send_byte(
        bus_addr,
        speed,
        byte);
}


static int8_t _st1wire_ReceiveByte(
    uint8_t bus_addr,
    uint8_t speed,
    uint8_t *rcv_byte)
{
    return (int8_t)st1wire_phy_receive_byte(
        bus_addr,
        speed,
        rcv_byte);
}


st1wire_ReturnCode_t st1wire_init(void)
{
    return st1wire_phy_init();
}


st1wire_ReturnCode_t st1wire_deinit(void)
{
    return st1wire_phy_deinit();
}


st1wire_ReturnCode_t st1wire_SendFrame(
    uint8_t bus_addr,
    uint8_t dev_addr,
    uint8_t speed,
    uint8_t *frame,
    uint16_t frame_length)
{
    uint8_t recv_byte;
    int8_t ret;
    uint16_t i;

#ifdef ST1WIRE_ENABLE_DEBUG_LOG
    ST1WIRE_DEBUG_PRINTF(
        "\n\r; ST1Wire %d >",
        bus_addr);
#endif

    ret = _st1wire_SendStart(
        bus_addr,
        speed);

    if (ret == ST1WIRE_OK)
    {
        if (dev_addr != 0U)
        {
            ret = _st1wire_SendByte(
                bus_addr,
                speed,
                dev_addr);

            if (ret != ST1WIRE_OK)
            {
#ifdef ST1WIRE_ENABLE_DEBUG_LOG
                ST1WIRE_DEBUG_PRINTF(
                    " ADDR ACK ERROR ");
#endif
                return ST1WIRE_BUS_ACK_ERROR;
            }

            st1wire_phy_inter_byte_delay(speed);
        }

#ifndef ST1WIRE_NO_LEN_FIX

        ret = _st1wire_SendByte(
            bus_addr,
            speed,
            ((frame_length >> 8U) & 0x07U));

        if (ret == ST1WIRE_OK)
        {
            st1wire_phy_inter_byte_delay(speed);

#endif

            ret = _st1wire_SendByte(
                bus_addr,
                speed,
                (frame_length & 0xFFU));

#ifndef ST1WIRE_NO_LEN_FIX
        }
#endif

        if (ret == ST1WIRE_OK)
        {
            for (i = 0U;
                 i < frame_length;
                 i++)
            {
                st1wire_phy_inter_byte_delay(speed);

                ret = _st1wire_SendByte(
                    bus_addr,
                    speed,
                    frame[i]);

                if (ret == ST1WIRE_BUS_ACK_ERROR)
                {
#ifdef ST1WIRE_ENABLE_DEBUG_LOG
                    ST1WIRE_DEBUG_PRINTF(
                        " DATA %d ACK ERROR ",
                        i);
#endif
                    break;
                }
            }

            if (ret == ST1WIRE_OK)
            {
                st1wire_phy_inter_byte_delay(speed);

                ret = _st1wire_ReceiveByte(
                    bus_addr,
                    speed,
                    &recv_byte);

                if ((ret == ST1WIRE_OK) &&
                    (recv_byte != 0x20U))
                {
#ifdef ST1WIRE_ENABLE_DEBUG_LOG
                    ST1WIRE_DEBUG_PRINTF(
                        " Frame ACK ERROR ");
#endif
                    ret = ST1WIRE_BUS_ACK_ERROR;
                }
            }
        }
    }

#ifdef ST1WIRE_ENABLE_DEBUG_LOG

    for (i = 0U;
         i < frame_length;
         i++)
    {
        ST1WIRE_DEBUG_PRINTF(
            " %02X",
            frame[i]);
    }

#endif

    st1wire_phy_frame_end_delay(speed);

    return (st1wire_ReturnCode_t)ret;
}


st1wire_ReturnCode_t st1wire_ReceiveFrame(
    uint8_t bus_addr,
    uint8_t dev_addr,
    uint8_t speed,
    uint8_t *frame,
    uint16_t *pframe_length)
{
    volatile uint8_t ret =
        ST1WIRE_BUS_ACK_ERROR;

    volatile uint16_t i;

    uint8_t rcv_byte;

    ret = _st1wire_SendStart(
        bus_addr,
        speed);

    if (ret == ST1WIRE_OK)
    {
        if (dev_addr != 0U)
        {
            ret = _st1wire_SendByte(
                bus_addr,
                speed,
                dev_addr);

            if (ret != ST1WIRE_OK)
            {
#ifdef ST1WIRE_ENABLE_DEBUG_LOG
                ST1WIRE_DEBUG_PRINTF(
                    "\n\r; ST1Wire %d < DEV ADDR ACK ERROR",
                    bus_addr);
#endif
                return ST1WIRE_BUS_ACK_ERROR;
            }

            st1wire_phy_receive_request_delay(speed);
        }

        ret = _st1wire_SendByte(
            bus_addr,
            speed,
            0x00U);

#ifndef ST1WIRE_NO_LEN_FIX

        if (ret == ST1WIRE_OK)
        {
            st1wire_phy_receive_request_delay(speed);

            ret = _st1wire_SendByte(
                bus_addr,
                speed,
                0x00U);
        }

#endif
    }

    if (ret != ST1WIRE_OK)
    {
#ifdef ST1WIRE_ENABLE_DEBUG_LOG
        ST1WIRE_DEBUG_PRINTF(
            "\n\r; ST1Wire %d < STATUS FRAME Send ERROR",
            bus_addr);
#endif
        return (st1wire_ReturnCode_t)ret;
    }

    st1wire_phy_inter_byte_delay(speed);

    ret = _st1wire_ReceiveByte(
        bus_addr,
        speed,
        &rcv_byte);

    if ((ret == ST1WIRE_OK) &&
        (rcv_byte == 0x20U))
    {
        st1wire_phy_inter_byte_delay(speed);

        ret = _st1wire_ReceiveByte(
            bus_addr,
            speed,
            &rcv_byte);

#ifndef ST1WIRE_NO_LEN_FIX

        if (ret == ST1WIRE_OK)
        {
            *pframe_length =
                ((uint16_t)rcv_byte << 8U);

            st1wire_phy_inter_byte_delay(speed);

            ret = _st1wire_ReceiveByte(
                bus_addr,
                speed,
                &rcv_byte);
        }

#endif

        if (ret == ST1WIRE_OK)
        {
            *pframe_length +=
                rcv_byte;

            for (i = 0U;
                 i < *pframe_length;
                 i++)
            {
                st1wire_phy_inter_byte_delay(speed);

                ret = _st1wire_ReceiveByte(
                    bus_addr,
                    speed,
                    frame + i);

                if (ret != ST1WIRE_OK)
                {
                    break;
                }
            }
        }
    }
    else
    {
#ifdef ST1WIRE_ENABLE_DEBUG_LOG
        ST1WIRE_DEBUG_PRINTF(
            "\n\r; ST1Wire %d < BYTE ACK ERROR",
            bus_addr);
#endif

        return ST1WIRE_BUS_ACK_ERROR;
    }

    st1wire_phy_frame_end_delay(speed);

#ifdef ST1WIRE_ENABLE_DEBUG_LOG

    ST1WIRE_DEBUG_PRINTF(
        "\n\r; ST1Wire %d <",
        bus_addr);

    for (i = 0U;
         i < *pframe_length;
         i++)
    {
        ST1WIRE_DEBUG_PRINTF(
            " %02X",
            frame[i]);
    }

#endif

    return (st1wire_ReturnCode_t)ret;
}


void st1wire_wake(uint8_t bus_addr)
{
    st1wire_phy_wake(bus_addr);
}


void st1wire_recovery(
    uint8_t bus_addr,
    uint8_t speed)
{
    st1wire_phy_recovery(
        bus_addr,
        speed);
}