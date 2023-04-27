/*============================================================================
 * Módulo TEST LOOP.
 * Archivo: SMTB_LOOP.c
 * Versión: 1.0
 * Fecha 12/01/2023
 * Autor: Hernán Gomez Molino
 *===========================================================================*/
 
#include "SMTB_LOOP.h"
#include "SUCI.h"
#include "SMTB_EEPROM.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "sapi.h"

//#define TIMEOUT 1000

/*Intercommunication ----------------------------------*/
//INTERCOMM TYPES
#define COMMAND 	255
#define ROM			254
#define SENSOR		253
//COMMANDS
#define I_AM		240
#define U_ARE		239
#define SEND_ROM	238
#define ACK			237
/*-----------------------------UNIFICAR!!!------------------------*/

//#define SEM_GREEN 	ENET_RXD1
//#define SEM_RED 	ENET_TXEN

extern QueueHandle_t intercommQueue;
extern struct intercommMsg_t
{
	uint8_t msgType;
	uint8_t order;
	char message[8];
	uint8_t crc;
};

void remote_uci_test(uint8_t local_uci)
{
	struct intercommMsg_t msgToRemote;

	msgToRemote.msgType = COMMAND;
	msgToRemote.order = I_AM;
	if(local_uci == 1)
		msgToRemote.message[0] = UCI_1;
	else if(local_uci == 2)
		msgToRemote.message[0] = UCI_2;
	xQueueSend(intercommQueue, &msgToRemote, TIMEOUT);
}

void uci_id_test(void)
{
	uint8_t posicion;
	posicion = Board_EEPROM_readByte(433);
		if(posicion==1)
		{
			setStatus(BIT_UCI_1, ON);
			setStatus(BIT_UCI_2, OFF);
			/* Create the other task in exactly the same way. */
			//xTaskCreate( UCI_1_task, (const char *)"UCI#1 Task", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL );
			//  configASSERT( res == pdPASS );
		}
		else if(posicion==2)
		{
			setStatus(BIT_UCI_1, OFF);
			setStatus(BIT_UCI_2, ON);
			/* Create the other task in exactly the same way. */
			//xTaskCreate( UCI_2_task, (const char *)"UCI#2 Task", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL );
			//  configASSERT( res == pdPASS );
		}
		else
		{
			//  ToDo: manejo de error POSICION
			//printf("ERROR Posición: %d    ",Board_EEPROM_readByte(433));delay(1000); ////////////////////cambiar por API FREERTOS/////////////////////////////
	//		modo=0xA0;
		}

}

void rom_test(void)
{
	uint8_t serie;
	serie = Board_EEPROM_readByte(432);
	if((100>serie)||(serie>120))
	{
		setStatus(BIT_EEPROM, OFF);
		//  ToDo: manejo de error EEPROM
		//printf("ERROR Eeprom\r\n");
	}

	setStatus(BIT_EEPROM, ON);
}

/*=============================================================================
* FUNCIÓN: list_test
* Que hace: Chequea la existencia de una lista completa de sensores en ROM
* PARÁMETROS:
* Que recibe: nada
* Que devuelve:
* -1 si no hay una lista completa (algun sensor sin su primer número en 0x28)
*  0 si hay lista completa.
* Variables externas que modifca: N/A
*============================================================================*/
int list_test(void)
{
	tempSens_t buffer;
	int k,j,retVal = 0;

	for( k=0; k<6; k++ ) //cambiar el MagicNumber x N!!!!!!!!!!!!!!!!!!!!
		{
			Board_EEPROM_loadSensor(k, &buffer);
			for( j=0; j<8; j++ )
				printf("%X - ",buffer.ROM_NO[j]);
			printf("\r\n");
			if(buffer.ROM_NO[0] != 0x28)
				retVal = -1;
			vTaskDelay(1000);
		}
	printf("retorno: %d\r\n",retVal);
	return(retVal);
}

void ROM_request(uint8_t position)
{
	struct intercommMsg_t msgToRemote;

	printf("ROM_request\r\n");
	msgToRemote.msgType = COMMAND;
	msgToRemote.order = SEND_ROM;
	msgToRemote.message[0] = position;
	xQueueSend(intercommQueue, &msgToRemote, TIMEOUT);
	//1 enviar req de ROM a UCI_REM.....................OK
	//2 recibir en UCI_REM..............................OK
	//3 actualizar dato ROM_remota
}

void buzzer_test(void)
{
	//ToDo: modulo HAL para buzzer///////////////////////
	gpioInit(GPIO4, GPIO_OUTPUT );                     //
	gpioWrite(GPIO4,ON); // Buzzer                     //
	vTaskDelay(100);										   //
	gpioWrite(GPIO4,OFF);                              //
	//hasta acá//////////////////////////////////////////
}

//ToDo: pasar a modulo HAL semaforo
void semaphoreTest(void)
{
	gpioWrite( SEM_RED, ON );
	vTaskDelay(500);
	gpioWrite( SEM_GREEN, ON );
	vTaskDelay(500);
	gpioWrite( SEM_RED, OFF );
	vTaskDelay(500);
	gpioWrite( SEM_GREEN, OFF );
}
