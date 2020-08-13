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
 *
 * ----------------------------------------------------------------------------
 * app.c
 * - ADC UART application that measure and monitor VBAT.
 * - The value of VBAT is sent by UART to a terminal.
 * - If the value of VBAT goes lower than 1.6 V a error message is sent.
 * - As battery monitor is only on Channel 6 or 7, the ADC_CHANNEL need to 6 or
 *   7.
 * ----------------------------------------------------------------------------
 * $Revision: 1.23 $
 * $Date: 2019/12/03 22:30:29 $
 * ------------------------------------------------------------------------- */

#include <app.h>
#include <stdio.h>
#include <printf.h>

/* ----------------------------------------------------------------------------
 * Global Variables Declaration
 * ------------------------------------------------------------------------- */
volatile uint8_t data_ready_flag;
volatile uint8_t bat_error_flag;
volatile float adc_value;

/* ----------------------------------------------------------------------------
 * Function      : void ADC_BATMON_IRQHandler(void)
 * ----------------------------------------------------------------------------
 * Description   : Handle ADC and BATMON interrupts. When the interrupt is from
 *                 ADC add the ADC value to the accumulator adc_value and
 *                 increment the counter. When counter reach 100 calculate the
 *                 average and set data_ready.
 *                 When the interrupt is from battery monitor set bat_error.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void ADC_BATMON_IRQHandler(void)
{
    static uint32_t adc_samples_count = 0;
    static uint32_t adc_filter_sum    = 0.0f;

    /* Get status of ADC */
    uint32_t adc_status = Sys_ADC_Get_BATMONStatus();
    if ((adc_status & (1 << ADC_BATMON_STATUS_BATMON_ALARM_STAT_Pos)) ==
        BATMON_ALARM_TRUE)
    {
        /* Battery monitor alarm status is set */
        bat_error_flag = 1;

        /* Clear the battery monitor status and counter */
        Sys_ADC_Clear_BATMONStatus();
        uint32_t dummy = ADC->BATMON_COUNT_VAL;
    }
    else
    {
        adc_filter_sum = adc_filter_sum + ADC->DATA_TRIM_CH[ADC_CHANNEL];
        adc_samples_count++;
        if (adc_samples_count == ADC_FILTER_COUNTS)
        {
            adc_samples_count = 0;
            adc_value = (adc_filter_sum + (float)ADC_FILTER_COUNTS / 2.0f) /
                        (float)ADC_FILTER_COUNTS;
            adc_filter_sum    = 0;
            data_ready_flag   = 1;
        }
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the system by disabling interrupts, switching to
 *                 the 8 MHz clock (divided from the 48 MHz crystal),
 *                 configuring the DIOs required for the UART interface,
 *                 configuring ADC to measure VBAT/2, enabling battery monitor
 *                 and enabling interrupts.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Initialize(void)
{
    /* Mask all interrupts */
    __set_PRIMASK(PRIMASK_DISABLE_INTERRUPTS);

    /* Disable all existing interrupts, clearing all pending source */
    Sys_NVIC_DisableAllInt();
    Sys_NVIC_ClearAllPendingInt();

    /* Test DIO12 to pause the program to make it easy to re-flash */
    DIO->CFG[RECOVERY_DIO] = DIO_MODE_INPUT  | DIO_WEAK_PULL_UP |
                             DIO_LPF_DISABLE | DIO_6X_DRIVE;
    while (DIO_DATA->ALIAS[RECOVERY_DIO] == 0);

    /* Prepare the 48 MHz crystal
     * Start and configure VDDRF */
    ACS_VDDRF_CTRL->ENABLE_ALIAS = VDDRF_ENABLE_BITBAND;
    ACS_VDDRF_CTRL->CLAMP_ALIAS  = VDDRF_DISABLE_HIZ_BITBAND;

    /* Wait until VDDRF supply has powered up */
    while (ACS_VDDRF_CTRL->READY_ALIAS != VDDRF_READY_BITBAND);

    /* Enable RF power switches */
    SYSCTRL_RF_POWER_CFG->RF_POWER_ALIAS   = RF_POWER_ENABLE_BITBAND;

    /* Remove RF isolation */
    SYSCTRL_RF_ACCESS_CFG->RF_ACCESS_ALIAS = RF_ACCESS_ENABLE_BITBAND;

    /* Start the 48 MHz oscillator without changing the other register bits */
    RF->XTAL_CTRL = ((RF->XTAL_CTRL & ~XTAL_CTRL_DISABLE_OSCILLATOR) |
                     XTAL_CTRL_REG_VALUE_SEL_INTERNAL);

    /* Enable 48 MHz oscillator divider to generate an 8 MHz clock. */
    RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_6_BYTE;

    /* Wait until 48 MHz oscillator is started */
    while (RF_REG39->ANALOG_INFO_CLK_DIG_READY_ALIAS !=
           ANALOG_INFO_CLK_DIG_READY_BITBAND);

    /* Switch to (divided 48 MHz) oscillator clock */
    Sys_Clocks_SystemClkConfig(JTCK_PRESCALE_1 |
                               EXTCLK_PRESCALE_1 |
                               SYSCLK_CLKSRC_RFCLK);

    /* Setup DIO6 as a GPIO output */
    Sys_DIO_Config(LED_DIO, DIO_MODE_GPIO_OUT_0);

    /* Initialize the UART and associated DMA */
    UART_Initialize();

    /* Set the ADC configuration */
    Sys_ADC_Set_Config(ADC_VBAT_DIV2_NORMAL | ADC_NORMAL | ADC_PRESCALE_1280H);

    /* Set the battery monitor interrupt configuration */
    Sys_ADC_Set_BATMONIntConfig(INT_EBL_ADC |
                                ADC_CHANNEL <<
                                ADC_BATMON_INT_ENABLE_ADC_INT_CH_NUM_Pos |
                                INT_EBL_BATMON_ALARM);

    /* Set the battery monitor configuration, use channel ADC_CHANNEL to battery
     * monitoring. */
    Sys_ADC_Set_BATMONConfig((100 << ADC_BATMON_CFG_ALARM_COUNT_VALUE_Pos) |
                             (THRESHOLD_CFG <<
                              ADC_BATMON_CFG_SUPPLY_THRESHOLD_Pos) |
                             BATMON_CH(ADC_CHANNEL));

    /* Configure ADC_CHANNEL input selection to VBAT/2 */
    Sys_ADC_InputSelectConfig(ADC_CHANNEL, ADC_POS_INPUT_VBAT_DIV2 |
                              ADC_NEG_INPUT_GND);

    /* Configure both input selection for an ADC channel to GND so the OFFSET is
     * subtracted automatically to result. */
    Sys_ADC_InputSelectConfig(ADC_GND_CHANNEL, ADC_POS_INPUT_GND |
                              ADC_NEG_INPUT_GND);

    /* Enable interrupts */
    NVIC_EnableIRQ(ADC_BATMON_IRQn);

    /* Unmask all interrupts */
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
}

/* ----------------------------------------------------------------------------
 * Function      : void Send_ADC_Value (void)
 * ----------------------------------------------------------------------------
 * Description   : Create a buffer with "ADC VALUE = x.xxx V" where x is the
 *                 ADC value read by the ADC and send it to the UART. The number
 *                 of digits of the mantissa is defined by NUM_DIGITS. The value
 *                 add to the buffer is truncated.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Send_ADC_Value(void)
{
    uint8_t size;
    float adc_in_volts;
    char buffer[32];

    /* Multiply by 2 as we measure VBAT/2 and divide by a gain factor to convert
     * the value from ADC. */
    adc_in_volts = (adc_value * 2.0f) / (float)ADC_GAIN;
    sprintf(buffer, "ADC input value = %.3f V\n", adc_in_volts);
    size = strlen((const char *)buffer);
    UART_FillTXBuffer(size, (uint8_t *)buffer);
    PRINTF("ADC input value = %d mV\n",(int32_t)(adc_in_volts * 1000));
}

