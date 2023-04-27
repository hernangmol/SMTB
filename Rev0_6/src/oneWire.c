/*============================================================================
 * Módulo de comunicación protocolo 1 Wire.
 * Archivo: oneWire.c
 * Versión: 1.2
 * Fecha 08/10/2021
 * Autor: Hernán Gomez Molino
 *===========================================================================*/

#include "oneWire.h"
#include "OW_port.h"

/*=============================================================================
* FUNCIÓN: OWpresence
* Que hace: Chequea la presencia de sensores (esclavos) en el bus oneWire.
* PARÁMETROS:
* Que recibe: int port, int pin que usa oneWire.
* Que devuelve: -1 si no hay sensores, 0 si hay sensores.
* Variables externas que modifca: N/A
*============================================================================*/
int OWpresence(int port, int pin)
{
	// envía pulso de reset
	OWsetOut(port,pin);
	OWhigh(port,pin);
	OWdelay_uS(1000);
	OWlow(port,pin);
	OWdelay_uS(480);
	OWsetIn(port,pin);
	OWdelay_uS(40);
	// lee el bus oneWire
	if(OWread(port,pin) == TRUE)
		{
		return -1;
		}
	else
		{
		return 0;
		}
}

/*=============================================================================
* FUNCIÓN: OWcommand
* Que hace: Escribe en el bus OW la secuencia de bits de un comando, lee la
* respuesta y la deja en el buffer.
* PARÁMETROS:
* Que recibe: char cmd, comando a escribir en el bus
* 			  char * buffer, dirección del buffer para guardar respuesta
* 			  char n tamaño del buffer
* 			  int port, int pin puerto y pin del bus oneWire
* Que devuelve: N/A
* Variables externas que modifca: el buffer apuntado por el segundo parámetro
*============================================================================*/
void OWcommand(char cmd, char * buffer, char n, int port, int pin)
{
	volatile char i = 1, j;
	volatile char * p = (char *)buffer;

	OWsetOut(port,pin);
	do
	{
		// si la máscara = 1
		if(cmd & i)
		{
			OWwriteBit(OneWireStruct, 1);
		}
		// si la máscara = 0
		else
		{
			OWwriteBit(OneWireStruct, 0);
		}
		// chequea si llegó al último bit
		if(i==0x80)
		{
			break;
		}
		else
		{
			// corrimiento a izquierda de la máscara
			i <<= 1;
		}
	}while(i != 0);

	// lectura de un byte(8-> ROM, 9-> scratch)
	for(i=0; i<n; i++)
	{
		p[i] = 0;
		// lectura de a un bit
		for(j=0; j<8; j++)
		{
			OWsetOut(port,pin);
			OWlow(port,pin);
			OWdelay_uS(3);
			OWsetIn(port,pin);
			OWdelay_uS(12);
			p[i] >>= 1;
			if(OWread(port,pin)) p[i] |= 0x80;
			OWdelay_uS(55);
		}
	}
}

/*=============================================================================
* FUNCIÓN: OWreadROM
* Que hace: Lee la dirección del dispositivo oneWire (ROM).
* PARÁMETROS:
* Que recibe: char * buffer8, dirección del buffer para guardar respuesta
* 			  int port, int pin puerto y pin del bus oneWire
* Que devuelve:  -1 si no hay lectura, 0 si hay lectura.
* Variables externas que modifca: N/A
*============================================================================*/
int OWreadROM(char * buffer8, int port, int pin)
{
	int rv = -1;
	char crc = 0;
	char *p = buffer8;

	if(OWpresence(port, pin)==0)
	{
		OWdelay_uS(400);
		void disableInterrupts(void);
		// comando Read ROM
		OWcommand(0x33, p, 8, port, pin);
		void enableInterrupts(void);
		// chequea CRC
		crc = OWcrc(p, 7);
		if(crc == p[7])
		{
			rv = 0;
		}
	}
	return rv;
}

/*=============================================================================
* FUNCIÓN: OWcrc
* Que hace: Calcula el CRC para comparar con el enviado por el dispositivo.
* PARÁMETROS:
* Que recibe: char* code, puntero a la copia de la ROM o scratchpad
* 			  char n, longitud (7 para ROM, 8 para scratchpad)
* Que devuelve: el CRC calculado (char)
* Variables externas que modifca: N/A
*============================================================================*/
char OWcrc(char* code, char n)
{
	char crc=0, inbyte, i, mix;

	// recorre cada byte
	while(n--)
	{
		inbyte = *code++;
		// recorre bit a bit
		for(i=8; i; i--)
		{
			// calcula el carry
			mix= (crc ^ inbyte) & 0x01;
			// corrimiento a derecha de CRC
			crc >>= 1;
			// si carry es uno
			if(mix)
			{
				// hace XOR bitwise bits 7, 3 y 2
				crc ^= 0x8C;
			}
			// corrimiento a derecha del byte de entrada
			inbyte >>= 1;
		}
	}
	return crc;
}

