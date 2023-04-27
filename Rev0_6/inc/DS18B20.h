/*============================================================================
 * Módulo de comunicación protocolo 1 Wire.
 * Archivo: DS18B20.h
 * Versión: 1.2
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/


#ifndef MIS_PROGS_PCSE_TP_FINAL_INC_DS18B20_H_
#define MIS_PROGS_PCSE_TP_FINAL_INC_DS18B20_H_

#include "oneWire.h"
#include "SUCI.h"

//SENSOR STATUSES
#define INACTIVE 	 0
#define ACTIVE  	 1
#define SEN_ALARM    2

/*=====[Declaración de tipos de datos públicos]==============================*/

typedef struct
{
	char ROM_NO[8];
	char estado;
	char  temp;
	int eje;
	int posicion;
}
tempSens_t;

extern tempSens_t sensor[N];

/*=====[Declaración de funciones públicas]===================================*/

int  DSreadScratch(char *, int, int);
int  DSreadTemperature(int, int);
void DSmultipleConv(int, int);
int  DSreadROMtemp(int , int ,int);
int  DSwriteScratch(char *, int, int);

#endif /* MIS_PROGS_TP_PDM_INC_SMTB_ONEWIRE_H_ */
