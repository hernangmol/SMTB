/*
 * SMTB_EEPROM.h
 *
 *  Created on: 18 feb. 2022
 *      Author: hernan
 */

#ifndef SMTB_INT_V3_INC_SMTB_EEPROM_H_
#define SMTB_INT_V3_INC_SMTB_EEPROM_H_

#include <stdint.h>
#include <stddef.h>
#include "DS18B20.h"

void Board_EEPROM_init(void);
int32_t Board_EEPROM_readByte(uint32_t addr);
int32_t Board_EEPROM_writeByte(uint32_t addr,uint8_t value);
int Board_EEPROM_recSensor(int posicion, tempSens_t sensor);
int Board_EEPROM_loadSensor(int posicion, tempSens_t* buffer);

#endif /* SMTB_INT_V3_INC_SMTB_EEPROM_H_ */
