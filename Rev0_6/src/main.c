
/*============================================================================
 * Módulo
 * Archivo:
 * Fecha 17/07/2022
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

/* Mapa de I/O:
 *
 * GPIO_0:
 * GPIO_1: Matriz LED
 * GPIO_2: Bus OW#1
 * GPIO_3: Tecla L
 * GPIO_4: Buzzer
 * GPIO_5: Tecla U
 * GPIO_6: Bus OW#2
 * GPIO_7: Tecla D
 * GPIO_8: Tecla R
 * Led bicolor: RXD1, TXEN
 */

/*=====[Inclusión de dependencias]===========================================*/

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include <string.h>
#include "SUCI.h"
#include "DS18B20.h"
#include "SMTB_UART.h"
#include "SMTB_BUTTONS.h"
#include "SMTB_LCD.h"
#include "SMTB_LEDMAT.h"
#include "SMTB_EEPROM.h"
#include "SMTB_LOOP.h"

/*Tasks functions. */
void vTask1( void *);
void sensorMng( void *);
void UCI_2_task( void *);   //ToDo:arreglar nombre
void vDisplayMng( void *);
void vButtonMng( void *);
void vConsoleMng( void *);
void vCommMng( void *);

/*-----------------------------------------------------------*/
void menuUpdate(int button);
tempSens_t altaUpdate(void);
void altaSecUpdate(void);
void altaSec2Update(void);
void menuInit(void);
void menuAlarmInit(void);
void menuSensInit(void);
void menuROMInit(void);
void menuLoopInit(void);
void menuEjeInit(void);
void menuPosInit(void);
void menuConfInit(int, int);
void menuROMcopyInit(void);
void menuAlarmUpdate(int);
void menuViewUpdate(int);
void menuSensUpdate(int);
int	 menuEjeUpdate(int);
int  menuPosUpdate(int);
void menuROMupdate1(int);
void menuROMupdate(int);
void menuLoopUpdate(int);
void semaphoreInit(void);
//void semaphoreTest(void);
void readROM(void);
void copyROM(int);
void ROMconsist(void);
void resetSensors(void);
//void altaSec2(void);
void silenceTimerCallback(TimerHandle_t);
void normalTimerCallback(TimerHandle_t);
/* Funciones de testeo */
void testFunc_senAlarm(void);

#define VERSION 0
#define SUBVERSION 6
#define T_SEC 1000 	//tiempo de secuencia(ms)
#define ALARM_TRIGGER 28
#define TAMANO_Q 8

/*Intercommunication ----------------------------------*/
//INTERCOMM TYPES
#define COMMAND 	255
#define ROM			254
#define SENSOR		253
//COMMANDS
#define I_AM		240
#define U_ARE		239
#define SEND_ROM	238
#define PULL_CFG	237
#define ACK			236
/*-----------------------------------------------------*/
//BUTTONS
#define LEFT	0
#define UP 		1
#define DOWN	2
#define RIGHT	3

tempSens_t sensor[N];
char rem_ROM[N][8]; //Matriz de ROM remota
static OWbus_t* p_Bus = NULL; ////arreglar la nomenclatura
static OWbus_t* p_Bus_2 = NULL;
//static uint32_t status;
uint32_t status;
//static uint32_t mascara;

//ToDo: desglobalizar estas variables
//int unknown;
tempSens_t un_buffer;
int ROM_active;///////////////////////////////////////
uint8_t crc;////////////////////////////////////////
int i = 0;///////////////////////////////////////////
int j = 0;///////////////////////////////////////////
int quiet = 0;///////////////////////////////////////
uint8_t modo;////////////////////////////////////////
uint8_t rem_data = 0;////////////////////////////////
uint8_t Sensor_buffer;///////////////////////////////
uint8_t Temp_buffer;/////////////////////////////////
uint8_t Estado_buffer;////////////////////////////////
uint8_t flagAlarm,quantAlarm, flagDegraded, quantActive;
uint8_t systemMode = INITIATION;
QueueHandle_t consoleQueue;
QueueHandle_t intercommQueue;
TimerHandle_t silenceTimer, normalTimer;
struct outgoingMsg_t
{
	int sender;
	char message[10];
};
struct intercommMsg_t
{
	uint8_t msgType;
	uint8_t order;
	char message[8];
	uint8_t crc;
};

int main( void )
{
	consoleQueue = xQueueCreate( TAMANO_Q, sizeof(struct outgoingMsg_t));
	intercommQueue = xQueueCreate( 2*TAMANO_Q, sizeof(struct outgoingMsg_t));
	silenceTimer = xTimerCreate("alarmSilenceTimer", 600000, pdFALSE, (void *) 0, silenceTimerCallback);
	normalTimer = xTimerCreate("HMIbackToNormalTimer", 10000, pdFALSE, (void *) 0, normalTimerCallback);

	xTaskCreate( vTask1,
                (const char *)"Task 1",
                configMINIMAL_STACK_SIZE*2,
                NULL,
                tskIDLE_PRIORITY+1,
                NULL );

   /* Start the scheduler to start the tasks executing. */
   vTaskStartScheduler();

   //for( ;; );
   return 0;
}
/*-----------------------------------------------------------*/

