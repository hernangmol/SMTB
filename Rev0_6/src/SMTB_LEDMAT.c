/*=============================================================================
 * Program:SMTB_LEDMAT
 * Date: 2021/09/07
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "SMTB_LEDMAT.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/

#define SPI_GPIO 		 GPIO1
//#define T_STEP 			 100
#define REG_TEST		 0x0F
#define REG_SCANLIMIT	 0x0B
#define REG_INTENSITY	 0x0A
#define REG_DECODE		 0x09
#define REG_SHUTDOWN	 0x0C

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

//static uint8_t data;
//static uint8_t reg;

void writeRow(uint8_t regX, uint8_t dataX )
{
	uint8_t message[2];

	message[0] = regX;
	message[1] = dataX;
	gpioWrite(SPI_GPIO, OFF);
	spiWrite(SPI0, message, 2);
	gpioWrite(SPI_GPIO, ON);
	//delay(T_STEP);
}

void ledMatrixInit(void)
{
	gpioInit( SPI_GPIO, GPIO_OUTPUT );
	gpioWrite(SPI_GPIO, ON);
//	 control(TEST, OFF);
//	reg = REG_TEST;
//	data = 0x00;	// no test
	writeRow(REG_TEST, 0x00);
////	  control(SCANLIMIT, ROW_SIZE-1);       // scan limit is set to max on startup
//	reg = REG_SCANLIMIT;
//	data = 0x07;
	writeRow(REG_SCANLIMIT, 0x07);
////	  control(INTENSITY, MAX_INTENSITY/2);  // set intensity to a reasonable value
//	reg = REG_INTENSITY;
//	data = 0x04;
	writeRow(REG_INTENSITY,0x0F);
////	  control(DECODE, OFF);                 // ensure no decoding (warm boot potential issue)
//	reg = REG_DECODE;
//	data = 0x00;
	writeRow(REG_DECODE,0x00);
////	  control(SHUTDOWN, OFF);               // take the modules out of shutdown mode
//	reg = REG_SHUTDOWN;
//	data = 0x01;
	writeRow(REG_SHUTDOWN,0x01);
//// test ON
//	reg = REG_TEST;
//	data = 0x01;
	writeRow(REG_TEST,0x01);
	delay(200);
//// test OFF
//	reg = REG_TEST;
//	data = 0x00;
	writeRow(REG_TEST,0x00);
}

void ledMatrixClear(void)
{
	for(int i=0;i<7;i++)
	{
//		reg = i;
//		data = 0;
		writeRow(i,0);
		//delay(10);
	}
}

void ledMatrixTest(void)
{
	int data;
	for(int i=1;i<7;i++)
	{
//		reg = i;
		data = 128;
		for(int j=0;j<7;j++)
		{
			data>>=1;
			writeRow(i, data);
			delay(100);
		}
	}
	//vTaskDelay(200);
	delay(200);
	ledMatrixClear();
	data=0b00011000;
	writeRow(3, data);
	writeRow(4, data);
	//vTaskDelay(200);
	delay(200);
	ledMatrixClear();
	data=0b00111100;
	writeRow(2, data);
	writeRow(5, data);
	data=0b00100100;
	writeRow(3, data);
	writeRow(4, data);
	//vTaskDelay(200);
	delay(200);
	ledMatrixClear();
	data=0b01111110;
	writeRow(1, data);
	writeRow(6, data);
	data=0b01000010;
	writeRow(2, data);
	writeRow(5, data);
	data=0b01000010;
	writeRow(3, data);
	writeRow(4, data);
	//vTaskDelay(200);
	delay(200);
	ledMatrixClear();
	data=0b00111100;
	writeRow(2, data);
	writeRow(5, data);
	data=0b00100100;
	writeRow(3, data);
	writeRow(4, data);
	//vTaskDelay(200);
	delay(200);
	ledMatrixClear();
	data=0b00011000;
	writeRow(3, data);
	writeRow(4, data);
	//vTaskDelay(200);
	delay(200);
	ledMatrixClear();
	return;
}

