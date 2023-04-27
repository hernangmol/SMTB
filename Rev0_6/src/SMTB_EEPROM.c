/*============================================================================
 * Módulo para prueba de driver para sensor DS18B20.
 * Archivo: PCSE_TP.c
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

/*=====[Inclusión de dependencias]===========================================*/

#include "SMTB_EEPROM.h"
#include "board.h"
#include "eeprom_18xx_43xx.h"
#include "sapi.h"


#define EEPROM_CLUSTER 0

int Board_EEPROM_loadSensor(int posicion, tempSens_t* p_buffer)
{
	for(int i=8; i>0; i-- )
		{
		//p_buffer->ROM_NO[8-i] = Board_EEPROM_readByte((posicion-1)*12 + i-1);///////simplificar
		p_buffer->ROM_NO[8-i] = Board_EEPROM_readByte((posicion)*12 + i-1);///////simplificar
		}
	// ToDo: return!!!!!!!!!!!!!
}

int Board_EEPROM_recSensor(int posicion, tempSens_t sensor)
{
	uint32_t aux = 0;
	int addr = (EEPROM_CLUSTER*512) + (posicion)*12;
	if(addr>=(16*1024))
		return -1;
	int addr4 = addr/4;
	int pageAddr = addr4/EEPROM_PAGE_SIZE;
	int pageOffset = addr4 - pageAddr*EEPROM_PAGE_SIZE;
	uint32_t *pEepromMem = (uint32_t*)EEPROM_ADDRESS(pageAddr,pageOffset*4);
	for(int i=0; i<4; i++)
	{
		printf("aux = %X\r\n", aux);
		aux = aux + sensor.ROM_NO[i];
		if(i < 3)
			aux *= 256;
	}
	printf("aux = %X\r\n", aux);
	pEepromMem[1] = aux;
	Chip_EEPROM_WaitForIntStatus(LPC_EEPROM, EEPROM_INT_ENDOFPROG);
	aux = 0;
	for(int i=0; i<4; i++)
		{
			printf("aux = %X\r\n", aux);
			aux = aux + sensor.ROM_NO[4 + i];
			if(i < 3)
				aux *= 256;
		}
		printf("aux = %X\r\n", aux);
		pEepromMem[0] = aux;
		Chip_EEPROM_WaitForIntStatus(LPC_EEPROM, EEPROM_INT_ENDOFPROG);

	return 0;
}

int32_t Board_EEPROM_writeByte(uint32_t addr,uint8_t value)
{
	//ToDo: agregar manejo de cluster activo

	if(addr>=(16*1024))
		return -1;

	uint32_t addr4 = addr/4;
	uint32_t pageAddr = addr4/EEPROM_PAGE_SIZE;
	uint32_t pageOffset = addr4 - pageAddr*EEPROM_PAGE_SIZE;

	uint32_t *pEepromMem = (uint32_t*)EEPROM_ADDRESS(pageAddr,pageOffset*4);

	// read 4 bytes in auxValue
	uint32_t auxValue = pEepromMem[0];
	uint8_t* pAuxValue = (uint8_t*)&auxValue;

	// modify auxValue with new Byte value
	uint32_t indexInBlock = addr % 4;
	pAuxValue[indexInBlock] = value;

	//write auxValue back in eeprom
	pEepromMem[0] = auxValue;
	Chip_EEPROM_WaitForIntStatus(LPC_EEPROM, EEPROM_INT_ENDOFPROG);
	return 0;
}

int32_t Board_EEPROM_readByte(uint32_t addr)
{
	//ToDo: agregar manejo de cluster activo

	if(addr>=(16*1024))
        return -1;

	uint32_t addr4 = addr/4;
	uint32_t pageAddr = addr4/EEPROM_PAGE_SIZE;
	uint32_t pageOffset = addr4 - pageAddr*EEPROM_PAGE_SIZE;

	uint32_t *pEepromMem = (uint32_t*)EEPROM_ADDRESS(pageAddr,pageOffset*4);

	// read 4 bytes in auxValue
	uint32_t auxValue = pEepromMem[0];
	uint8_t* pAuxValue = (uint8_t*)&auxValue;

	// modify auxValue with new Byte value
	uint32_t indexInBlock = addr % 4;
	return (int32_t) pAuxValue[indexInBlock];

}

void Board_EEPROM_init(void)
{
	Chip_EEPROM_Init(LPC_EEPROM);
  	/* Set Auto Programming mode */
 	Chip_EEPROM_SetAutoProg(LPC_EEPROM,EEPROM_AUTOPROG_AFT_1WORDWRITTEN);
}