void vTask1( void *pvParameters )
{
	uint8_t auxAlarm, auxActive;
	struct outgoingMsg_t varA;
	varA.sender = TASK_1;
	strcpy(varA.message, "prueba2");//------------------------SACAR!

	BaseType_t res;

	if (SMTB_Init()!=0)
	{
		// ToDo: mensaje de error
		// bloqueo del programa
		while(TRUE)
		{
			gpioToggle (LEDR); // ToDo: Cambiar por led bicolor
			vTaskDelay(300);
		}
	}
/* TEST LOOP--------------------------------------------- */

	buzzer_test();

	lcdGoToXY( 0, 0 ); // MOVER???/////////////////////////
	lcdSendStringRaw( "SMTB Firmware " );                //
	lcdGoToXY( 0, 1 );                                   //
	lcdSendStringRaw( "Version: " );                     //
	lcdData(VERSION + 48);                               //
	lcdSendStringRaw( "." );                             //
	lcdData(SUBVERSION + 48);                            //
	///////////////////////////////////////////////////////

	ledMatrixClear();
	ledMatrixTest();
	semaphoreTest();
	rom_test();
	uci_id_test();
/* FIN TEST LOOP------------------------------------------*/
	for(int i=0;i<N;i++)
	{
		//sensor[i].estado = ACTIVE;
		sensor[i].eje = i/6+1;
		sensor[i].posicion = i%6+1;
	}
	if(getStatus(BIT_UCI_1) && !getStatus(BIT_UCI_2))
		xTaskCreate( sensorMng, (const char *)"SensorMng", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL );
	else if(getStatus(BIT_UCI_2) && !getStatus(BIT_UCI_1))
		xTaskCreate( UCI_2_task, (const char *)"UCI#2 Task", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL );
	else
		;
		// ToDo: manejo de error POSICION

	xTaskCreate(vDisplayMng, (const char *)"Display Task", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL );
    xTaskCreate(vButtonMng, (const char *)"Button Task", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL );
    xTaskCreate(vCommMng, (const char *)"Communication Task", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL );
    xTaskCreate(vConsoleMng, (const char *)"Console Task", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL );

    vTaskDelay(2000);									//
    while( true )
   	{
    	flagAlarm = 0;
    	auxAlarm = 0;
    	auxActive = 0;
    	flagDegraded = 0;
    	for(int k = 0;k<N;k++)
    	{
    		if(sensor[k].estado == SEN_ALARM)
    		{
    			flagAlarm = 1;
    			auxAlarm ++;
    		}
    		if(sensor[k].estado == INACTIVE)
    		{
    			flagDegraded = 1;
    		}
    		if(sensor[k].estado != INACTIVE)
    		{
    			auxActive ++;
    		}
    	}
    	quantAlarm = auxAlarm;
    	quantActive = auxActive;
    	vTaskDelay(1000);
	   //printf("SYSMODE: %d\r\n",systemMode);
	   //Implementar acá el heart beat check
   	}
   return;
}
/*-----------------------------------------------------------*/

void sensorMng( void *pvParameters )
{
	struct intercommMsg_t msgToUci2;
	int next = 0;////////////////////////////////////////
	tempSens_t ee_buffer;
	int unknown;

	while(1)
	{
		/*Inicializa comunicación con UCI remota*/
		if(getStatus(BIT_REMOTE) == FALSE)
		{
			remote_uci_test(UCI_1);
		}

		/*Inicializa bus One Wire #1*/
		if(getStatus(BIT_BUS_1) == FALSE)
		{
			printf("BIT_BUS_1: 0 \r\n");
			p_Bus = OWinit(GPIO2);
			printf("punt_bus: %d \r\n",p_Bus);
			if(p_Bus == NULL)
				printf("BUS INIT FAILED \r\n");
				//ToDo manejo de errores de bus
			else
				setStatus(BIT_BUS_1, TRUE);
		}
		/*Inicializa sensores*/
		else if(getStatus(BIT_SENSORS) == FALSE)
		{
			//printf("1 next: %d \r\n",next);
			next = OWsearch(p_Bus,ALL_DEVICES);// ToDo: manejo de error!!!!!!!!!!!!
			//printf("2 next: %d \r\n",next);
			while(next > 0)
			{
				unknown = 1;
				//printf("sensor encontrado\r\n");
				for(i=0;i<N;i++)
				{
					Board_EEPROM_loadSensor(i, &ee_buffer); // ToDo manejo de errores
					//
					int not_equal_flg = 0;
					for(j=0;j<8;j++)
					{
						if(OneWireStruct->ROM_NO[j]!= ee_buffer.ROM_NO[j])
						{
							//printf("distinto \r\n");///////////////////////////////////////////////////////
							not_equal_flg = 1;
							break; //termina el loop cuando encontró un byte diferente ////ACTIVAR
						}
					}
					if(not_equal_flg == 0)//son iguales
					{
						sensor[i].estado = ACTIVE;
//						sensor[i].eje = i/6+1;
//						sensor[i].posicion = i%6+1;
						unknown = 0;

						//printf("sensor %d - ACTIVO\r\n",i);
						for(int k=0;k<8;k++)
						{
							sensor[i].ROM_NO[k] = ee_buffer.ROM_NO[k];
						}
					}
//					else
//					{
//						printf("sensor desconocido\r\n");
//					}
				}
				if(unknown == 1)
				{
					printf("sensor desconocido\r\n");//ToDo: en else manejo de sensor desconocido
					for(int k=0; k<8;k++)
					{
						printf("%X-",OneWireStruct->ROM_NO[k]);
						un_buffer.ROM_NO[k]=OneWireStruct->ROM_NO[k];
					}
					printf("\r\n");
				}
				next = OWsearch(p_Bus,ALL_DEVICES);
			}
			setStatus(BIT_SENSORS,TRUE);
			systemMode = NORMAL;
		}

		if(getStatus(BIT_SENSORS)) //si hay bus inicializado
		{
			//printf("BIT_BUS: 1 \r\n");
			for(j=0;j<N;j++)
			{
				if(sensor[j].estado != INACTIVE)
				{
					taskENTER_CRITICAL();
					sensor[j].temp = DSreadROMtemp(p_Bus->OWport,p_Bus->OWpin, j);
					taskEXIT_CRITICAL();
					msgToUci2.msgType = SENSOR;
					msgToUci2.order = j;
					msgToUci2.message[0] = sensor[j].temp;
					if(sensor[j].temp > ALARM_TRIGGER)
					{
						sensor[j].estado = SEN_ALARM;
						//systemMode = SYS_ALARM;
						//flagAlarm = 1;
					}
					else
					{
						sensor[j].estado = ACTIVE;
					}
					msgToUci2.message[1] = sensor[j].estado;
					xQueueSend(intercommQueue, &msgToUci2, 1000);
					vTaskDelay(1000);
				}
			}
			DSmultipleConv(p_Bus->OWport,p_Bus->OWpin);
//			systemMode = NORMAL;
		}
	}
	vTaskDelay(1000);
}

