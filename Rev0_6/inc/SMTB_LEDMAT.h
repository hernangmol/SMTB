/*
 * SMTB_LEDMAT.h
 *
 *  Created on: 15 feb. 2022
 *      Author: hernan
 */

#ifndef SMTB_INT_V2_INC_SMTB_LEDMAT_H_
#define SMTB_INT_V2_INC_SMTB_LEDMAT_H_

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>

/*=====[Prototypes (declarations) of public functions]=======================*/

void writeRow(uint8_t, uint8_t);
void ledMatrixInit(void);
void ledMatrixClear(void);
void ledMatrixTest(void);

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/

/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/


/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/


#endif /* SMTB_INT_V2_INC_SMTB_LEDMAT_H_ */
