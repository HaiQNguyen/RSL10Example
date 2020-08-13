/* ----------------------------------------------------------------------------
 * Copyright (c) 2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 * ----------------------------------------------------------------------------
 * uart.c
 * - Support code for handling DMA-based UART data transfers
 * ----------------------------------------------------------------------------
 * $Revision: 1.9 $
 * $Date: 2018/02/27 14:31:40 $
 * ------------------------------------------------------------------------- */

#include "uart.h"

/* UART buffers */
uint32_t *gUARTTXData;
uint32_t gNextData;

/* ----------------------------------------------------------------------------
 * Function      : void UART_Initialize()
 * ----------------------------------------------------------------------------
 * Description   : Initialize the UART interface and supporting DMA channels
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void UART_Initialize(void)
{
    static uint32_t UARTTXBuffer[TX_BUFFER_SIZE];
    static uint32_t UARTRXBuffer[RX_BUFFER_SIZE];

    /* Setup a UART interface on DIO5 (TX) and DIO4 (RX), routing data using
     * DMA channels 0 (TX), 1 (RX) to two defined buffers - don't setup DMA
     * channel 0 since there isn't any UART data to transmit yet. */
    Sys_DMA_ChannelDisable(DMA_TX_NUM);
    Sys_DMA_ChannelDisable(DMA_RX_NUM);

    gUARTTXData = UARTTXBuffer;
    gNextData   = 0;

    Sys_UART_DIOConfig(DIO_6X_DRIVE | DIO_WEAK_PULL_UP | DIO_LPF_ENABLE,
                       UART_TX, UART_RX);
    Sys_UART_Enable(SystemCoreClock, UART_BAUD_RATE, UART_DMA_MODE_ENABLE);

    Sys_DMA_ClearChannelStatus(DMA_RX_NUM);
    Sys_DMA_ChannelConfig(DMA_RX_NUM, DMA_RX_CFG, RX_BUFFER_SIZE, 0,
                          (uint32_t)&UART->RX_DATA, (uint32_t)UARTRXBuffer);
}

/* ----------------------------------------------------------------------------
 * Function      : uint32_t UART_EmptyRXBuffer(uint8_t *data)
 * ----------------------------------------------------------------------------
 * Description   : Copy data from the UART RX buffer, and return this data
 *                 along with the length of the data returned.
 * Inputs        : None
 * Outputs       : - data           - Pointer to the buffer to transfer data to
 *                 - Return value   - Number of values in the UART RX buffer
 * Assumptions   : It is okay to re-start the UART RX buffer count after
 *                 calling this function
 * ------------------------------------------------------------------------- */
uint32_t UART_EmptyRXBuffer(uint8_t *data)
{
    uint32_t size;
    uint32_t temp;
    unsigned int i;

    /* Check if the start interrupt event occurred and then setup the RX next
     * data pointer */
    if (((Sys_DMA_Get_ChannelStatus(DMA_RX_NUM) & DMA_START_INT_STATUS) != 0) &&
        (gNextData == 0))
    {
        gNextData = DMA->DEST_BASE_ADDR[DMA_RX_NUM];
    }

    /* Set temp so that processing in this function is atomic */
    temp = DMA->NEXT_DEST_ADDR[DMA_RX_NUM];

    /* If gNextData is not initialized or matches the next address location
     * to be written return a size of 0. */
    if (gNextData == temp)
    {
        return (0);
    }

    /* Otherwise convert the size from the input word size (32 bits) to the
     * expected word size (8 bits) for UART transfers, and copy the received
     * words from the DMA buffer. Keep gNextData up to date as we read data from
     * the buffer so its set for the next time we enter this function */
    if (gNextData < temp)
    {
        /* Handle copying out data that does not wrap around the end of the
         * buffer where the DMA is copying data to. */
        size = (temp - gNextData) >> 2;
        for (i = 0; i < size; i++)
        {
            data[i]   = *(uint8_t *)gNextData;
            gNextData = gNextData + 4;
        }
    }
    else
    {
        /* Handle copying out data that wraps around the end of the
         * buffer where the DMA is copying data to. */
        size = (temp + ((RX_BUFFER_SIZE << 2) - gNextData)) >> 2;
        temp = ((RX_BUFFER_SIZE << 2) -
                (gNextData - DMA->DEST_BASE_ADDR[DMA_RX_NUM])) >> 2;
        for (i = 0; i < temp; i++)
        {
            data[i]   = *(uint8_t *)gNextData;
            gNextData = gNextData + 4;
        }
        gNextData = DMA->DEST_BASE_ADDR[DMA_RX_NUM];
        if (size - temp > 0)
        {
            for (i = 0; i < (size - temp); i++)
            {
                data[temp + i] = *(uint8_t *)gNextData;
                gNextData = gNextData + 4;
            }
        }
    }

    return (size);
}

