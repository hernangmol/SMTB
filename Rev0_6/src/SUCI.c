/*============================================================================
 * Módulo de comunicación protocolo 1 Wire.
 * Archivo: SUCI.c
 * Versión: 0.1
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

#include "SUCI.h"
#include "oneWire.h"
#include "FreeRTOS.h"
#include "SMTB_BUTTONS.h"
#include "SMTB_LCD.h"
#include "SMTB_LEDMAT.h"
#include "SMTB_EEPROM.h"
#include "DS18B20.h"

extern void semaphoreInit(void);
extern uint32_t status;

//#define ESCRIBIR_SENSOR
//#define BORRAR_SENSOR
//#define ESCRIBIR_SERIE
//#define ESCRIBIR_POSICION
//#define LEER_SENSOR
//#define OVERRIDE_CONSIST

//#define N 12 // definido acá y en main!!!!!!!!

/*=============================================================================
* FUNCIÓN: DSreadScratch!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ACTUALIZAR!!!!!!!!!!!!!!!!!
* Que hace: Lee el scratchpad.
* PARÁMETROS:
* Que recibe: char * buffer9, dirección del buffer para guardar respuesta
* 			  int port, int pin puerto y pin del bus oneWire
* Que devuelve:  -1 si no hay lectura, 0 si hay lectura.
* Variables externas que modifca: N/A
*============================================================================*/
int SMTB_Init (void)
{
	//ToDo: chequeos de errores
	//ToDo: enviar lo que es oneshot a main()
	uartInit( UART_USB, 115200);//Comunicación por consola
	uartInit( UART_485, 9600);
	SMTB_LCD_Init();
	spiConfig( SPI0 );
	ledMatrixInit();
	buttonsInit();
	Board_EEPROM_init();
	semaphoreInit();
//	systemMode = NORMAL;
	for(int i=0; i<N;i++ ) //Inicialización valores de eje, posición y temp
	{
		sensor[i].eje = (i/6);
		sensor[i].posicion = i-sensor[i].eje*6;
		sensor[i].temp = 0;
		sensor[i].estado = INACTIVE;   // ToDo: eliminar openTranquera
	}
///////////////////////////////////////////////////////////////////////////////////////////
#ifdef ESCRIBIR_SENSOR

	tempSens_t SensorPrueba;
	uint8_t aux[8]= {0x28, 0x9A, 0x21, 0x07, 0xD6, 0x01, 0x3C, 0xC2};
	for(int i=0; i<8; i++)
	{
		SensorPrueba.ROM_NO[i]= aux[i];
		printf("%X - ",SensorPrueba.ROM_NO[i]);
		gpioToggle (LED3);
		delay(200);
	}
	printf("\r\n");
	if(Board_EEPROM_recSensor(3,SensorPrueba)!=0)
		gpioWrite(LEDR, ON);
	else
		gpioWrite(LEDG, ON);
#endif
////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BORRAR_SENSOR

	int pos = 3; // posición del sensor a borrar (hardcoded)
	tempSens_t SensorPrueba;
	//uint8_t aux[8]= {0x28, 0x9A, 0x21, 0x07, 0xD6, 0x01, 0x3C, 0xC2};
	for(int i=0; i<8; i++)
	{
		//SensorPrueba.ROM_NO[i]= aux[i];
		SensorPrueba.ROM_NO[i]= 0;
		delay(200);
	}
	printf("\r\n");
	if(Board_EEPROM_recSensor(pos,SensorPrueba)!=0)
		gpioWrite(LEDR, ON);
	else
		gpioWrite(LEDG, ON);
#endif
////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ESCRIBIR_SERIE

	Board_EEPROM_writeByte(432,102);
	printf("serie: %d   ", Board_EEPROM_readByte(432));

#endif
////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ESCRIBIR_POSICION

	Board_EEPROM_writeByte(433,2);
	printf("serie: %1   ", Board_EEPROM_readByte(433));

#endif
////////////////////////////////////////////////////////////////////////////////////////////
#ifdef LEER_SENSOR

	tempSens_t buffer;
	printf("leido EEPROM:\r\n");
	for(int i=0; i<N; i++ )
	{
		Board_EEPROM_loadSensor(i, &buffer);
		for(int j=0; j<8; j++ )
			printf("%X - ",buffer.ROM_NO[j]);
		printf("\r\n");
		delay(1000);
	}

#endif
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OVERRIDE_CONSIST
	setStatus(BIT_ROM_CONS_CHK, TRUE);
	setStatus(ROM_CONSIST, FALSE);
#endif

	return(0);
}
int setStatus(uint32_t feature, bool_t flag)
{
	uint32_t mascara;
	if(flag)
	{
		status |= feature;
		return(0);
	}
	else
	{
		mascara = ~feature;
		status &= mascara;
		return(0);
	}
	return(-1);
}

bool_t getStatus(uint32_t feature)
{
	//uint32_t mascara;
	if(status & feature)
		return(TRUE);
	else
		return(FALSE);
}