void UCI_2_task( void *pvParameters )
{
	systemMode = NORMAL;
	while(1)
	{
		//printf("UCI#2 IN \r\n ");
		vTaskDelay(1000);
	}
}

void vDisplayMng( void *pvParameters )
{
   	int k;
   //while( systemMode == 1 )
	while( true)
   {
		switch(systemMode)
		{
			case INITIATION:
				lcdGoToXY( 13, 1 );
				lcdData(0x32);
				lcdGoToXY( 14, 1 );
				lcdData(0x7F);
				lcdGoToXY( 15, 1 );
				lcdData(0x31);
				break;
			case NORMAL:
			//case DEGRADED:
			//case SYS_ALARM:
				for (k=0; k<N; k++)
				{
					gpioToggle(LEDB);
					lcdClear();
					taskENTER_CRITICAL();
					SMTB_Temp_Show(sensor[k].temp, sensor[k].eje, sensor[k].posicion, sensor[k].estado);
					taskEXIT_CRITICAL();
					SMTB_Mode_Show();
					ledMatrixClear();
					for (int i=0; i<6; i++)
					{
						writeRow(sensor[k].eje,0x80>>(sensor[k].posicion));
						if(sensor[k].estado == SEN_ALARM)
							gpioWrite(SEM_RED, ON);
						else if(sensor[k].estado == INACTIVE)
						{
							gpioWrite(SEM_GREEN, ON);
							gpioWrite(SEM_RED, ON);
						}
						else
							gpioWrite(SEM_GREEN, ON);
						vTaskDelay(150);
						if(sensor[k].estado != ACTIVE) //titilar
						{
							ledMatrixClear(); // ojo que apaga todos los leds!!!
							gpioWrite(SEM_GREEN, OFF); ///ToDo: función sAPI!!!
							gpioWrite(SEM_RED, OFF);
						}
						if(sensor[k].estado == SEN_ALARM && quiet == 0)
						{
							gpioToggle(GPIO4); // Buzzer ///ToDo: función sAPI!!!
							//ToDo: chequear timer de silenciado
						}
							vTaskDelay(150);
					}
					if(systemMode != NORMAL)
						break;
				}
				break;
			case MAIN_MENU:
				ledMatrixClear();
				SMTB_Menu_Show();
				break;
			case ALARM_INIT:
				ledMatrixClear();
				SMTB_Menu_Show();
				break;
			case ALARM_VIEW:
				ledMatrixClear();
				SMTB_Alarm_View_Init();
				break;
			case SENS_INIT:
				ledMatrixClear();
				SMTB_Menu_Show();
				break;
			case ALTA_SEC:
				ledMatrixClear();
				altaSecUpdate();
				break;
			case ALTA_SEC_2:
				ledMatrixClear();
				altaSec2Update();
				break;
			case ALTA_INIT:
				ledMatrixClear();
				SMTB_Alta_Init();
				break;
			case ALTA_EJE:
			case ALTA_POS:
				ledMatrixClear();
				SMTB_Menu_Show();
				break;
			case ALTA_CONF:
				ledMatrixClear();
				break;
			case SENS_VIEW:
				ledMatrixClear();
				SMTB_Sens_View_Init();
				break;
			case ROM_INIT:
				ledMatrixClear();
				SMTB_Menu_Show();
				break;
			case ROM_CONSIST:
				ledMatrixClear();
				SMTB_ROM_consist();
				systemMode = ROM_CONSIST_2;
				break;
			case ROM_CONSIST_2:
				while(getStatus(BIT_ROM_CONS_CHK)== FALSE); //ToDo manejo de error (NORMAL timer)
				if(getStatus(BIT_ROM_CONSIST))
					SMTB_ROM_consist_OK();
				else
					SMTB_ROM_consist_NOK();
				vTaskDelay(4000);
				systemMode = NORMAL;
				break;
			case ROM_COPY:
				SMTB_Menu_Show();
				break;
			case ROM_COPY_2:
				SMTB_ROM_copying();
				systemMode = ROM_COPY_3;
				break;
			case ROM_COPY_3:
				break;
			case ROM_COPY_4:      // en prueba de eliminacion
				SMTB_ROM_consist();
				ROMconsist();
				SMTB_ROM_copying();
				copyROM(2);
				systemMode = NORMAL;
				break;
			case LOOP:
				SMTB_Menu_Show();
				break;
			case STATUS:
				SMTB_Status_Show();
				break;
			case CHK_MATRIX:
				ledMatrixTest();
				systemMode = NORMAL;
				break;
			case CHK_BUZZER:
				buzzer_test();
				systemMode = NORMAL;
				break;
			case CHK_SEMAPH:
				semaphoreTest();
				systemMode = NORMAL;
				break;
			case PENDING:
				SMTB_pending();
				vTaskDelay(2000);
				systemMode = NORMAL;
				break;
			default:
			{
				//SMTB_Temp_Show(00,37);
			}
		}
		vTaskDelay(1000);
    }
}

