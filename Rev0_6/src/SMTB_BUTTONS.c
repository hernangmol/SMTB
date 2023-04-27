/*============================================================================
 * Módulo de botones.
 * Archivo: SMTB_BUTTONS.c
 * Versión: 1.2
 * Fecha 25/06/2022
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

#include "SMTB_BUTTONS.h"
#include "sapi.h"


////SYSTEM STATUSES
//#define INITIATION		0
//#define NORMAL 			1
//#define SYS_ALARM  		2
//#define DEGRADED 		3
//#define MAINTENANCE		4
//#define SYSTEM_ERROR	5

extern uint8_t systemMode;

gpioMap_t button[4] = {GPIO3, GPIO5, GPIO7, GPIO8};

typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} buttonStates_t;

buttonStates_t buttonState[4];

void buttonsInit(void)
{
	for(int i=0;i<4;i++)
		gpioInit(button[i], GPIO_INPUT_PULLDOWN );

	for(int i=0;i<4;i++)
		buttonState[i] = STATE_BUTTON_UP;
}

int buttonsUpdate(void)
{
//	if(gpioRead(GPIO3))
//	{
//		systemMode = MAINTENANCE;
//	}

	for(int i=0;i<4;i++)
	{
		switch(buttonState[i])
		{
			case STATE_BUTTON_UP:
				if(gpioRead(button[i]))
					buttonState[i] = STATE_BUTTON_FALLING;
				break;
			case STATE_BUTTON_FALLING:
				if(gpioRead(button[i]))
					buttonState[i] = STATE_BUTTON_DOWN;
				break;
			case STATE_BUTTON_DOWN:
				if(!gpioRead(button[i]))
					buttonState[i] = STATE_BUTTON_RISING;
				break;
			case STATE_BUTTON_RISING:
				if(!gpioRead(button[i]))
				{
					buttonState[i] = STATE_BUTTON_UP;
					printf("Boton %d presionado \r\n", i);
					return(i);
				}
				break;
		}
	}
	return(-1);
}


void buttonsTest(void)
{
	if(gpioRead(GPIO3))
	{
		printf("tecla L\n");
	}
	if(gpioRead(GPIO5))
	{
		printf("tecla U\n");
	}
	if(gpioRead(GPIO7))
	{
		printf("tecla D\n");
	}
	if(gpioRead(GPIO8))
	{
		printf("tecla R\n");
	}
}
