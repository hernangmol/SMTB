/*============================================================================
 * Módulo de comunicación protocolo 1 Wire.
 * Archivo: oneWire.h
 * Versión: 1.2
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

#ifndef MIS_PROGS_PCSE_TP_FINAL_INC_ONEWIRE_H_
#define MIS_PROGS_PCSE_TP_FINAL_INC_ONEWIRE_H_

#include "OW_port.h"

#define ALL_DEVICES 0xF0
#define ALARM_DEVICES 0xEC


/*=====[Declaración de funciones públicas]===================================*/

char OWcrc(char* code, char n);
int  OWpresence(int, int);
void OWcommand(char, char *, char, int, int);
int  OWreadROM(char *, int, int);
char OWsearch(OWbus_t* , char);
void OWwriteByte(OWbus_t* OneWireStruct, char byte);
char OWreadByte(OWbus_t*);
void OWresetSearch(OWbus_t*);

#endif /* MIS_PROGS_PCSE_TP_FINAL_INC_ONEWIRE_H_ */
