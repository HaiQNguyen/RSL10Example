//-----------------------------------------------------------------------------
// Copyright (c) 2018 Semiconductor Components Industries LLC
// (d/b/a "ON Semiconductor").  All rights reserved.
// This software and/or documentation is licensed by ON Semiconductor under
// limited terms and conditions.  The terms and conditions pertaining to the
// software and/or documentation are available at
// http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf ("ON Semiconductor Standard
// Terms and Conditions of Sale, Section 8 Software") and if applicable the
// software license agreement.  Do not use this software and/or documentation
// unless you have carefully read and you agree to the limited terms and
// conditions.  By using this software and/or documentation, you agree to the
// limited terms and conditions.
//-----------------------------------------------------------------------------
#include <BDK.h>
#include <BSP_Components.h>

#include <stdio.h>
#include "main.h"

#warning uncomment this to read sensor data
//#define TEST_SENSOR

#warning uncomment this to read adc data
#define TEST_ADC


void Magnetic_Callback(bhy_data_generic_t *data, bhy_virtual_sensor_t sensor);
void LinearAccel_CallBack(bhy_data_generic_t *data, bhy_virtual_sensor_t sensor);
void Gyro_CallBack(bhy_data_generic_t *data, bhy_virtual_sensor_t sensor);
void ADC_BATMON_IRQHandler(void);

bool adc_ready = false;
volatile int adc_value;


//Struct to hold elapse time in millisecond
typedef struct {
	uint32_t start;
	uint32_t elapse;
}Time_Elapse_Millis;

Time_Elapse_Millis time_elapse;

//Start counting time
void Timer_Start(Time_Elapse_Millis * t);

// Stop counting time
// calculate number of ms has passed since the Timer_Start is called
// this number can be access via t-> elapse
void Timer_Stop(Time_Elapse_Millis * t);


int main(void)
{
	//return status of the sensor library


    /* Initialize BDK library, set system clock (default 8MHz). */
    BDK_Initialize();

    /* Initialize all LEDs */
    LED_Initialize(LED_RED);
    LED_Initialize(LED_GREEN);
    LED_Initialize(LED_BLUE);

    /* Initialize Button to call callback function when pressed or released. */
    BTN_Initialize(BTN0);

    /* AttachScheduled -> Callback will be scheduled and called by Kernel Scheduler. */
    /* AttachInt -> Callback will be called directly from interrupt routine. */
    BTN_AttachScheduled(BTN_EVENT_TRANSITION, &PB_TransitionEvent, (void*)BTN0, BTN0);

#ifdef TEST_ADC
    /*ADC Setup ***********************************************************************/
    //Set DIO3 as ADC pin
    Sys_DIO_Config(3,DIO_MODE_DISABLE | DIO_NO_PULL);

    //8 channel are sample
    //sample = slow clock/1280
    Sys_ADC_Set_Config( ADC_NORMAL | ADC_PRESCALE_128H);

    //channel 3 contains ADC_positive_inout from DIO3
    Sys_ADC_InputSelectConfig(3, ADC_POS_INPUT_DIO3 | ADC_NEG_INPUT_GND);

    //enable interrupt on channel 3 and disable batery monitor
    Sys_ADC_Set_BATMONIntConfig(INT_EBL_ADC | ADC_INT_CH3 | INT_DIS_BATMON_ALARM);


    //Enable interupt
    NVIC_EnableIRQ(ADC_BATMON_IRQn);
#endif

#ifdef TEST_SENSOR
	/*Sensor setup**********************************************************************/
	int32_t retval;

    /* Increase I2C bus speed to 400kHz. */
    //HAL_I2C_Init();
	HAL_I2C_SetBusSpeed(HAL_I2C_BUS_SPEED_FAST);

	retval = BHI160_NDOF_Initialize();
	if(BHY_SUCCESS != retval)
	{
		printf("sensor init error\n");
		return 0;
	}

	retval = BHI160_NDOF_EnableSensor(BHI160_NDOF_S_MAGNETIC_FIELD, &Magnetic_Callback, 10);
	if(BHY_SUCCESS != retval)
	{
		printf("magnetic enable error\n");
		return 0;
	}

	retval = BHI160_NDOF_EnableSensor(BHI160_NDOF_S_LINEAR_ACCELERATION, &LinearAccel_CallBack, 10);
	if(BHY_SUCCESS != retval)
	{
		printf("linear accel enable error\n");
		return 0;
	}

	retval = BHI160_NDOF_EnableSensor(BHI160_NDOF_S_RATE_OF_ROTATION, &Gyro_CallBack, 10);
	if(BHY_SUCCESS != retval)
	{
		printf("gyro enable error\n");
		return 0;
	}
#endif

	Timer_Start(&time_elapse);
    printf("APP: Entering main loop.\r\n");
    while (1)
    {
    	static int count=0;
    	static unsigned int minRead = 65535;
    	static unsigned int maxRead = 0;
    	static unsigned long readings=0;
    	/* Execute any events that have occurred & refresh Watchdog timer. */
        BDK_Schedule();
#ifdef TEST_ADC
        if(adc_ready)
        {
        	readings += adc_value;
        	if (adc_value < minRead) minRead=adc_value;
        	if (adc_value > maxRead) maxRead=adc_value;
        	count ++;

        	Timer_Stop(&time_elapse);
        	if (time_elapse.elapse > 250) {
        		Timer_Start(&time_elapse);
            	printf("rate: %5.5d, avg: %d, min: %d, max:%d\n", count*4, (int)(readings/count), minRead, maxRead);
            	count=0;
            	minRead = 65535;
            	maxRead = 0;
            	readings=0;
        	}

        	adc_ready = false;
        }
#endif
        SYS_WAIT_FOR_INTERRUPT;
    }

    return 0;
}

