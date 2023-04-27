/*============================================================================
 * Módulo LCD.
 * Archivo: SMTB_LCD.h
 * Versión: 1.2
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __LCD_TEST_H__
#define __LCD_TEST_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/

/*=====[Prototypes (declarations) of public functions]=======================*/

int SMTB_LCD_Init(void);
void SMTB_Temp_Show(int,uint8_t,uint8_t,uint8_t);
void SMTB_Mode_Show(void);
void SMTB_Menu_Show(void);
void SMTB_Status_Show(void);
void SMTB_Alarm_View_Init(void);
void SMTB_Alta_Init(void);
//	void SMTB_Alta_Sensor(void); // en prueba de eliminación 20230412
void SMTB_Sens_View_Init(void);
void SMTB_ROM_consist(void);
void SMTB_ROM_copy(void);
void SMTB_ROM_consist_OK(void);
void SMTB_ROM_consist_NOK(void);
void SMTB_ROM_consist_update(void);
void SMTB_ROM_copying(void);
void SMTB_ROM_constCheckNotRun(void);
void SMTB_ROM_constOk(void);

void SMTB_pending(void);

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __LCD_TEST_H__ */