/*=============================================================================
* FUNCIÓN: OWsearch
* Que hace: releva el bus en busca de dispositivos, bit a bit, una vez que
* encuentra uno lo inhibe de responder, de manera de seguir buscando hasta el
* último
* PARÁMETROS:
* Que recibe:(puntero a OWbus_t) estructura que indentifica el bus a leer.
* 			 (char) comando que indica si la busqueda es entre todos los
* 			 dispositivos o solo los que están en alarma.
* Que devuelve: 1 si se encontró un nuevo dispositivo, 0 si no se encontró.
* Variables externas que modifca: OneWireStruct->ROM[].
*============================================================================*/
char OWsearch(OWbus_t* OneWireStruct, char command)
{
	char i=1;
	char id_bit_number;
	char last_zero, rom_byte_number, search_result;
	char id_bit, cmp_id_bit;
	char rom_byte_mask, search_direction;


	/* Inicializa la busqueda */
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;


	// si el llamado anterior no fue el último
	if (!OneWireStruct->lastDeviceFlag)
	{
		// 1-Wire reset
		if (OWpresence(OneWireStruct->OWport,OneWireStruct->OWpin))
		{
			// Resetea la busqueda 
			OneWireStruct->lastDiscrepancy = 0;
			OneWireStruct->lastDeviceFlag = 0;
			OneWireStruct->lastFamilyDiscrepancy = 0;
			return 0;
		}


		// envía search command
		OWdelay_uS(100);
		do
			{
				OWwriteBit(OneWireStruct, command & i);
				if(i==0x80)
				{
					break;
				}
				else
				{
					// corrimiento a izquierda de la máscara
					i <<= 1;
				}
			}while(i != 0);
		// loop para hacer la busqueda
		do
		{
			//printf("executing Owsearch\r\n");/////////////////////////ToDo:////////si lo saco no funciona!!!!
			//printf("aabb ccddee\n");
			printf("             \n");
			// lee un bit y su complemento
			id_bit = OWreadBit(OneWireStruct);
			cmp_id_bit = OWreadBit(OneWireStruct);
			// sale si no hay dispositivos
			if ((id_bit == 1) && (cmp_id_bit == 1))
			{
				break;
			}
			else
			{
				if (id_bit != cmp_id_bit)
				{
					search_direction = id_bit;  // escribe valor de busqueda
				}
				else
				{
					if (id_bit_number < OneWireStruct->lastDiscrepancy)
					{
						search_direction = ((OneWireStruct->ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					}
					else
					{
						// si es igual a la última elige 1, sino 0
						search_direction = (id_bit_number == OneWireStruct->lastDiscrepancy);
					}
					// si 0 fue elegido, graba su posición en LastZero
					if (search_direction == 0)
					{
						last_zero = id_bit_number;
						// check ultima discrepancia en la familia 
						if (last_zero < 9)
						{
							OneWireStruct->lastFamilyDiscrepancy = last_zero;
						}
					}
				}
				// coloca el bit en ROM byte rom_byte_number
				if (search_direction == 1)
				{
					OneWireStruct->ROM_NO[rom_byte_number] |= rom_byte_mask;
				}
				else
				{
					OneWireStruct->ROM_NO[rom_byte_number] &= ~rom_byte_mask;
				}
				OWwriteBit(OneWireStruct, search_direction);
				id_bit_number++;
				rom_byte_mask <<= 1;

				// si la máscara es 0 va a un nuevo rom_byte_number y resetea la máscara.
				if (rom_byte_mask == 0)
				{
					//hace  crc8(ROM_NO[rom_byte_number]);
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}

		}
		while (rom_byte_number < 8);  // loop hasta recorrer los  ROM bytes 0-7
		// si la busqueda fue exitosa
		if (!(id_bit_number < 65))
		{
			// busqueda exitosa, entonces setea LastDiscrepancy,LastDeviceFlag,search_result
			OneWireStruct->lastDiscrepancy = last_zero;
			// chequea si es el ultimo dispositivo
			if (OneWireStruct->lastDiscrepancy == 0)
			{
				OneWireStruct->lastDeviceFlag = 1;
			}
			search_result = 1;
		}
	}
	// Si no hay dispositivo resetea el contador asi la proxima busqueda será como la primera
	if (!search_result || !OneWireStruct->ROM_NO[0])
	{
		OneWireStruct->lastDiscrepancy = 0;
		OneWireStruct->lastDeviceFlag = 0;
		OneWireStruct->lastFamilyDiscrepancy = 0;
		search_result = 0;
	}
return search_result;
}

/*=============================================================================
* FUNCIÓN: OWwriteByte
* Que hace: escribe un byte en el bus oneWire.
* PARÁMETROS:
* Que recibe:(puntero a OWbus_t) estructura que indentifica el bus a escribir.
* 			 (char) byte a escribir.
* Que devuelve: N/A.
* Variables externas que modifca: N/A
*============================================================================*/
void OWwriteByte(OWbus_t* OneWireStruct, char byte)
{
	char i = 8;
	/* Escribe 8 bits */
	while (i--) {
		/* LSB primero */
		OWwriteBit(OneWireStruct, byte & 0x01);
		byte >>= 1;
	}
}

/*=============================================================================
* FUNCIÓN: OWreadByte
* Que hace: lee un byte del bus oneWire.
* PARÁMETROS:
* Que recibe:(puntero a OWbus_t) estructura que indentifica el bus a leer.
* Que devuelve: (char) byte leido.
* Variables externas que modifca: N/A
*============================================================================*/
char OWreadByte(OWbus_t* OneWireStruct)
{
	char i = 8, byte = 0;
	while (i--)
	{
		byte >>= 1;
		byte |= (OWreadBit(OneWireStruct) << 7);
	}
	return byte;
}

/*=============================================================================
* FUNCIÓN: OWresetSearch
* Que hace: Inicializa los parámetros de busqueda (OWsearch).
* PARÁMETROS:
* Que recibe:(puntero a OWbus_t) estructura que indentifica el bus a leer.
* Que devuelve: N/A
* Variables externas que modifca: N/A
*============================================================================*/
void OWresetSearch(OWbus_t* OneWireStruct)
{
	OneWireStruct->lastDiscrepancy = 0;
	OneWireStruct->lastDeviceFlag = 0;
	OneWireStruct->lastFamilyDiscrepancy = 0;
}

