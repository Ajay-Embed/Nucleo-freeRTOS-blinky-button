/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include<stdio.h>
#include<stdint.h>
#include <string.h>

//Global space
#define TRUE			1
#define FALSE			0
#define PRESSED 		TRUE
#define NOTPRESSED	 	FALSE

uint8_t button_status_flag = NOTPRESSED;
void ledTask_Handler(void *params);
void ButtonTask_Handler(void *params);
static void prvSetuphardware(void);					//FUNCTION FOR ALL THE HARDWARE INITIALIZATIONS
static void prvSetupUART(void);   					//HELPER FUNCTION TO SETUP THE UART
void printmsg(char* msg);

int main(void)
{
		//1. Reset all the clock configurations
		RCC_DeInit();

		//2. Update the system core clock variable
		SystemCoreClockUpdate();

		//3. Has various UART and other peripheral initializations init
		prvSetuphardware();


		//4. Creating the LED task
		xTaskCreate(ledTask_Handler, "LED-Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

		//5. Button task
		xTaskCreate(ButtonTask_Handler, "BUTTON-Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

		//6. Schedule them
		vTaskStartScheduler();


	for(;;);
}


void ledTask_Handler(void *params)
{




	while(1)
	{
		if(button_status_flag == PRESSED)
		{
			//Turn ON the LED
			GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
		}else
		{
			//Turn OFF the LED
			GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
		}

	}
}

void ButtonTask_Handler(void *params)
{

	//poll the button pin, if pressed update the gpio flag
	while(1)
	{
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13))
		{
			//If this returns 1 then  button is not pressed on the nucleo board - opposite in discovery board
			button_status_flag = NOTPRESSED;
		}
		else
		{
			//Then button is pressed
			button_status_flag = PRESSED;

		}
	}
}



static void prvSetupUART(void)
{
	GPIO_InitTypeDef gpio_uart_pins;
	USART_InitTypeDef uart2_init;


	//enabling the peripheral clock control of the RCC
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,  ENABLE);

	//Configure PA2 as UART2 Tx and PA2 as UART2 Rx
	memset(&gpio_uart_pins, 0, sizeof(gpio_uart_pins));
	//1. Configuring the alternate functionality of the gpio peripherals
	gpio_uart_pins.GPIO_Mode					=	GPIO_Mode_AF;
	gpio_uart_pins.GPIO_Pin						=	GPIO_Pin_2;
	gpio_uart_pins.GPIO_PuPd					=	GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &gpio_uart_pins);

	//2. GPIO ALternate function mode settings
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);  //PA2- Tx - USART2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);  //PA2- Rx - USART2

	memset(&uart2_init, 0, sizeof(uart2_init));
	//UART PARAMETER INITIALIZATIONS
	uart2_init.USART_BaudRate					=  115200;
	uart2_init.USART_HardwareFlowControl		=  USART_HardwareFlowControl_None;
	uart2_init.USART_Mode						=  USART_Mode_Tx | USART_Mode_Rx;
	uart2_init.USART_Parity						=  USART_Parity_No;
	uart2_init.USART_StopBits					=  USART_StopBits_1;
	uart2_init.USART_WordLength					=  USART_WordLength_8b;
	USART_Init(USART2, &uart2_init);

//Enable the USART2 peripheral
	USART_Cmd(USART2, ENABLE);

}

static void prvSetupGPIO()
{
	//Bydefault all the peripherals clock are disabled, so check with your daatasheets and check where the peripherals GPIOA and GPIOC are hanging
	//In my case they are on the AHB1 bus, SO I will enable them using an API provided in the StdPeripheral library.

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);


	GPIO_InitTypeDef LedInit, ButtonInit;
	LedInit.GPIO_Mode = GPIO_Mode_OUT;
	LedInit.GPIO_OType= GPIO_OType_PP;
	LedInit.GPIO_Pin  = GPIO_Pin_5;
	LedInit.GPIO_Speed = GPIO_Low_Speed;
	LedInit.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOA, &LedInit);

	ButtonInit.GPIO_Mode = GPIO_Mode_IN;
	ButtonInit.GPIO_OType= GPIO_OType_PP;
	ButtonInit.GPIO_Pin  = GPIO_Pin_13;
	ButtonInit.GPIO_Speed = GPIO_Low_Speed;
	ButtonInit.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOC, &ButtonInit);


}



static void prvSetuphardware(void)
{
	//setup UART2
	prvSetupUART();
	//setup button and led
	prvSetupGPIO();

}

void printmsg(char* msg)
{
	for(uint32_t i=0; i< strlen(msg); i+=1)
	{
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
	USART_SendData(USART2, msg[i]);
	}
}