void vButtonMng( void *pvParameters )
{
	int button;
	tempSens_t SensorPrueba; ////ToDo: cambiar nombre

	vTaskDelay(2000);//////////////////chequear
	while(1)
	{
		//buttons_test();
		button = buttonsUpdate();
		//printf("Boton %d recibido - vButtonMng\r\n", button);
		if (button != -1)
		{
			switch(systemMode)
			{
				//case SYS_ALARM:
				//case DEGRADED:
				case NORMAL:
					if(button == LEFT)
					{
						systemMode = MAIN_MENU;
						menuInit();
					}
					break;
				case MAIN_MENU:
					menuUpdate(button);
					break;
				case ALARM_INIT:
					menuAlarmUpdate(button);
					break;
				case ALARM_VIEW:
					menuViewUpdate(button);
					break;
				case SENS_INIT:
					menuSensUpdate(button);
					break;
				case ALTA_SEC:
					if(button == RIGHT)
					{
						systemMode = ALTA_EJE;
						SensorPrueba = un_buffer;
						menuEjeInit();
					}
					if(button == LEFT)
						systemMode = NORMAL;
					break;
				case ALTA_SEC_2:
					if(button == LEFT)
						systemMode = NORMAL;
					break;
				case ALTA_INIT:
					if(button == RIGHT)
					{
						systemMode = ALTA_EJE;
						SensorPrueba = altaUpdate(); //ToDo: manejo de error
						menuEjeInit();
					}
					if(button == LEFT)
						systemMode = MAIN_MENU;
					break;
				case ALTA_EJE:
					SensorPrueba.eje = menuEjeUpdate(button);
					if(button == RIGHT)
					{
						systemMode = ALTA_POS;
						//altaUpdate();
						menuPosInit();
					}
					break;
				case ALTA_POS:
					SensorPrueba.posicion = menuPosUpdate(button);
					//printf("posicion devuelta: %d\r\n",SensorPrueba.posicion); //test
					if(SensorPrueba.posicion != -1)
					{
						systemMode = ALTA_CONF;
						menuConfInit(SensorPrueba.eje, SensorPrueba.posicion);
					}
					break;
				case ALTA_CONF:
					if(button == RIGHT)
					{
						int aux = (SensorPrueba.eje - 1)*6 + SensorPrueba.posicion-1;
						if(Board_EEPROM_recSensor(aux,SensorPrueba)!=0)
							;//aviso de falla
						else
							systemMode = NORMAL;
					}
					if(button == LEFT)
						systemMode = NORMAL;
					break;
				case SENS_VIEW:
					menuViewUpdate(button);
					break;
				case ROM_INIT:
					menuROMupdate1(button);
					break;
				case ROM_CONSIST:
					//menuROMupdate1(button);
					break;
				case ROM_COPY:
					menuROMupdate(button);
					break;
				case ROM_COPY_2:
					break;
				case ROM_COPY_3:
					break;
				case ROM_COPY_4:
					//ROMconsist();
					//systemMode = NORMAL;
					break;
				case LOOP:
					menuLoopUpdate(button);
					break;
				case STATUS:
					break;
				case CHK_MATRIX:
					break;
				case CHK_BUZZER:
					break;
				case CHK_SEMAPH:
					break;
			}
		}
		vTaskDelay(40);// este marca la cadencia de los botones (antirrebote)
	}
}

void vConsoleMng( void *pvParameters )
{
	struct outgoingMsg_t msgBuffer,* p_msgBuffer;
	p_msgBuffer = &msgBuffer;

	while(1)
	{
		xQueueReceive(consoleQueue, p_msgBuffer, portMAX_DELAY);
		switch(msgBuffer.sender)
		{
		case TASK_1:
			printf("<TASK_1> ");
			printf("%s\n\r",msgBuffer.message);
			break;
		case UCI_1:
			printf("<UCI_1> ");
			printf("%s\n\r",msgBuffer.message);
			break;
		case UCI_2:
			printf("<UCI_2> ");
			printf("%s\n\r",msgBuffer.message);
			break;
		}
	}
}