/* ----------------------------------------------------------------------------
 * Function      : void UART_FillTXBuffer(uint32_t txSize,
 *                                        uint8_t *data)
 * ----------------------------------------------------------------------------
 * Description   : Fill the UART TX buffer with data from the specified array.
 * Inputs        : - txSize     - Amount of data to transfer
 *                 - data       - Pointer to the data to be transferred
 * Outputs       : return value - Error indication; returns UART_ERRNO_OVERFLOW
 *                                if the data doesn't fit in the current DMA
 *                                transfer, UART_ERRNO_NONE otherwise.
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
unsigned int UART_FillTXBuffer(uint32_t txSize, uint8_t *data)
{
    uint32_t *temp;
    uint32_t tempMask;
    unsigned int i;

    /* Check if the TX buffer is currently being used to transfer data (use
     * the base DMA channel, since we're using that for UART TX) */
    if (DMA_CTRL0[DMA_TX_NUM].ENABLE_ALIAS == DMA_ENABLE_BITBAND)
    {
        if (DMA_CTRL1[DMA_TX_NUM].TRANSFER_LENGTH_SHORT + txSize >
            TX_BUFFER_SIZE)
        {
            return (UART_ERRNO_OVERFLOW);
        }

        /* Append this data to the currently transferring data and update
         * the transfer length if the transfer is still active (unpacking
         * the data) */
        temp = gUARTTXData + (DMA_CTRL1[DMA_TX_NUM].TRANSFER_LENGTH_SHORT);
        for (i = 0; i < txSize; i++)
        {
            *temp = data[i];
            temp++;
        }

        /* Create a short critical section to make sure that we don't extend
         * the transfer, and then decide that it wasn't extended because the
         * DMA is disabled (as may occur if an interrupt occured between this
         * and the following instruction). */
        tempMask = __get_PRIMASK();
        __set_PRIMASK(1);
        DMA_CTRL1[DMA_TX_NUM].TRANSFER_LENGTH_SHORT =
            (DMA_CTRL1[DMA_TX_NUM].TRANSFER_LENGTH_SHORT + txSize);

        /* If the transfer is still going after extending the transfer,
         * the new data will be output as part of the existing transfer.
         * Otherwise, we need to fall through and start a new transfer to
         * send this data. */
        if (DMA_CTRL0[DMA_TX_NUM].ENABLE_ALIAS == DMA_ENABLE_BITBAND)
        {
            __set_PRIMASK(tempMask);
            return (UART_ERRNO_NONE);
        }
        __set_PRIMASK(tempMask);
    }

    /* If the DMA transfer was not active, or finished while trying to append
     * to the buffer, create a new DMA controlled transfer */
    if (txSize > TX_BUFFER_SIZE)
    {
        return (UART_ERRNO_OVERFLOW);
    }

    /* Copy the data passed in to UARTTXData (unpacking the data) */
    temp = gUARTTXData;
    for (i = 0; i < txSize; i++)
    {
        *temp = data[i];
        temp++;
    }

    /* Transfer the data */
    Sys_DMA_ChannelConfig(DMA_TX_NUM, DMA_TX_CFG, txSize, 0,
                          (uint32_t)gUARTTXData, (uint32_t)&UART->TX_DATA);
    Sys_DMA_ClearChannelStatus(DMA_TX_NUM);
    return (UART_ERRNO_NONE);
}
