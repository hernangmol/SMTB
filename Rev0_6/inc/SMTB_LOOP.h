/*============================================================================
 * Módulo LCD.
 * Archivo: SMTB_LCD.h
 * Versión: 1.2
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __SMTB_LOOP_H__
#define __SMTB_LOOP_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/
#define TIMEOUT 1000

#define SEM_GREEN 	ENET_RXD1
#define SEM_RED 	ENET_TXEN

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

/*=====[Prototypes (declarations) of public functions]=======================*/

void rom_test(void);
void uci_id_test(void);
void remote_uci_test(uint8_t);
int list_test(void);
void ROM_request(uint8_t);
void buzzer_test(void);
void semaphoreTest(void);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __SMTB_LOOP_H__ */

