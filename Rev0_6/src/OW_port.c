/*============================================================================
 * Módulo de Interfaz oneWire / EDU-CIAA-NXP.
 * Archivo: OW_port.c
 * Versión: 1.2
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

#include "OW_port.h"

/*=============================================================================
* FUNCIÓN: OWinit
* Que hace: Crea una struct OWbus_t y la carga con el GPIO, port y pin
* asociados, setea el modo del GPIO y los registros DWT y DEMCR para la
* temporización que usará oneWire.
* PARÁMETROS:
* Que recibe: gpioMap_t GPIO_OW, el GPIO que usará oneWire
* Que devuelve: OWbus_t* punt, puntero a OWbus_t con los datos del bus oneWire
* Variables externas que modifca: N/A
*============================================================================*/
OWbus_t* OWinit(gpioMap_type GPIO_OW)
{

	int * H_DWT_DEMCR	 = (int *)0xE000EDFC;
	int * H_DWT_CTRL	 = (int *)0xE0001000;

	static OWbus_t bus;
	static OWbus_t* punt = &bus;
	OneWireStruct = punt;//------------------------SACAR

	bus.OWgpio = GPIO_OW;
	// Look up table GPIO/ port,pin
	switch(GPIO_OW)
		{
		case GPIO_0:
			bus.OWport = 3;
			bus.OWpin = 0;
			break;
		case GPIO_1:
			bus.OWport = 3;
			bus.OWpin = 3;
			break;
		case GPIO_2:
			bus.OWport = 3;
			bus.OWpin = 4;
			break;
		case GPIO_6:
			bus.OWport = 3;
			bus.OWpin = 6;
			break;
		default:
			punt = NULL;
			return (punt);
		}

	 // seteo de modo de pin oneWire
	Chip_SCU_PinMux (bus.OWport,bus.OWpin,SCU_MODE_INACT | SCU_MODE_ZIF_DIS, SCU_MODE_FUNC0 );
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, bus.OWport, bus.OWpin);
	// bit24[TRCENA]   = habilita todos los DWT
	*H_DWT_DEMCR |= 1<<24;
	// bit0[CYCCNTENA] =  enable CYCCNT
	*H_DWT_CTRL |= 1;
	return (punt);
}

/*=============================================================================
* FUNCIÓN: OWinit2
* Que hace: Crea una struct OWbus_t y la carga con el GPIO, port y pin
* asociados, setea el modo del GPIO y los registros DWT y DEMCR para la
* temporización que usará oneWire.
* PARÁMETROS:
* Que recibe: gpioMap_t GPIO_OW, el GPIO que usará oneWire
* Que devuelve: OWbus_t* punt, puntero a OWbus_t con los datos del bus oneWire
* Variables externas que modifca: N/A
*============================================================================*/
OWbus_t* OWinit2(gpioMap_type GPIO_OW)
{

	int * H_DWT_DEMCR	 = (int *)0xE000EDFC;
	int * H_DWT_CTRL	 = (int *)0xE0001000;

	static OWbus_t bus;
	static OWbus_t* punt = &bus;
	OneWireStruct2 = punt;//------------------------SACAR

	bus.OWgpio = GPIO_OW;
	// Look up table GPIO/ port,pin
	switch(GPIO_OW)
		{
		case GPIO_0:
			bus.OWport = 3;
			bus.OWpin = 0;
			break;
		case GPIO_1:
			bus.OWport = 3;
			bus.OWpin = 3;
			break;
		case GPIO_2:
			bus.OWport = 3;
			bus.OWpin = 4;
			break;
		case GPIO_6:
			bus.OWport = 3;
			bus.OWpin = 6;
			break;
		default:
			punt = NULL;
			return (punt);
		}

	 // seteo de modo de pin oneWire
	Chip_SCU_PinMux (bus.OWport,bus.OWpin,SCU_MODE_INACT | SCU_MODE_ZIF_DIS, SCU_MODE_FUNC0 );
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, bus.OWport, bus.OWpin);
	// bit24[TRCENA]   = habilita todos los DWT
	*H_DWT_DEMCR |= 1<<24;
	// bit0[CYCCNTENA] =  enable CYCCNT
	*H_DWT_CTRL |= 1;
	return (punt);
}

