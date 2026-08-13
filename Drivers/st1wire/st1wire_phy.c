/**
 ******************************************************************************
 * \file    st1wire_phy.c
 * \brief   ST1Wire physical layer
 ******************************************************************************
 */

#include "st1wire_phy.h"
#include "st1wire_timer.h"
#include "st1wire_dma.h"
#include "st1wire_platform.h"

static volatile uint16_t
    capture_buffer[ST1WIRE_PHY_TX_CAPTURE_COUNT];

static uint16_t
    tx_dma_buffer[ST1WIRE_PHY_TX_EDGE_COUNT - 1U];

static uint16_t
    rx_output_buffer[3];


static uint16_t get_short_time(uint8_t speed)
{
    return (speed == 0U)
        ? ST1WIRE_2C_SHORT_PULSE
        : ST1WIRE_3C_SHORT_PULSE;
}


static uint16_t get_long_time(uint8_t speed)
{
    return (speed == 0U)
        ? ST1WIRE_2C_LONG_PULSE
        : ST1WIRE_3C_LONG_PULSE;
}


static uint16_t get_ack_time(uint8_t speed)
{
    return (speed == 0U)
        ? ST1WIRE_2C_ACK_PULSE
        : ST1WIRE_3C_ACK_PULSE;
}


static void hw_stop(void)
{
    st1wire_timer_stop();
    st1wire_dma_stop();
    st1wire_timer_release_pin();
}


static int8_t idle_detection(uint8_t bus_addr)
{
    st1wire_platform_start_timeout(
        ST1WIRE_IDLE);

    while (st1wire_platform_io_get(bus_addr))
    {
        if (st1wire_platform_is_timeout_exceeded())
        {
            return ST1WIRE_BUS_ARBITRATION_FAULT;
        }
    }

    return ST1WIRE_OK;
}


static int8_t wait_capture(void)
{
    st1wire_platform_start_timeout(
        ST1WIRE_PHY_TIMEOUT_US);

    while (st1wire_dma_capture_remaining() != 0U)
    {
        if (st1wire_platform_is_timeout_exceeded())
        {
            return ST1WIRE_BUS_RECEIVE_TIMEOUT;
        }
    }

    return ST1WIRE_OK;
}


st1wire_ReturnCode_t st1wire_phy_init(void)
{
    st1wire_platform_init();
    st1wire_timer_init();
    st1wire_dma_init();

    return ST1WIRE_OK;
}


st1wire_ReturnCode_t st1wire_phy_deinit(void)
{
    hw_stop();

    st1wire_timer_deinit();
    st1wire_dma_deinit();
    st1wire_platform_deinit();

    return ST1WIRE_OK;
}


st1wire_ReturnCode_t st1wire_phy_send_start(
    uint8_t bus_addr,
    uint8_t speed)
{
    uint16_t start_time;

    start_time = (speed == 0U)
        ? ST1WIRE_2C_START_PULSE
        : ST1WIRE_3C_START_PULSE;

    st1wire_platform_io_in(bus_addr);

    while (!idle_detection(bus_addr))
    {
    }

    if (st1wire_platform_io_get(bus_addr) == 0U)
    {
        return ST1WIRE_BUS_ARBITRATION_FAULT;
    }

    st1wire_platform_io_out(bus_addr);
    st1wire_platform_io_clear(bus_addr);

    st1wire_platform_delay(start_time);

    st1wire_platform_io_set(bus_addr);

    if (speed == 0U)
    {
        st1wire_platform_delay(
            ST1WIRE_2C_INTER_BYTE_DELAY);
    }

    return ST1WIRE_OK;
}


st1wire_ReturnCode_t st1wire_phy_send_byte(
    uint8_t bus_addr,
    uint8_t speed,
    uint8_t byte)
{
    uint16_t edge[ST1WIRE_PHY_TX_EDGE_COUNT];

    uint16_t short_t;
    uint16_t long_t;
    uint16_t time;
    uint16_t index;
    uint16_t i;

    (void)bus_addr;

    short_t = get_short_time(speed);
    long_t = get_long_time(speed);

    time = 0U;
    index = 0U;

    /* Sync */
    time += short_t;
    edge[index++] = time;

    time += long_t;
    edge[index++] = time;

    /* Byte, MSB first */
    for (i = 0U; i < 8U; i++)
    {
        if ((byte & (1U << (7U - i))) != 0U)
        {
            time += long_t;
            edge[index++] = time;

            time += short_t;
            edge[index++] = time;
        }
        else
        {
            time += short_t;
            edge[index++] = time;

            time += long_t;
            edge[index++] = time;
        }
    }

    for (i = 1U;
         i < ST1WIRE_PHY_TX_EDGE_COUNT;
         i++)
    {
        tx_dma_buffer[i - 1U] = edge[i];
    }

    st1wire_timer_prepare();

    st1wire_dma_config_capture(
        capture_buffer,
        ST1WIRE_PHY_TX_CAPTURE_COUNT);

    st1wire_dma_config_output(
        tx_dma_buffer,
        ST1WIRE_PHY_TX_EDGE_COUNT - 1U);

    st1wire_timer_set_compare(edge[0]);
    st1wire_timer_enable_dma_requests();

    st1wire_dma_start_capture();
    st1wire_dma_start_output();

    st1wire_timer_start();

    if (wait_capture() != ST1WIRE_OK)
    {
        hw_stop();

        return ST1WIRE_BUS_ACK_ERROR;
    }

    hw_stop();

    if (capture_buffer[19] <=
        capture_buffer[18])
    {
        return ST1WIRE_BUS_ACK_ERROR;
    }

    return ST1WIRE_OK;
}


