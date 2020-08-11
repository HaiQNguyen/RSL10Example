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
#include <SoftwareTimer.h>
#include "SEGGER_RTT.h"

#include <stdio.h>
#include "main.h"

#define BUFF_SIZE 1024
#define SEND_SIZE 80
#define SEND_LOOP 2500

uint8_t buffer_1024_Byte[BUFF_SIZE];
char    send_buffer_Char[2*BUFF_SIZE+2];
struct SwTimer elapse_timer;
struct SwTimer_Duration elapsed;
uint32_t printf_sending_time;

volatile bool start_test = false;
void SetupTestData(void);
void ExecuteTest(void);
void SetupExecuteTest(void);

int main(void)
{
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
    BTN_AttachScheduled(BTN_EVENT_RELEASED, &PB_TransitionEvent, (void*)BTN0, BTN0);


    SwTimer_Initialize(&elapse_timer);
    printf("APP: Entering main loop.\r\n");
    while (1)
    {
        /* Execute any events that have occurred & refresh Watchdog timer. */
        BDK_Schedule();

        if(start_test)
        {
        	printf("Send %d * %d bytes of data\n", SEND_LOOP, SEND_SIZE);
        	SetupTestData();
			ExecuteTest();
        	//SetupExecuteTest();
        	printf("Send %d * %d bytes of data\n", SEND_LOOP, SEND_SIZE);
			printf("time: %lu %lu ms\n", elapsed.seconds , elapsed.nanoseconds / 1000000L);
        	start_test = false;
        }

        SYS_WAIT_FOR_INTERRUPT;
    }

    return 0;
}

void PB_TransitionEvent(void *arg)
{
    ButtonName btn = (ButtonName)arg;

    if(btn == BTN0)
    {
    	start_test = true;
    }
}

void SetupTestData(void)
{
	for(int i = 0; i < SEND_SIZE; i++)
	{
		buffer_1024_Byte[i] = rand();
	}
}
void ExecuteTest(void)
{
	char *hexchar;
	for(int j = 0; j < SEND_SIZE; j++) {
		hexchar = send_buffer_Char + 2*j;
		*hexchar = (buffer_1024_Byte[j] >> 4);
		*hexchar += '0';
		if (*hexchar > '9') {
			*hexchar += 7;
		}
		hexchar ++;
		*hexchar = (buffer_1024_Byte[j] & 0x0f);
		*hexchar += '0';
		if (*hexchar > '9') {
			*hexchar += 7;
		}
	}
	send_buffer_Char[2*SEND_SIZE] = '\n';
	send_buffer_Char[2*SEND_SIZE+1] = 0;

	SwTimer_Start(&elapse_timer);
	for (int i = 0; i < SEND_LOOP; i++) {
		//printf(send_buffer_Char); // really bad performance
		SEGGER_RTT_printf(0, "%s", send_buffer_Char);
		//SEGGER_RTT_Write(0, send_buffer_Char, 30);
		//SEGGER_RTT_WriteString(0, send_buffer_Char);
	}
	SwTimer_GetElapsed(&elapse_timer, &elapsed);
	SwTimer_Stop(&elapse_timer);
}

void SetupExecuteTest(void)
{
	char *hexchar;
	SwTimer_Start(&elapse_timer);
	for (int i = 0; i < SEND_LOOP; i++) {
		for(int j = 0; j < SEND_SIZE; j++) {
			buffer_1024_Byte[j] = rand();
		}
		for(int j = 0; j < SEND_SIZE; j++) {
			hexchar = send_buffer_Char + 2*j;
			*hexchar = (buffer_1024_Byte[j] >> 4);
			*hexchar += '0';
			if (*hexchar > '9') {
				*hexchar += 7;
			}
			hexchar ++;
			*hexchar = (buffer_1024_Byte[j] & 0x0f);
			*hexchar += '0';
			if (*hexchar > '9') {
				*hexchar += 7;
			}
		}
		send_buffer_Char[2*SEND_SIZE] = '\n';
		send_buffer_Char[2*SEND_SIZE+1] = 0;

		printf(send_buffer_Char);
		//SEGGER_RTT_printf(0, "%s", send_buffer_Char);
		//SEGGER_RTT_Write(0, send_buffer_Char, 30);
		//SEGGER_RTT_WriteString(0, send_buffer_Char);
	}
	SwTimer_GetElapsed(&elapse_timer, &elapsed);
	SwTimer_Stop(&elapse_timer);
}