/*=============================================================================
* FUNCIÓN: OWdelay_uS
* Que hace: Genera un delay bloqueante (operación atómica) del orden de uS.
* PARÁMETROS:
* Que recibe: (unsigned int t) cantidad de microsegundos a esperar.
* Que devuelve: N/A
* Variables externas que modifca: N/A
*============================================================================*/
void OWdelay_uS(unsigned t)
{
	static volatile int * H_DWT_CYCCNT	 = (int *)0xE0001004;

	// carga el contador de ciclos en 0
	*H_DWT_CYCCNT = 0;
	// carga los uS a esperar
	t *= (SystemCoreClock/1000000);
	// chequea si el contador alcanzó la cuenta
	while(*H_DWT_CYCCNT < t);
}

/*=============================================================================
* FUNCIÓN: OWreadBit
* Que hace: Lee un bit del bus oneWire.
* PARÁMETROS:
* Que recibe: (puntero a OWbus_t) estructura que indentifica el bus a leer.
* Que devuelve: el bit leido.
* Variables externas que modifca: N/A
*============================================================================*/
char OWreadBit(OWbus_t* OneWireStruct)
{
	char bit = 0;

	OWsetOut(OneWireStruct->OWport, OneWireStruct->OWpin);
	OWlow(OneWireStruct->OWport, OneWireStruct->OWpin);
	OWdelay_uS(3);
	OWsetIn(OneWireStruct->OWport, OneWireStruct->OWpin);
	OWdelay_uS(12);
	if(OWread(OneWireStruct->OWport, OneWireStruct->OWpin))
		bit = 1;
	OWdelay_uS(55);
	// Return bit value
	return bit;
}

/*=============================================================================
* FUNCIÓN: OWwriteBit
* Que hace: Escribe un bit en el bus oneWire.
* PARÁMETROS:
* Que recibe: (puntero a OWbus_t) estructura que indentifica el bus a escribir
* 			  (char) valor del bit a escribir.
* Que devuelve: N/A
* Variables externas que modifca: N/A
*============================================================================*/
void OWwriteBit(OWbus_t* OneWireStruct, char bit)
{
	if (bit)
	{
		OWlow(OneWireStruct->OWport, OneWireStruct->OWpin);
		OWsetOut(OneWireStruct->OWport, OneWireStruct->OWpin);
		OWdelay_uS(3);
		OWhigh(OneWireStruct->OWport, OneWireStruct->OWpin);
		OWsetIn(OneWireStruct->OWport, OneWireStruct->OWpin);
		OWdelay_uS(60);

	}
	else
	{
		// Set line low
		OWlow(OneWireStruct->OWport, OneWireStruct->OWpin);
		OWsetOut(OneWireStruct->OWport, OneWireStruct->OWpin);
		OWdelay_uS(60);
		OWhigh(OneWireStruct->OWport, OneWireStruct->OWpin);
		OWsetIn(OneWireStruct->OWport, OneWireStruct->OWpin);
		OWdelay_uS(10);

	}
}

/*=============================================================================
* FUNCIÓN: disableInterrupts
* Que hace: deshabilita las interrupciones desde la máscara (PRIMASK).
* PARÁMETROS:
* Que recibe: N/A
* Que devuelve: N/A
* Variables externas que modifca: N/A
*============================================================================*/
void disableInterrupts(void)
{
	__set_PRIMASK(1);
}

/*=============================================================================
* FUNCIÓN: enableInterrupts
* Que hace: habilita las interrupciones desde la máscara (PRIMASK).
* PARÁMETROS:
* Que recibe: N/A
* Que devuelve: N/A
* Variables externas que modifca: N/A
*============================================================================*/
void enableInterrupts(void)
{
	__set_PRIMASK(0);
}