void vCommMng( void *pvParameters )
{
	uint8_t recvbyte;
	tempSens_t aux;
	uint8_t pos_aux;
	int reserve;
	struct intercommMsg_t msgBuffer, inMsg, outMsg, * p_msgBuffer;
	p_msgBuffer = &msgBuffer;
	printf("<ICOMM> Initializing \r\n");

	while(1)
	{
		//Transmisión
		if(xQueueReceive(intercommQueue, p_msgBuffer, 100) == pdTRUE)
		{
			while(uartTxReady( UART_485 )==0)
			{
				vTaskDelay(1);
				printf("waiting\r\n");
			}
			uartWriteByteArray( UART_485,p_msgBuffer, 11);
			if(getStatus (BIT_UCI_2)) ////////////////////////////sacar
				printf("enviando x uart\r\n");//
		}
		//Recepción
		if(uartRxReady( UART_485))
		//if(uartReadByte( UART_485, &recvbyte )) //de prueba
		{
			uartReadByte( UART_485, &recvbyte );
			//printf("ICOMM recibido: %d\r\n",recvbyte);
			printf("\n"); // Si lo saco, no anda!!!//////////////////
			switch(recvbyte)
			{
				case COMMAND:
					printf("<COMMAND> ");
					uartReadByte( UART_485, &recvbyte );
					//printf("ICOMM recibido: %d\r\n",recvbyte);
					switch(recvbyte)
					{
						case I_AM:
							printf("<I AM> ");
							uartReadByte( UART_485, &recvbyte );
							if(recvbyte == UCI_1)
							{
								printf("< UCI#1 >\r\n ");
								setStatus(BIT_REMOTE,TRUE);
								outMsg.msgType = COMMAND;
								outMsg.order = U_ARE;
								outMsg.message[0] = UCI_1;
								xQueueSend(intercommQueue, &outMsg, 1000);
							}
							//ToDo: agregar UCI#2
							uartRxFlush( UART_485 );
							break;
						case U_ARE:
							printf("<U_ARE> ");
							uartReadByte( UART_485, &recvbyte );
							if(recvbyte == UCI_1)
							{
								printf("< UCI#1 >\r\n ");
								setStatus(BIT_REMOTE,TRUE);
							}
							break;
						case SEND_ROM:
							while(uartRxReady( UART_485) == 0)
								vTaskDelay(1);///////////////
							printf("<ROM SEND REQUESTED>\r\n ");
							uartReadByte( UART_485, &recvbyte );
							//for(uint8_t i=0; i<N; i++)
							//{
								msgBuffer.msgType = ROM;
								msgBuffer.order = recvbyte;
								msgBuffer.crc = recvbyte; //carga crc con valor inicial
								Board_EEPROM_loadSensor(msgBuffer.order, &aux);
								for(int j=0; j<8; j++)
								{
									msgBuffer.message[j] = aux.ROM_NO[j]; // PRUEBA/////////////////////////
									msgBuffer.crc += msgBuffer.message[j]; //actualiza crc
								}
								xQueueSend(intercommQueue, &msgBuffer, TIMEOUT);
								//vTaskDelay(1000);
							//}
							break;
						case PULL_CFG:
							printf("<Pulling config> \r\n");
							systemMode = ROM_COPY_4;
							break;
					}
					break;
				case SENSOR:
					//printf("<SENSOR> ");
					if(uartRxReady( UART_485))
					{
						uartReadByte( UART_485, &recvbyte );
						//printf("< %d > ",recvbyte);
						pos_aux = recvbyte;
					}
					else
						break;
					if(uartRxReady( UART_485))
					{
						uartReadByte( UART_485, &recvbyte );
						//printf("< temp = %d > ",recvbyte);
						aux.temp = recvbyte;
						//printf("< temp = %d > ",aux.temp);
					}
					else
						break;
					if(uartRxReady( UART_485))
					{
						uartReadByte( UART_485, &recvbyte );
						//printf("< estado = %d >\r\n",recvbyte);
						aux.estado = recvbyte;
					//	printf("< estado = %d >\r\n",aux.estado);
					}
					else
						break;
					sensor[pos_aux].temp = aux.temp;
					sensor[pos_aux].estado = aux.estado;
					uartRxFlush( UART_485 );
					break;
				case ROM:
					reserve =-1;
					//ROM_flag =-1; //ToDo: no es un flag, cambiar nombre

					printf("rec ROM ");
					while(uartRxReady( UART_485) == 0)
						vTaskDelay(1);///////////////
					uartReadByte( UART_485, &recvbyte );
					reserve = recvbyte; //Reserva el num de ROM para utilizar despues
					printf("num: %d\r\n",reserve);
					crc = recvbyte;//primera carga de crc
					Board_EEPROM_loadSensor(reserve, &aux);
					for(int i=0;i<8;i++)
					{
						while(uartRxReady( UART_485) == 0)
							vTaskDelay(1);///////////////
						uartReadByte( UART_485, &recvbyte );
						printf("recibo ROM byte: %X\r\n",recvbyte);
						rem_ROM[reserve][i] = recvbyte;// se usa en rom copy
						if(recvbyte != aux.ROM_NO[i])
							setStatus(BIT_ROM_CONSIST, FALSE);
						crc += recvbyte;
					}
					while(uartRxReady( UART_485) == 0)
						vTaskDelay(1);///////////////
					uartReadByte( UART_485, &recvbyte );
					printf("rec CRC: %d\r\n",recvbyte);
					printf("calcCRC: %d\r\n",crc);
					if(crc == recvbyte)
						ROM_active = reserve;
					printf("==========================\r\n");
					//switch(recvbyte)
						//{
					break;
			}
		}
	}
}

tempSens_t altaUpdate(void)
{
	int next;
	tempSens_t Sensor;

	if(getStatus(BIT_BUS_2) == FALSE)
	{
		p_Bus_2 = OWinit2(GPIO6);
		printf("punt_bus: %d \r\n",p_Bus_2);
	}
	else
	{
		printf("bus inicializado \r\n");
	}

	if(p_Bus_2 == NULL)
		printf("BUS INIT FAILED \r\n");
	else
		setStatus(BIT_BUS_2,TRUE);
	if(!OWpresence(p_Bus_2->OWport,p_Bus_2->OWpin))
//	{
		printf("SENSOR DETECTADO \r\n");
		next = OWsearch(p_Bus_2,ALL_DEVICES);// ToDo: manejo de error!!!!!!!!!!!!
		printf("next: %d\r\n",next);
		if(next == 1)
		{
			printf("sensor: ");
			for(int k=0;k<8;k++)
			{
				printf("%X ",p_Bus_2->ROM_NO[k]);
				Sensor.ROM_NO[k] = p_Bus_2->ROM_NO[k];
			}
			printf("\r\n");
		}
		else
			printf("sensor NO detectado\r\n");
		return(Sensor);
//	}
//	else
//		printf("SENSOR NO DETECTADO \r\n");
}

