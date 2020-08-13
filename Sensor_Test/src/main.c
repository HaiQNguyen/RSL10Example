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

void Magnetic_Callback(bhy_data_generic_t *data, bhy_virtual_sensor_t sensor);
void LinearAccel_CallBack(bhy_data_generic_t *data, bhy_virtual_sensor_t sensor);
void Gyro_CallBack(bhy_data_generic_t *data, bhy_virtual_sensor_t sensor);

int main(void)
{
	//return status of the sensor library
	int32_t retval;

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



    /* Increase I2C bus speed to 400kHz. */
    HAL_I2C_Init();
	//HAL_I2C_SetBusSpeed(HAL_I2C_BUS_SPEED_FAST);

	/*Sensor setup**********************************************************************/
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

    printf("APP: Entering main loop.\r\n");
    while (1)
    {
        /* Execute any events that have occurred & refresh Watchdog timer. */
        BDK_Schedule();

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

