/*============================================================================
 * Módulo para prueba de driver para sensor DS18B20.
 * Archivo: PCSE_TP.h
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __SUCI_H__
#define __SUCI_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>
#include <sapi.h>

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/
#define N 24
// features
#define BIT_EEPROM 			0x80000000 //bit #31
#define BIT_UCI_1 			0x40000000 //bit #30
#define BIT_UCI_2 			0x20000000 //bit #29
#define BIT_REMOTE			0x10000000 //bit #28
#define BIT_BUS_1			0x08000000 //bit #27
#define BIT_BUS_2			0x04000000 //bit #26
#define BIT_SENSORS			0x02000000 //bit #25
#define BIT_ROM_1			0x01000000 //bit #24 // ToDo: pendiente conectar y estado
#define BIT_ROM_2			0x00800000 //bit #23 // ToDo: pendiente conectar y estado
#define BIT_ROM_CONS_CHK	0x00400000 //bit #22
#define BIT_ROM_CONSIST		0x00200000 //bit #21

//CONSOLE QUEUE SENDERS
#define TASK_1	0
#define UCI_1	1
#define UCI_2 	2

//SYSTEM MODES/SUBMODES       ToDo: unificar!!!
#define INITIATION				 0
#define NORMAL 					 1
#define MAIN_MENU				 2
#define		ALARM_INIT 			30
#define		ALARM_VIEW			31
#define 	SENS_INIT			40
#define 		ALTA_SEC		43
#define			ALTA_SEC_2		44
#define			ALTA_SEC_3		45
#define 		ALTA_INIT 		53
#define 		ALTA_EJE		54
#define 		ALTA_POS		55
#define 		ALTA_CONF		56
#define			SENS_VIEW		57
#define		ROM_INIT			60
#define 		ROM_CONSIST		71
#define 		ROM_CONSIST_2	72
#define 		ROM_COPY		73
#define 		ROM_COPY_2		74
#define 		ROM_COPY_3		75
#define 		ROM_COPY_4		76
#define 	LOOP				80
#define 		STATUS			81
#define			CHK_MATRIX		82
#define			CHK_BUZZER		83
#define			CHK_SEMAPH		84
#define SYSTEM_ERROR			90
#define PENDING					91


/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

/*=====[Prototypes (declarations) of public functions]=======================*/

int SMTB_Init (void);
int setStatus(uint32_t, bool_t);
bool_t getStatus(uint32_t feature);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __SUCI_H__ */
