/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

#include "queue.h"

/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

TaskHandle_t Button_1_Monitor_Handler = NULL;
TaskHandle_t Button_2_Monitor_Handler = NULL;
TaskHandle_t Periodic_Transmitter_Handler = NULL;
TaskHandle_t Uart_Receiver_Handler = NULL;
TaskHandle_t Load_1_Simulation_Handler = NULL;
TaskHandle_t Load_2_Simulation_Handler = NULL;


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */

QueueHandle_t Message_1_Queue = NULL;
QueueHandle_t Message_2_Queue = NULL;
QueueHandle_t Message_3_Queue = NULL;


static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
	pinState_t ButtonState;

void Button_1_Monitor_Task( void * pvParameters )
{
		pinState_t Button_1_State;
		TickType_t xLastWakeTime = xTaskGetTickCount();
		signed char Button_1_Flag = 0;
	
    for( ;; )
    {
			Button_1_State = GPIO_read(PORT_1, PIN0);
			
			GPIO_write(PORT_1,PIN3,PIN_IS_LOW);	
			vTaskDelayUntil(&xLastWakeTime, 50);
			GPIO_write(PORT_1,PIN3,PIN_IS_HIGH);	
			if(Button_1_State != GPIO_read(PORT_1, PIN0) && Button_1_State== PIN_IS_LOW)
			{
        Button_1_Flag = 'P';
				xQueueOverwrite(Message_1_Queue, &Button_1_Flag );
			}
			
      else if(Button_1_State != GPIO_read(PORT_1, PIN0) && Button_1_State== PIN_IS_HIGH)
      {	
        Button_1_Flag = 'N';
				xQueueOverwrite(Message_1_Queue, &Button_1_Flag );
      }
    }
}
void Button_2_Monitor_Task( void * pvParameters )
{
		pinState_t Button_2_State;
		TickType_t xLastWakeTime = xTaskGetTickCount();
		signed char Button_2_Flag = 0;
	
    for( ;; )
    {
			Button_2_State = GPIO_read(PORT_1, PIN1);
			
			GPIO_write(PORT_1,PIN4,PIN_IS_LOW);	
			vTaskDelayUntil(&xLastWakeTime, 50);
			GPIO_write(PORT_1,PIN4,PIN_IS_HIGH);	

			if(Button_2_State != GPIO_read(PORT_1, PIN1) && Button_2_State== PIN_IS_LOW)
			{
        Button_2_Flag = 'P';
				xQueueOverwrite(Message_2_Queue, &Button_2_Flag );
			}
			
      else if(Button_2_State != GPIO_read(PORT_1, PIN1) && Button_2_State== PIN_IS_HIGH)
      {	
        Button_2_Flag = 'N';
				xQueueOverwrite(Message_2_Queue, &Button_2_Flag );
      }
    }
}

void Periodic_Transmitter_Task(void* pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	unsigned char Local_U8_Counter = 0;
	char String[12] = "\nPending...";
	String[11] = '\0';
	for(;;)
	{
		for (Local_U8_Counter = 0; Local_U8_Counter <= 4; Local_U8_Counter++)
		{
			xQueueSend(Message_3_Queue, String+Local_U8_Counter, 100);
		}
		
		GPIO_write(PORT_1,PIN5,PIN_IS_LOW);	
		vTaskDelayUntil(&xLastWakeTime, 100);
		GPIO_write(PORT_1,PIN5,PIN_IS_HIGH);	
	}
}