void menuUpdate(int button)
{
	//int next;
	//int prueba;
	//char buffer[9];

	struct intercommMsg_t varB;
	varB.msgType = 1;

	switch(button)
	{
		case LEFT:
			systemMode = NORMAL;
			break;
		case UP:
			itemSelected --;
			if (itemSelected < windowStart)
				windowStart --;
			if (itemSelected == 0)
				itemSelected = 1;
			break;
		case DOWN:
			itemSelected ++;
			if (itemSelected >= windowStart + WINDOW_HEIGHT)
				windowStart ++;
			if(windowStart > 1)
			{
				windowStart = 1;
				itemSelected = 4;
			}
			break;
		case RIGHT:
			switch(itemSelected)
			{
				case 0:
					break;
				case 1: //SUBMENU ALARMAS
					menuAlarmInit();
					systemMode = ALARM_INIT;
					break;
				case 2://SUBMENU SENSORES
					menuSensInit();
					systemMode = SENS_INIT;
					break;
				case 3://SUBMENU ROM
					menuROMInit();
					systemMode = ROM_INIT;
					break;
				case 4://SUBMENU MANTENIMIENTO
					menuLoopInit();
					systemMode = LOOP;
					break;
			}
			break;
		default:
			;
	}
}

void menuAlarmUpdate(int button)
{
	switch(button)
	{
		case LEFT:
			systemMode = NORMAL;
			break;
		case UP:
			itemSelected --;
			if (itemSelected == 0)
				itemSelected = 1;
			break;
		case DOWN:
			itemSelected ++;
			if (itemSelected > 1)
				itemSelected = 2;
			break;
		case RIGHT:
			switch(itemSelected)
			{
				case 0:
					break;
				case 1://SILENCIAR ALARMAS
					quiet = 1;
					xTimerStart(silenceTimer,0);
					systemMode = NORMAL;
					break;
				case 2://VISUALIZAR ALARMAS
					xTimerStart(normalTimer,0);
					systemMode = ALARM_VIEW;
					break;
			}
			break;
		default:
			;
	}
}

void menuViewUpdate(int button)
{
	switch(button)
	{
	//cualquier tecla saca a modo normal
		case LEFT:
		case UP:
		case DOWN:
		case RIGHT:
			systemMode = NORMAL;
			break;
	}
}

void menuSensUpdate(int button)
{
	switch(button)
	{
		case LEFT:
			systemMode = NORMAL;
			break;
		case UP:
			itemSelected --;
			if (itemSelected < windowStart)
				windowStart --;
			if (itemSelected == 0)
				itemSelected = 1;
			break;
		case DOWN:
			itemSelected ++;
			if (itemSelected >= windowStart + WINDOW_HEIGHT)
				windowStart ++;
			if(windowStart > 1)
			{
				windowStart = 1;
				itemSelected = 4;
			}
			break;
		case RIGHT:
			switch(itemSelected)
			{
				case 0:
					break;
				case 1: // Reset de bus
					resetSensors();
					break;
				case 2: // Alta bus #2
					systemMode = ALTA_INIT;
					//readROM();
					break;
				case 3:// Alta secuencial
					if(un_buffer.ROM_NO[0] == 0x28)
					{
						systemMode = ALTA_SEC;
						//xTimerStart(normalTimer,0);
						//printf("1 desconocido \r\n");
					}
					else
					{
						systemMode = ALTA_SEC_2;
						xTimerStart(normalTimer,0);
						//printf("0 desconocido \r\n");
					//systemMode = SENS_INIT;
					//systemMode = ALTA_INIT;
					}
					break;
				case 4:
					xTimerStart(normalTimer,0);
					systemMode = SENS_VIEW;
					break;
				case 5:
					//ROMconsist();
					break;
				case 6:
					break;
				//insertar aqui las funciones de menu
			}
			break;
		default:
			;
	}
}

int menuEjeUpdate(int button)
{
	int aux_eje;
	switch(button)
	{
		case UP:
			itemSelected --;
			if (itemSelected < windowStart)
				windowStart --;
			if (itemSelected == 0)
				itemSelected = 1;
			break;
		case DOWN:
			itemSelected ++;
			if (itemSelected >= windowStart + WINDOW_HEIGHT)
				windowStart ++;
			if(windowStart > 3)
			{
				windowStart = 3;
				itemSelected = 6;
			}
			break;
		case RIGHT:
			aux_eje = itemSelected;///Almacenar !!!!!!!!!!!!!!!!!!!!!!!!
			printf("eje seleccionado: %d\r\n",aux_eje); //test
			break;
		case LEFT:
			systemMode = NORMAL;
			break;
		default:
			break;
	}
	return(aux_eje);
}

int menuPosUpdate(int button)
{
	int aux_pos = -1;
	//printf("switch\r\n"); //test
	switch(button)
	{
		case LEFT:
			systemMode = NORMAL;
			break;
		case UP:
			itemSelected --;
			if (itemSelected < windowStart)
				windowStart --;
			if (itemSelected == 0)
				itemSelected = 1;
			break;
		case DOWN:
			itemSelected ++;
			if (itemSelected >= windowStart + WINDOW_HEIGHT)
				windowStart ++;
			if(windowStart > 3)
			{
				windowStart = 3;
				itemSelected = 6;
			}
			break;
		case RIGHT:
			aux_pos = itemSelected;///Almacenar!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			printf("posicion seleccionada: %d\r\n",aux_pos); //test
			break;
		default:
			printf("default\r\n"); //test
			break;
	}
	printf("posicion retornada: %d",aux_pos);
	return(aux_pos);
}