void PB_TransitionEvent(void *arg)
{
    ButtonName btn = (ButtonName)arg;

    switch (btn)
    {
    case BTN0:
        break;
    default:
        return;
    }

}

void Magnetic_Callback(bhy_data_generic_t *data, bhy_virtual_sensor_t sensor)
{
	//print raw sensor data
	int16_t x,y,z;
	x = data->data_vector.x;
	y = data->data_vector.y;
	z = data->data_vector.y;
	printf("Magnetic - id:%d x:%d y:%d z:%d\n", data->data_vector.sensor_id, x, y, z);
}

void LinearAccel_CallBack(bhy_data_generic_t *data, bhy_virtual_sensor_t sensor)
{
	//print raw sensor data
	int16_t x,y,z;
	x = data->data_vector.x;
	y = data->data_vector.y;
	z = data->data_vector.y;
	printf("Linear Acceleration - id:%d x:%d y:%d z:%d\n", data->data_vector.sensor_id, x, y, z);
}
void Gyro_CallBack(bhy_data_generic_t *data, bhy_virtual_sensor_t sensor)
{
	//print raw sensor data
		int16_t x,y,z;
		x = data->data_vector.x;
		y = data->data_vector.y;
		z = data->data_vector.y;
		printf("Gyroscope - id:%d x:%d y:%d z:%d\n", data->data_vector.sensor_id, x, y, z);
}

void ADC_BATMON_IRQHandler(void)
{
	 adc_value = ADC->DATA_TRIM_CH[3];
	 adc_ready = true;
}

/*Code for counting time*/
void Timer_Start(Time_Elapse_Millis * t)
{

#ifdef USING_SW_TIMER
#error not implemeneted
#else
	t->start = HAL_Time();
#endif
}

void Timer_Stop(Time_Elapse_Millis * t)
{
#ifdef USING_SW_TIMER
#error not implemeneted
#else
	t->elapse = HAL_Time() - t->start;
#endif
}