void Uart_Receiver_Task(void* pvParameters)
{
		TickType_t xLastWakeTime = xTaskGetTickCount();
		signed char Local_Message_1_Data;
		signed char Local_Message_2_Data;
		unsigned char Local_Message_3_Data[12];
		unsigned char Local_U8_Counter = 0;
		for (;;)
	{
		if ( uxQueueMessagesWaiting(Message_1_Queue) != 0 )
		{
			xQueueReceive(Message_1_Queue, &Local_Message_1_Data, 0);
			xSerialPutChar('\n');		
			xSerialPutChar('B');
			xSerialPutChar('u');
			xSerialPutChar('t');
			xSerialPutChar('t');
			xSerialPutChar('o');
			xSerialPutChar('n');
			xSerialPutChar('1');
			xSerialPutChar(':');
			xSerialPutChar(' ');
			xSerialPutChar(Local_Message_1_Data);
		}
		
		if ( uxQueueMessagesWaiting(Message_2_Queue) != 0 )
		{
			xQueueReceive(Message_2_Queue, &Local_Message_2_Data, 0);
			xSerialPutChar('\n');		
			xSerialPutChar('B');
			xSerialPutChar('u');
			xSerialPutChar('t');
			xSerialPutChar('t');
			xSerialPutChar('o');
			xSerialPutChar('n');
			xSerialPutChar('2');
			xSerialPutChar(':');
			xSerialPutChar(' ');
			xSerialPutChar(Local_Message_2_Data);
		}
		
		if ( uxQueueMessagesWaiting(Message_3_Queue) != 0 )
		{
			for(Local_U8_Counter = 0; Local_U8_Counter <= 4; Local_U8_Counter++)
			{
				xQueueReceive(Message_3_Queue, Local_Message_3_Data+Local_U8_Counter, 0);
			}

			for(Local_U8_Counter = 0; Local_U8_Counter <= 4; Local_U8_Counter++)
			{
				/* Send the name of button character by character */
				xSerialPutChar(Local_Message_3_Data[Local_U8_Counter]);
			}
			
			xQueueReset(Message_3_Queue);
		}
		
		GPIO_write(PORT_1,PIN6,PIN_IS_LOW);	
			vTaskDelayUntil(&xLastWakeTime, 20);
		GPIO_write(PORT_1,PIN6,PIN_IS_HIGH);	

	}
}


void Load_1_Simulation_Task(void* pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	unsigned int Local_U32_counter = 0;

	for(;;)
	{
		for(Local_U32_counter=0;Local_U32_counter<=47959;Local_U32_counter++);
		GPIO_write(PORT_1,PIN7,PIN_IS_LOW);	
		vTaskDelayUntil(&xLastWakeTime, 10);
		GPIO_write(PORT_1,PIN7,PIN_IS_HIGH);	
	}
}

void Load_2_Simulation_Task(void* pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	unsigned int Local_U32_counter = 0;

	for(;;)
	{
		for(Local_U32_counter=0;Local_U32_counter<=115101;Local_U32_counter++);
			GPIO_write(PORT_1,PIN8,PIN_IS_LOW);	
		vTaskDelayUntil(&xLastWakeTime, 100);
			GPIO_write(PORT_1,PIN8,PIN_IS_HIGH);	
	}
}


void vApplicationTickHook(void)
{
GPIO_write(PORT_1,PIN2,PIN_IS_HIGH);	
GPIO_write(PORT_1,PIN2,PIN_IS_LOW);	
}


/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
	Message_1_Queue = xQueueCreate(1, sizeof(char));
	Message_2_Queue = xQueueCreate(1, sizeof(char));
	Message_3_Queue = xQueueCreate(5, sizeof(char));

	
	xTaskPeriodicCreate(Button_1_Monitor_Task,
						"Button 1 Monitor Task",
						100, (void*)0,
						1,
						&Button_1_Monitor_Handler,
						50);
						
	xTaskPeriodicCreate(Button_2_Monitor_Task,
						"Button 2 Monitor Task",
						100,
						(void*)0,
						1,
						&Button_2_Monitor_Handler,
						50);
						
	xTaskPeriodicCreate(Periodic_Transmitter_Task,
						"Periodic Transmitter Task",
						100,
						(void*)0,
						1,
						&Periodic_Transmitter_Handler,
						100);
						
	xTaskPeriodicCreate(Uart_Receiver_Task,
						"Uart Receiver Task",
						100,
						(void*)0,
						1,
						&Uart_Receiver_Handler,
						20);
						
	xTaskPeriodicCreate(Load_1_Simulation_Task,
						"Load 1 Simulation Task",
						100,
						(void*)0,
						1,
						&Load_1_Simulation_Handler,
						10);
						
	xTaskPeriodicCreate(Load_2_Simulation_Task,
						"Load 2 Simulation Task",
						100,
						(void*)0,
						1,
						&Load_2_Simulation_Handler,
						100);
	
	
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/