void menuROMupdate1(int button)
{
	int aux_ROM;

	switch(button)
	{
	case UP:
		itemSelected --;
		if (itemSelected < windowStart)
			windowStart --;
		if (itemSelected == 0)
			itemSelected = 1;
		break;
	case DOWN:
		itemSelected ++;
		if (itemSelected > 2)
			itemSelected = 3;
		break;
	case RIGHT:
		switch(itemSelected)
		{
		case 0:
			break;
		case 1: // Consistencia
			systemMode = ROM_CONSIST;
			ROMconsist();
			break;
		case 2: // Copia de ROM
			menuROMcopyInit();
			systemMode = ROM_COPY;
			break;
		case 3://ROM dump
			readROM();
			systemMode = NORMAL;

			break;
		}
		break;
	case LEFT:
		systemMode = NORMAL;
		break;
	default:
		break;
	}
}

void menuROMupdate(int button)
{
	int aux_ROM;

	switch(button)
	{
	case UP:
		itemSelected --;
		if (itemSelected < windowStart)
			windowStart --;
		if (itemSelected == 0)
			itemSelected = 1;
		break;
	case DOWN:
		itemSelected ++;
		if (itemSelected > 2)
			itemSelected = 2;
		break;
	case RIGHT:
		aux_ROM = itemSelected;///Almacenar !!!!!!!!!!!!!!!!!!!!!!!!
		printf("ROM seleccionada: %d\r\n",aux_ROM); //test
		if(aux_ROM == 1)
			copyROM(1);
		else if(aux_ROM == 2)
			copyROM(2);
		else
			systemMode = NORMAL;//ToDo: log de error
		xTimerStart(normalTimer,0);
		break;
	case LEFT:
		systemMode = NORMAL;
		break;
	default:
		break;
	}
}

void menuLoopUpdate(int button)
{
	switch(button)
	{
		case LEFT:
			systemMode = NORMAL;
			break;
		case UP:
			itemSelected --;
			if (itemSelected < windowStart)
				windowStart --;
			if (itemSelected == 0)
				itemSelected = 1;
			break;
		case DOWN:
			itemSelected ++;
			if (itemSelected >= windowStart + WINDOW_HEIGHT)
				windowStart ++;
			if(windowStart > 2)
			{
				windowStart = 2;
				itemSelected = 5;
			}
			break;
		case RIGHT:
			switch(itemSelected)
			{
			case 0:
				break;
			case 1: // Ver estados
				xTimerStart(normalTimer,0);
				systemMode = STATUS;
				break;
			case 2: // Check matriz
				systemMode = CHK_MATRIX;
				break;
			case 3:// Check buzzer
				systemMode = CHK_BUZZER;
				break;
			case 4: // Check semáforo
				systemMode = CHK_SEMAPH;
				break;
			}
			break;
		default:
			printf("default\r\n"); //test
			break;
	}
}

void altaSecUpdate(void)
{
	lcdClear();
	lcdGoToXY( 0,0 );
	lcdSendStringRaw( "Sensor detectado: " );
	lcdGoToXY( 0,2 );
	lcdSendStringRaw( "Ingresar en ROM?" );
	lcdGoToXY( 0,3 );
	lcdSendStringRaw( "NO                SI" );
}

void altaSec2Update(void)
{
	lcdClear();
	lcdGoToXY( 0,0 );
	lcdSendStringRaw( "Sensor no detectado: " );
	lcdGoToXY( 0,1 );
	lcdSendStringRaw( "Conecte el sensor de " );
	lcdGoToXY( 0,2 );
	lcdSendStringRaw( "recambio y haga " );
	lcdGoToXY( 0,3 );
	lcdSendStringRaw( "reset del bus" );
}

void menuInit(void)
{
	/* Menu inicial*/

	strcpy(menu[0], "MENU:");
	strcpy(menu[1], "Alarmas");
	strcpy(menu[2], "Sensores");
	strcpy(menu[3], "ROM");
	strcpy(menu[4], "Mantenimiento");
	strcpy(menu[5], " ");
	strcpy(menu[6], " ");

	windowStart = 0;
	itemSelected = 1;
}

void menuAlarmInit(void)
{
	/* Menu Alarmas*/

	strcpy(menu[0], "ALARMAS:");
	strcpy(menu[1], "Silenciar");
	strcpy(menu[2], "Visualizar");
	strcpy(menu[3], " ");
	strcpy(menu[4], " ");
	strcpy(menu[5], " ");
	strcpy(menu[6], " ");

	windowStart = 0;
	itemSelected = 1;
}

void menuSensInit(void)
{
	/* Menu SENSORES */

	strcpy(menu[0], "SENSORES:");
	strcpy(menu[1], "Reset/Act");
	strcpy(menu[2], "Alta");
	strcpy(menu[3], "Recambio");
	strcpy(menu[4], "Visualizar");

	windowStart = 0;
	itemSelected = 1;
}

void menuROMInit(void)
{
	/* Menu Alarmas*/

	strcpy(menu[0], "ROM:");
	strcpy(menu[1], "Consistencia");
	strcpy(menu[2], "Copiar");
	strcpy(menu[3], "Dump (USB)");
	strcpy(menu[4], " ");
	strcpy(menu[5], " ");
	strcpy(menu[6], " ");

	windowStart = 0;
	itemSelected = 1;
}

void menuLoopInit(void)
{
	strcpy(menu[0], "MANTENIMIENTO:");
	strcpy(menu[1], "Ver estados");
	strcpy(menu[2], "Check matriz");
	strcpy(menu[3], "Check buzzer ");
	strcpy(menu[4], "Check LED de estado");
	strcpy(menu[5], " ");
	strcpy(menu[6], " ");

	windowStart = 0;
	itemSelected = 1;
}

void menuEjeInit(void)
{
	/* Menu inicial*/

	strcpy(menu[0], "Par montado?");
	strcpy(menu[1], "1");
	strcpy(menu[2], "2");
	strcpy(menu[3], "3");
	strcpy(menu[4], "4");
	strcpy(menu[5], "5");
	strcpy(menu[6], "6");

	windowStart = 0;
	itemSelected = 1;
}

