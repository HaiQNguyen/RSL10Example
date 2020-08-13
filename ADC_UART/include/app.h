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
 * app.h
 * - Overall application header file for the ADC UART with DMA sample
 *   application.
 * ------------------------------------------------------------------------- */

#ifndef APP_H
#define APP_H

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
#include "uart.h"

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/
#define CONCAT(x, y)                    x##y
#define BATMON_CH(x)                    CONCAT(BATMON_CH, x)
#define LED_DIO                         6
#define ADC_GND_CHANNEL                 0
#define ADC_CHANNEL                     6
#define ADC_GAIN                        8192
#define THRESHOLD                       1.6
#define THRESHOLD_CFG                   ((uint32_t)(THRESHOLD / (2 * 0.0078)))
#define ADC_FILTER_COUNTS               100

/* DIO number that is used for easy re-flashing (recovery mode) */
#define RECOVERY_DIO                    12

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/

extern void ADC_BATMON_IRQHandler(void);

void Initialize(void);

void Send_ADC_Value(void);

void Send_VBAT_Error(void);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif    /* ifdef __cplusplus */

#endif    /* APP_H */
