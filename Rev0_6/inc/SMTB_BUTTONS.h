/*============================================================================
 * Módulo de botones.
 * Archivo: SMTB_BUTTONS.h
 * Versión: 1.2
 * Fecha 25/06/2022
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

#ifndef SMTB_BUTTONS_H_
#define SMTB_BUTTONS_H_

#define WINDOW_HEIGHT 4

char menu[7][20];
int windowStart;
int itemSelected;

void buttonsTest(void);
void buttonsInit(void);
int buttonsUpdate(void);

#endif /* SMTB_BUTTONS_H_ */
