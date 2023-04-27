/*============================================================================
 * Módulo de comunicación protocolo 1 Wire.
 * Archivo: DS18B20.c
 * Versión: 1.2
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

#include "DS18B20.h"
#include "oneWire.h"

/*=============================================================================
* FUNCIÓN: DSreadScratch
* Que hace: Lee el scratchpad.
* PARÁMETROS:
* Que recibe: char * buffer9, dirección del buffer para guardar respuesta
* 			  int port, int pin puerto y pin del bus oneWire
* Que devuelve:  -1 si no hay lectura, 0 si hay lectura.
* Variables externas que modifca: N/A
*============================================================================*/
int DSreadScratch(char * p, int port, int pin)
{
/////////////////ORIGINAL////////////////////////
//	int rv = -1;
//	char crc = 0;
//	//char * p = buffer9;
//
//	if(OWpresence(port, pin)==0)
//	{
//		OWdelay_uS(400);
//		disableInterrupts();
//		OWcommand(0x33, p, 8, port, pin);
//		// comando Read scratch
//		OWcommand(0xBE, p, 9, port, pin);
//		enableInterrupts();
//		// chequea CRC
//		crc = OWcrc(p, 8);
//		if(crc == p[8])
//		{
//			rv = 0;
//		}
//	}
//	return rv;
//	//////////////////////////////////////////PRUEBA//////////////
	OWsetIn(port,pin);
	// espera el fin de conversión
	while(OWread(port,pin) == FALSE);
	if(OWpresence(port, pin)==0)
	{
		printf("Presence\r\n");
		OWdelay_uS(400);
		disableInterrupts();
		// comando Read ROM
		OWcommand(0x33, p, 8, port, pin);
		// comando Read scratch
		OWcommand(0xBE, p, 9, port, pin);
		enableInterrupts();
	}
}

/*=============================================================================
* FUNCIÓN: DSreadTemperature
* Que hace: Lee la temperatura del sensor (para un solo sensor en bus, ignorando
* su ROM, sino utilizar DSreadROMTemperature).
* PARÁMETROS:
* Que recibe: (int) pin puerto y pin del bus oneWire.
* Que devuelve: la temperatura leida en °C y sin decimales.
* Variables externas que modifca: N/A
*============================================================================*/
int DSreadTemperature(int port, int pin)
{
	volatile int buffTemp = -1;
	volatile char crc = 0;
	char p[9];

	if(OWpresence(port,pin)==0)
	{
		gpioWrite( LED_PRUEBA, TRUE);
		OWdelay_uS(400);
		disableInterrupts();
		OWcommand(0x33, p, 8, port, pin);
		OWcommand(0x44, p, 0, port, pin);
		enableInterrupts();
		OWsetIn(port,pin);
		// espera el fin de conversión
		while(OWread(port,pin) == FALSE);
		OWpresence(port, pin);
		OWdelay_uS(400);
		disableInterrupts();
		// comando Read ROM
		OWcommand(0x33, p, 8, port, pin);
		// comando Read scratch
		OWcommand(0xBE, p, 9, port, pin);
		enableInterrupts();
		// chequea CRC
		crc = OWcrc(p, 8);
		if(crc == p[8])
		{
			buffTemp = p[1];
			buffTemp <<= 8;
			buffTemp |= p[0];
			buffTemp = buffTemp >> 4;
		}
	}
	return buffTemp;
}

/*=============================================================================
* FUNCIÓN: DSmultipleConv
* Que hace: Emite el comando para comenzar la conversión a todos los sensores
* en bus.
* PARÁMETROS:
* Que recibe: (int) pin puerto y pin del bus oneWire.
* Que devuelve: N/A
* Variables externas que modifca: N/A
*============================================================================*/
void DSmultipleConv(int port, int pin)
{
	volatile int buffTemp = -1;
	volatile char crc = 0;
	char p[9];

	if(OWpresence(port,pin)==0)
	{
		OWdelay_uS(400);
		disableInterrupts();
		OWwriteByte(OneWireStruct, 0xCC);
		OWcommand(0x44, p, 0, port, pin);
		enableInterrupts();
		OWsetIn(port,pin);
		// espera el fin de conversión
		while(OWread(port,pin) == false);

	}
}

/*=============================================================================
* FUNCIÓN: DSreadROMTemperature
* Que hace: Lee la temperatura de un sensor buscando por su ROM.
* PARÁMETROS:
* Que recibe: (int) pin puerto y pin del bus oneWire. (int) posición del sensor
* a leer en el vector de sensores.
* Que devuelve: la temperatura leida en °C y sin decimales.
* Variables externas que modifca: N/A
*============================================================================*/
int DSreadROMtemp(int port, int pin,int orden)
{
	volatile int buffTemp = -1;
	volatile char crc = 0;
	char p[9];
	int i;

	if(OWpresence(port,pin)==0)
	{
		OWpresence(port, pin);
		OWdelay_uS(400);
		disableInterrupts();
		OWwriteByte(OneWireStruct, 0x55);
		for(i=0;i<8;i++)
		{
			OWwriteByte(OneWireStruct, sensor[orden].ROM_NO[i]);
		}
		// comando Read scratch
		OWcommand(0xBE, p, 9, port, pin);
		enableInterrupts();
		// chequea CRC
		crc = OWcrc(p, 8);
		if(crc == p[8])
		{
			buffTemp = p[1];
			buffTemp <<= 8;
			buffTemp |= p[0];
			buffTemp = buffTemp >> 4;
		}
	}
	return buffTemp;
}

/*=============================================================================
* FUNCIÓN: DSwriteScratch     ACTUALIZAR!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* Que hace: Lee el scratchpad.
* PARÁMETROS:
* Que recibe: char * buffer9, dirección del buffer para guardar respuesta
* 			  int port, int pin puerto y pin del bus oneWire
* Que devuelve:  -1 si no hay lectura, 0 si hay lectura.
* Variables externas que modifca: N/A
*============================================================================*/
int DSwriteScratch(char * buffer9, int port, int pin)
{
	int rv = -1;
	char crc = 0;
	char * p = buffer9;

	if(OWpresence(port, pin)==0)
	{
		OWdelay_uS(400);
		disableInterrupts();
		//OWcommand(0x33, p, 8, port, pin);
		// comando Read scratch
		OWcommand(0x4E, p, 9, port, pin);
		enableInterrupts();
		// chequea CRC
//		crc = OWcrc(p, 8);
//		if(crc == p[8])
//		{
//			rv = 0;
//		}
	}
	return rv;
}