/* ----------------------------------------------------------------------------
 * Function      : void Send_VBAT_Error (void)
 * ----------------------------------------------------------------------------
 * Description   : Create a buffer with "VBAT lower than threshold!!" and send
 *                 it to the UART.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Send_VBAT_Error(void)
{
    uint8_t size;
    const char buffer[] = "VBAT voltage lower than threshold!!\n";
    size = strlen(buffer);
    UART_FillTXBuffer(size, (uint8_t *)buffer);
    PRINTF("VBAT voltage lower than threshold!!\n");
}

/* ----------------------------------------------------------------------------
 * Function      : int main(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the system. The ADC value is read and an average
 *                 of 100 values are made. When the average value is calculated
 *                 it's sent by UART. If a battery monitor interrupt is
 *                 generated an error message is sent by UART.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int main(void)
{
    /* Initialize global variables */
    data_ready_flag = 0;
    bat_error_flag  = 0;
    adc_value = 0.0f;

    /* Initialize the system */
    Initialize();
    PRINTF("DEVICE INITIALIZED\n");
    /* Spin loop */
    while (1)
    {
        /* Refresh the watch-dog timer */
        Sys_Watchdog_Refresh();

        if (data_ready_flag == 1)
        {
            data_ready_flag = 0;
            Send_ADC_Value();
        }
        if (bat_error_flag == 1)
        {
            bat_error_flag = 0;
            Send_VBAT_Error();
        }
    }
}
