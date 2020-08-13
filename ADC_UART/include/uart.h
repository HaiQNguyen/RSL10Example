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
 * uart.h
 * - Header file for the DMA-basd UART data tranfer support code
 * ------------------------------------------------------------------------- */

#ifndef UART_H
#define UART_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif    /* ifdef __cplusplus */

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/
#include <rsl10.h>

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/
#define UART_TX                         5
#define UART_RX                         4
#define UART_BAUD_RATE                  115200
#define RX_BUFFER_SIZE                  0x100
#define TX_BUFFER_SIZE                  0x200

#define DMA_RX_NUM                      1
#define DMA_RX_CFG                      (DMA_ENABLE                 | \
                                         DMA_ADDR_CIRC              | \
                                         DMA_TRANSFER_P_TO_M        | \
                                         DMA_PRIORITY_0             | \
                                         DMA_DISABLE_INT_DISABLE    | \
                                         DMA_ERROR_INT_DISABLE      | \
                                         DMA_COMPLETE_INT_DISABLE   | \
                                         DMA_COUNTER_INT_DISABLE    | \
                                         DMA_START_INT_ENABLE       | \
                                         DMA_LITTLE_ENDIAN          | \
                                         DMA_SRC_ADDR_STATIC        | \
                                         DMA_SRC_WORD_SIZE_32       | \
                                         DMA_SRC_UART               | \
                                         DMA_DEST_ADDR_INC          | \
                                         DMA_DEST_ADDR_STEP_SIZE_1  | \
                                         DMA_DEST_WORD_SIZE_32)

#define DMA_TX_NUM                      0
#define DMA_TX_CFG                      (DMA_ENABLE                 | \
                                         DMA_ADDR_LIN               | \
                                         DMA_TRANSFER_M_TO_P        | \
                                         DMA_PRIORITY_0             | \
                                         DMA_DISABLE_INT_DISABLE    | \
                                         DMA_ERROR_INT_DISABLE      | \
                                         DMA_COMPLETE_INT_DISABLE   | \
                                         DMA_COUNTER_INT_DISABLE    | \
                                         DMA_START_INT_DISABLE      | \
                                         DMA_LITTLE_ENDIAN          | \
                                         DMA_SRC_ADDR_INC           | \
                                         DMA_SRC_WORD_SIZE_32       | \
                                         DMA_SRC_ADDR_STEP_SIZE_1   | \
                                         DMA_DEST_ADDR_STATIC       | \
                                         DMA_DEST_UART              | \
                                         DMA_DEST_WORD_SIZE_32)

/* Define error codes */
#define UART_ERRNO_NONE                 0
#define UART_ERRNO_OVERFLOW             1

/* ----------------------------------------------------------------------------
 * Function Prototypes
 * --------------------------------------------------------------------------*/
extern void UART_Initialize(void);

extern uint32_t UART_EmptyRXBuffer(uint8_t *data);

extern unsigned int UART_FillTXBuffer(uint32_t txSize, uint8_t *data);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* UART_H */