void menuPosInit(void)
{
	/* Menu inicial*/

	strcpy(menu[0], "Posicion?");
	strcpy(menu[1], "1");
	strcpy(menu[2], "2");
	strcpy(menu[3], "3");
	strcpy(menu[4], "4");
	strcpy(menu[5], "5");
	strcpy(menu[6], "6");

	windowStart = 0;
	itemSelected = 1;
}

void menuConfInit(int aux_eje, int aux_pos)
{
	lcdClear();
	printf("menuConfInit\r\n");
	lcdGoToXY( 0,0 );
	lcdSendStringRaw( "Par montado: " );
	lcdData(aux_eje + 48);
	lcdGoToXY( 0,1 );
	lcdSendStringRaw( "Posicion: " );
	lcdData(aux_pos + 48);
	lcdGoToXY( 5,2 );
	lcdSendStringRaw( "Confirma? " );
	lcdGoToXY( 0,3 );
	lcdSendStringRaw( "NO                SI" );
}

void menuROMcopyInit(void)
{
	/* Menu inicial*/

	strcpy(menu[0], "Copiar ROM: ");
	strcpy(menu[1], "Loc a Rem");
	strcpy(menu[2], "Rem a Loc");
	strcpy(menu[3], " ");
	strcpy(menu[4], " ");
	strcpy(menu[5], " ");
	strcpy(menu[6], " ");

	windowStart = 0;
	itemSelected = 1;
}

void testFunc_senAlarm(void)
{
	gpioWrite(GPIO4,ON); // Buzzer
	delay(100);
	gpioWrite(GPIO4,OFF); // Buzzer
	sensor[3].estado = SEN_ALARM;
}

//ToDo: pasar a modulo HAL semaforo
void semaphoreInit(void)
{
	gpioInit(SEM_RED, GPIO_OUTPUT);
	gpioInit(SEM_GREEN, GPIO_OUTPUT);
}

////ToDo: pasar a modulo HAL semaforo
//void semaphoreTest(void)
//{
//	gpioWrite( SEM_RED, ON );
//	vTaskDelay(500);
//	gpioWrite( SEM_GREEN, ON );
//	vTaskDelay(500);
//	gpioWrite( SEM_RED, OFF );
//	vTaskDelay(500);
//	gpioWrite( SEM_GREEN, OFF );
//}

void readROM(void)
{
	tempSens_t buffer;
	printf("leido EEPROM:\r\n");
	for(int i=0; i<N+1; i++ )
	{
		Board_EEPROM_loadSensor(i, &buffer);
		for(int j=0; j<8; j++ )
			printf("%X - ",buffer.ROM_NO[j]);
		printf("\r\n");
		delay(1000);
	}
}

void copyROM(int source)
{
	ROM_active = 0;
	tempSens_t aux;
	struct intercommMsg_t msgToRem;
	int equal_flag;

	systemMode = ROM_COPY_2;
	if(source == 1)
	{
		printf("copiar ROM local en remota\r\n");
		//mandar mensaje LOC-REM
		msgToRem.msgType = COMMAND;
		msgToRem.order = PULL_CFG;
		xQueueSend(intercommQueue, &msgToRem, 1000);
	}
	else if(source == 2)
	{
		if (getStatus(BIT_ROM_CONS_CHK) == FALSE)
		{
			printf("consistency check not run\r\n");
			SMTB_ROM_constCheckNotRun();
			return;
		}
		if (getStatus(BIT_ROM_CONSIST))
		{
			printf("ROMs are consistent\r\n");
			SMTB_ROM_constOk();
			return;
		}
		for(int i=0; i<N; i++)
		{
			//load
			//Board_EEPROM_loadSensor(i, &aux);
			//change ROM
			for(int j=0; j<8;j++)
				aux.ROM_NO[j] = rem_ROM[i][j];
			//rec
			Board_EEPROM_recSensor(i, aux);
			//check
			equal_flag = 1;
			Board_EEPROM_loadSensor(i, &aux);
			for(int j=0; j<8;j++)
				if(aux.ROM_NO[j] != rem_ROM[i][j])
					equal_flag = 0;
			if(equal_flag != 1)
				i--;
			else
			{
				ROM_active = i;
				SMTB_ROM_consist_update();
			}
			vTaskDelay(500);
		}
		printf("ROM copy done\r\n");
		systemMode = NORMAL;
	}
	else
		;//ToDo: manejo de errores
}

void ROMconsist(void)
{
	ROM_active=-1;
	setStatus(BIT_ROM_CONSIST, TRUE);
	setStatus(BIT_ROM_CONS_CHK, FALSE);
	//con_check = 0;
	for(int k=0;k<N;k++)
	{
		ROM_request(k);
		vTaskDelay(1000);///////////bajar?
		printf("ROM activa:%d\r\n",ROM_active);
		if(ROM_active != k)
			k--;
		else
			SMTB_ROM_consist_update();
	}
	setStatus(BIT_ROM_CONS_CHK, TRUE);
	//con_check = 1;
	if(getStatus(BIT_ROM_CONSIST))
	{
		printf("ROMs consistency Ok\r\n");//ToDo: chequear flag de inconsistencia aca
		//SMTB_ROM_consist_OK();
	}
	else
	{
		printf("ROM consistency Nok\r\n");
		//SMTB_ROM_consist_NOK();
	}
}

void resetSensors(void)
{
	setStatus(BIT_SENSORS, FALSE);
}

void silenceTimerCallback(TimerHandle_t silenceTimer)
{
	quiet = 0;
}

void normalTimerCallback(TimerHandle_t normalTimer)
{
	systemMode = NORMAL;
	//printf("normal timer expired\r\n");
}