st1wire_ReturnCode_t st1wire_phy_receive_byte(
    uint8_t bus_addr,
    uint8_t speed,
    uint8_t *byte)
{
    uint16_t short_t;
    uint16_t long_t;
    uint16_t ack_t;
    uint16_t period;

    uint16_t sync_fall;
    uint16_t sync_rise;
    uint16_t data_end;
    uint16_t ack_start;
    uint16_t ack_end;

    uint16_t previous_rising;
    uint16_t falling;
    uint16_t rising;
    uint16_t high_time;
    uint16_t low_time;

    uint8_t value;
    uint8_t i;

    (void)bus_addr;

    short_t = get_short_time(speed);
    long_t = get_long_time(speed);
    ack_t = get_ack_time(speed);

    period = short_t + long_t;

    sync_fall = long_t;
    sync_rise = 2U * long_t;

    data_end =
        sync_rise +
        (8U * period);

    ack_start =
        data_end + 1U;

    ack_end =
        ack_start + ack_t;

    rx_output_buffer[0] = sync_rise;
    rx_output_buffer[1] = ack_start;
    rx_output_buffer[2] = ack_end;

    st1wire_timer_prepare();

    st1wire_dma_config_capture(
        capture_buffer,
        ST1WIRE_PHY_RX_CAPTURE_COUNT);

    st1wire_dma_config_output(
        rx_output_buffer,
        3U);

    st1wire_timer_set_compare(sync_fall);
    st1wire_timer_enable_dma_requests();

    st1wire_dma_start_capture();
    st1wire_dma_start_output();

    st1wire_timer_start();

    if (wait_capture() != ST1WIRE_OK)
    {
        hw_stop();

        return ST1WIRE_BUS_RECEIVE_TIMEOUT;
    }

    st1wire_platform_start_timeout(
        ST1WIRE_PHY_TIMEOUT_US);

    while (st1wire_timer_get_counter() <= ack_end)
    {
        if (st1wire_platform_is_timeout_exceeded())
        {
            hw_stop();

            return ST1WIRE_BUS_RECEIVE_TIMEOUT;
        }
    }

    hw_stop();

    value = 0U;

    previous_rising =
        capture_buffer[1];

    for (i = 0U; i < 8U; i++)
    {
        falling =
            capture_buffer[2U + (2U * i)];

        rising =
            capture_buffer[3U + (2U * i)];

        high_time =
            falling - previous_rising;

        low_time =
            rising - falling;

        value <<= 1U;

        if (high_time > low_time)
        {
            value |= 1U;
        }

        previous_rising = rising;
    }

    *byte = value;

    return ST1WIRE_OK;
}


void st1wire_phy_inter_byte_delay(
    uint8_t speed)
{
    if (speed == 0U)
    {
        st1wire_platform_delay(
            ST1WIRE_2C_INTER_BYTE_DELAY);
    }
    else
    {
        st1wire_platform_delay(
            ST1WIRE_3C_INTER_BYTE_DELAY);
    }
}


void st1wire_phy_receive_request_delay(
    uint8_t speed)
{
    if (speed == 0U)
    {
        st1wire_platform_delay(
            ST1WIRE_2C_INTER_FRAME_DELAY);
    }
    else
    {
        st1wire_platform_delay(
            ST1WIRE_3C_INTER_BYTE_DELAY);
    }
}


void st1wire_phy_frame_end_delay(
    uint8_t speed)
{
    if (speed == 0U)
    {
        st1wire_platform_delay(
            ST1WIRE_2C_INTER_FRAME_DELAY);
    }
    else
    {
        st1wire_platform_delay(
            ST1WIRE_3C_INTER_BYTE_DELAY);
    }
}


void st1wire_phy_wake(uint8_t bus_addr)
{
    st1wire_platform_wake(bus_addr);
}


void st1wire_phy_recovery(
    uint8_t bus_addr,
    uint8_t speed)
{
    if (speed != 0U)
    {
        return;
    }

    st1wire_platform_io_clear(bus_addr);
    st1wire_platform_delay(100000U);

    st1wire_platform_io_set(bus_addr);
    st1wire_platform_delay(100000U);
}