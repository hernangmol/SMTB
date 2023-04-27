/*============================================================================
 * Módulo de Interfaz oneWire / EDU-CIAA-NXP.
 * Archivo: OW_port.h
 * Versión: 1.2
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

#ifndef MIS_PROGS_PCSE_TP_FINAL_INC_OW_PORT_H_
#define MIS_PROGS_PCSE_TP_FINAL_INC_OW_PORT_H_

/*===========================================================================*
* Bloque de defines:
* Asegura la compatibilidad del módulo DS18B20 y oneWire, si no se utiliza la
* librería sAPI.
*============================================================================*/
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif
/*===========================================================================*
*============================================================================*/

#include "sapi.h"

typedef enum
{
	GPIO_0 = 32,
	GPIO_1 = 16, ///////////chequear!
	GPIO_2 = 31,
	GPIO_6 = 29

} gpioMap_type;

/*=====[Declaración de (function like) macros]===============================*/

#define OWsetOut(port,pin)	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, port, pin)
#define OWsetIn(port,pin)	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, port, pin)
#define OWread(port,pin)	Chip_GPIO_GetPinState(LPC_GPIO_PORT, port, pin)
#define OWlow(port,pin)		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, port, pin)
#define OWhigh(port,pin)	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, port, pin)

#define LED_PRUEBA LEDG

/*=====[Declaración de tipos y datos públicos]==============================*/

typedef struct
{
	gpioMap_type OWgpio;
	int OWport;
	int OWpin;
	char lastDiscrepancy;
	char lastFamilyDiscrepancy;
	char lastDeviceFlag;
	char ROM_NO[8];

} 	OWbus_t;

OWbus_t* OneWireStruct;
OWbus_t* OneWireStruct2;

/*=====[Declaración de funciones públicas]===============================*/

OWbus_t* OWinit(gpioMap_type GPIO_OW);
OWbus_t* OWinit2(gpioMap_type GPIO_OW);
void 	 OWdelay_uS(unsigned t);
char 	 OWreadBit(OWbus_t* OneWireStruct);
void 	 OWwriteBit(OWbus_t* , char);
void  	 disableInterrupts(void);
void 	 enableInterrupts(void);

#endif /* MIS_PROGS_PCSE_TP_FINAL_INC_OW_PORT_H_ */
