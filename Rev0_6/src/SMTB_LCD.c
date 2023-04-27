/*=============================================================================
 * Program: LCD_test
 * Date: 2021/08/24
 *===========================================================================*/

/*==================[inclusiones]============================================*/

#include "SMTB_LCD.h"
#include "sapi.h"
#include "SUCI.h"
#include "DS18B20.h"
#include "SMTB_LEDMAT.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "string.h"

#define OFFSET_LCD 0x30

extern uint8_t systemMode;
extern uint8_t flagAlarm, flagDegraded, quantAlarm, quantActive;
extern TimerHandle_t normalTimer;
extern int ROM_active;

const char SupIzq[8] =
{
	 0b00111,
	 0b01111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111
};

const char SupDer[8] =
{
	 0b11100,
	 0b11110,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111
};

const char Techo[8] =
{
	 0b11111,
	 0b11111,
	 0b11111,
	 0b00000,
	 0b00000,
	 0b00000,
	 0b00000,
	 0b00000
};

const char Entrepiso[8] =
{
	 0b11111,
	 0b11111,
	 0b11111,
	 0b00000,
	 0b00000,
	 0b00000,
	 0b11111,
	 0b11111
};

const char Piso[8] =
{
	 0b00000,
	 0b00000,
	 0b00000,
	 0b00000,
	 0b00000,
	 0b11111,
	 0b11111,
	 0b11111
};

const char InfIzq[8] =
{
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b01111,
	 0b00111
};

const char InfDer[8] =
{
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11110,
	 0b11100
};

const char Bloque[8] =
{
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111,
	 0b11111
};

enum{
   SI	 = 0,
   Te 	 = 1,
   SD 	 = 2,
   II    = 3,
   Pi	 = 4,
   ID 	 = 5,
   EP 	 = 6,
   Bl    = 7,
};

//extern struct OPTION
//{
//	int item;
//	int selected; //cambiar a bool_t?
//	char name[10];
//} menu[8];

extern char menu[7][20];
extern int windowStart;
extern int itemSelected;

bool_t printDig(uint8_t , uint8_t );

int SMTB_LCD_Init( void ){

   // ---------- CONFIGURACIONES ------------------------------

   // Inicializar y configurar la plataforma

   //i2cInit( I2C0, 100000 ); // A prueba//////////////////////////////////////

   delay( LCD_STARTUP_WAIT_MS );   // Wait for stable power (some LCD need that)

   // Inicializar LCD de 16x2 (caracteres x lineas) con cada caracter de 5x8 pixeles
   lcdInit( 20, 4, 5, 8 );
   //printf("col: %d , fil: %d",lcd.x, lcd.y );
   //debe descomentarse #define LCD_HD44780_I2C_PCF8574T en display_lcd_hd44780_gpios.h
   lcdCreateChar( SI, SupIzq );
   lcdCreateChar( Te, Techo );
   lcdCreateChar( SD, SupDer );
   lcdCreateChar( II, InfIzq);
   lcdCreateChar( Pi, Piso );
   lcdCreateChar( ID, InfDer );
   lcdCreateChar( EP, Entrepiso);
   lcdCreateChar( Bl, Bloque );

   lcdCursorSet( LCD_CURSOR_OFF ); // Apaga el cursor
   //lcdGoToXY( 0, 0 ); ///////////////////////////////////////////////////////////////////////
   lcdClear();                     // Borrar la pantalla
//   lcdSendStringRaw( "SMTB_Init" );
   //delay (2000);
   return 0;
}

 bool_t printDig(uint8_t dig, uint8_t pos)
 {
	 uint8_t fil;
	 uint8_t col;
	 if(pos==1)
	 {
		 fil = 0;
		 col = 1;
		 lcdGoToXY( col, fil );
	 }
	 else if (pos==2)
	 {
		 fil = 0;
		 col = 4;
		 lcdGoToXY( col, fil );
	 }
	 else return(FALSE);
	 switch(dig)
	 {
	 	 case 0:
	 	 lcdData(SI);
	 	 lcdData(Te);
	 	 lcdData(SD);
	 	 fil ++;
	 	 lcdGoToXY( col, fil );
	 	 lcdData(II);
	 	 lcdData(Pi);
	 	 lcdData(ID);
	 	 break;

	 	 case 1:
		 lcdSendStringRaw( " " );
		 lcdData(SI);
		 lcdSendStringRaw( " " );
		 fil ++;
		 lcdGoToXY( col, fil );
		 lcdSendStringRaw( " " );
		 lcdData(ID);
		 lcdSendStringRaw( " " );
		 break;


	 	 case 2:
	 	 lcdData(EP);
	 	 lcdData(EP);
	 	 lcdData(SD);
	 	 fil ++;
	 	 lcdGoToXY( col, fil );
	 	 lcdData(II);
	 	 lcdData(Pi);
	 	 lcdData(Pi);
	 	 break;

	 	 case 3:
	 	 lcdData(EP);
	 	 lcdData(EP);
	 	 lcdData(SD);
	 	 fil ++;
	 	 lcdGoToXY( col, fil );
	 	 lcdData(Pi);
	 	 lcdData(Pi);
	 	 lcdData(ID);
	 	 break;

	 	 case 4:
	 	 lcdData(II);
	 	 lcdData(Pi);
	 	 lcdData(Bl);
	 	 fil ++;
	 	 lcdGoToXY( col, fil );
	 	 lcdSendStringRaw( " " );
	 	 lcdSendStringRaw( " " );
	 	 lcdData(Bl);
	 	 break;

	 	 case 5:
	 	 lcdData(Bl);
	 	 lcdData(EP);
	 	 lcdData(EP);
	 	 fil ++;
	 	 lcdGoToXY( col, fil );
	 	 lcdData(Pi);
	 	 lcdData(Pi);
	 	 lcdData(ID);
	 	 break;

	 	 case 6:
	 	 lcdData(SI);
	 	 lcdData(EP);
	 	 lcdData(EP);
	 	 fil ++;
	 	 lcdGoToXY( col, fil );
	 	 lcdData(II);
	 	 lcdData(Pi);
	 	 lcdData(ID);
	 	 break;

	 	 case 7:
	  	 lcdData(Te);
	  	 lcdData(Te);
	  	 lcdData(Bl);
	  	 fil ++;
	  	 lcdGoToXY( col, fil );
	  	 lcdSendStringRaw( " " );
	  	 lcdSendStringRaw( " " );
	  	 lcdData(ID);
	  	 break;

	 	 case 8:
	 	 lcdData(SI);
	 	 lcdData(EP);
	 	 lcdData(SD);
	 	 fil ++;
	 	 lcdGoToXY( col, fil );
	 	 lcdData(II);
	 	 lcdData(Pi);
	 	 lcdData(ID);
	 	 break;

	     case 9:
	 	 lcdData(SI);
	 	 lcdData(EP);
	 	 lcdData(SD);
	 	 fil ++;
	 	 lcdGoToXY( col, fil );
	 	 lcdSendStringRaw( " " );
	 	 lcdSendStringRaw( " " );
	 	 lcdData(ID);
	 	 break;
	 }
 }

 void SMTB_Temp_Show(int tempToDisp, uint8_t eje, uint8_t sensor, uint8_t status)
{
	uint8_t dig1;
	uint8_t dig2;

// saturación del dato máximo
	if(tempToDisp > 125)
	{
		tempToDisp = 125;
	}
// imprime guiones si el sensor está inactivo
	if(status == INACTIVE)
	{
		lcdGoToXY( 1, 0 );
		lcdData(Pi);
		lcdData(Pi);
		lcdGoToXY( 4, 0 );
		lcdData(Pi);
		lcdData(Pi);
	}
	else
	{
//impresión de la centena
		if(tempToDisp > 100)
		{
			lcdGoToXY( 0, 0 );
			lcdData(SD);
			lcdGoToXY( 0, 1 );
			lcdData(ID);
			tempToDisp -= 100;
		}
		dig1 = tempToDisp / 10;
		dig2 = tempToDisp % 10;
		printDig(dig1, 1);
		printDig(dig2, 2);
	}
	lcdGoToXY( 7, 0 );
	lcdData(0x6f); // imprime el cerito
	lcdGoToXY( 1, 2 );
	lcdSendStringRaw( "Eje: " );
	lcdData(0x30 + eje);
	lcdGoToXY( 1, 3 );
	lcdSendStringRaw( "Sen: " );
	lcdData(0x30 + sensor);
}

void SMTB_Mode_Show(void)
{
	lcdGoToXY( 9, 0 );
	lcdSendStringRaw( "UCI #" );
	if(getStatus(BIT_UCI_1))
	{
		lcdData(0x31);
		lcdSendStringRaw( " " );
		if(getStatus(BIT_REMOTE))
		{
			lcdData(0x7E); //Imprime la flechita
			lcdSendStringRaw( " #2" );
		}
	}
	if(getStatus(BIT_UCI_2))
	{
		lcdData(0x32);
		lcdSendStringRaw( " " );
		if(getStatus(BIT_REMOTE))
		{
			lcdData(0x7F); //Imprime la flechita
			lcdSendStringRaw( " #1" );
		}
	}
	lcdGoToXY( 9, 1 );
	lcdSendStringRaw( "MOD:");
	if (flagAlarm != 0)
		lcdSendStringRaw( " ALARMA");
	else if (flagDegraded != 0)
		lcdSendStringRaw( "  FALLA");
	else
		lcdSendStringRaw( " NORMAL");
	lcdGoToXY( 9, 2 );
	lcdSendStringRaw( "ACT:  ");
	if(quantActive >= 30)
	{
		lcdData(0x33);
		quantActive -= 30;
	}
	else if(quantActive >= 20)
	{
		lcdData(0x32);
		quantActive -= 20;
	}
	else if(quantActive >= 10)
	{
		lcdData(0x31);
		quantActive -= 10;
	}
	else
		lcdSendStringRaw( " ");
	lcdData(0x30 + quantActive);
	lcdSendStringRaw( "/36");

	/*Cantidad de sensores en alarma*/
	lcdGoToXY( 9, 3 );
	lcdSendStringRaw( "ALRM: ");
	if(quantAlarm >= 30)
	{
		lcdData(0x33);
		quantAlarm -= 30;
	}
	else if(quantAlarm >= 20)
	{
		lcdData(0x32);
		quantAlarm -= 20;
	}
	else if(quantAlarm >= 10)
		{
			lcdData(0x31);
			quantAlarm -= 10;
		}
	else
		lcdSendStringRaw( " ");
	lcdData(0x30 + quantAlarm);
	lcdSendStringRaw( "/36");
}

void SMTB_Menu_Show(void)
{
	lcdClear();
	if (itemSelected == windowStart)
	{
		lcdGoToXY( 0, 0 );
		lcdSendStringRaw( "-> " );
	}
	lcdSendStringRaw( menu + windowStart);
	lcdGoToXY( 0, 1 );
	if (itemSelected == windowStart+1)
		 lcdSendStringRaw( "-> " );
	lcdSendStringRaw( menu + windowStart+1 );
	lcdGoToXY( 0, 2 );
	if (itemSelected == windowStart+2)
		lcdSendStringRaw( "-> " );
	lcdSendStringRaw( menu + windowStart+2 );
	lcdGoToXY( 0, 3 );
	if (itemSelected == windowStart+3)
		lcdSendStringRaw( "-> " );
	lcdSendStringRaw( menu + windowStart+3 );
}

void SMTB_Alarm_View_Init(void)
{
	uint8_t data[6] = {0};
//Actualización del LCD
	lcdClear();
	lcdGoToXY( 0, 0 );
	lcdSendStringRaw( "Visualizando: " );
	lcdGoToXY( 0, 1 );
	lcdSendStringRaw( "Sensores en alarma" );
	lcdGoToXY( 0, 2 );
	lcdSendStringRaw( "presione una tecla" );
	lcdGoToXY( 0, 3 );
	lcdSendStringRaw( "para SALIR" );

//Actualización de la matriz
	for (int k=0; k<N; k++)
	{
		if(sensor[k].estado == SEN_ALARM)
		{
			data[sensor[k].eje] |= 0x80>>(sensor[k].posicion);
		}
	}
	for (uint8_t i=0; i<6; i++)
	{
		writeRow(i, data[i]);
	}
}

void SMTB_Alta_Init(void)
{
	lcdClear();
	lcdGoToXY( 0, 0 );
	lcdSendStringRaw( "Alta de sensor: " );
	lcdGoToXY( 0, 1 );
	lcdSendStringRaw( "Conecte el sensor en" );
	lcdGoToXY( 0, 2 );
	lcdSendStringRaw( "el frente de la UCI" );
	lcdGoToXY( 0, 3 );
	lcdSendStringRaw( "y presione ENTER" );
}

void SMTB_Sens_View_Init(void)
{
	uint8_t data[6] = {0};
//Actualización del LCD
	lcdClear();
	lcdGoToXY( 0, 0 );
	lcdSendStringRaw( "Visualizando: " );
	lcdGoToXY( 0, 1 );
	lcdSendStringRaw( "Sensores inactivos" );
	lcdGoToXY( 0, 2 );
	lcdSendStringRaw( "presione una tecla" );
	lcdGoToXY( 0, 3 );
	lcdSendStringRaw( "para SALIR" );

//Actualización de la matriz
	for (int k=0; k<N; k++)
	{
		if(sensor[k].estado == INACTIVE)
		{
			data[sensor[k].eje] |= 0x80>>(sensor[k].posicion);
		}
	}
	for (uint8_t i=0; i<6; i++)
	{
		writeRow(i, data[i]);
	}
}

void SMTB_ROM_consist(void)
{
	lcdClear();
	lcdGoToXY( 0, 0);
	lcdSendStringRaw( "Chequeando" );
	lcdGoToXY( 0, 1);
	lcdSendStringRaw( "consistencia ROM" );
}

void SMTB_ROM_consist_OK(void)
{
	lcdGoToXY( 9, 3);
	lcdSendStringRaw( "OK" );
}

void SMTB_ROM_consist_NOK(void)
{
	lcdGoToXY( 10, 3);
	lcdSendStringRaw( "no OK" );
}

void SMTB_ROM_consist_update(void)
{
	ROM_active ++;
	lcdGoToXY( 3, 3);
	if(ROM_active >= 30)
		{
			lcdData(0x33);
			ROM_active -= 30;
		}
		else if(ROM_active >= 20)
		{
			lcdData(0x32);
			ROM_active -= 20;
		}
		else if(ROM_active >= 10)
		{
			lcdData(0x31);
			ROM_active -= 10;
		}
		else
			lcdSendStringRaw( " ");
		lcdData(0x30 + ROM_active);
		lcdSendStringRaw( "/36");
}

void SMTB_ROM_copy_NOK(void)
{
	lcdGoToXY( 6, 3);
	lcdSendStringRaw( "no OK" );
}

void SMTB_ROM_copying(void)
{
	lcdClear();
	lcdGoToXY( 0, 0);
	lcdSendStringRaw( "Copiando :" );
	lcdGoToXY( 0, 1);
	lcdSendStringRaw( " " );
}

void SMTB_ROM_constCheckNotRun(void) // ToDo: estilo de nombre no compatible
{
	lcdGoToXY( 1, 3);
	lcdSendStringRaw( "Const no chequeada" );
}

void SMTB_ROM_constOk(void) // ToDo: estilo de nombre no compatible
{
	lcdGoToXY( 1, 3);
	lcdSendStringRaw( "ROMS consistentes" );
}

void SMTB_pending(void)
{
	lcdClear();
	lcdGoToXY( 0, 0);
	lcdSendStringRaw( "Funcion pendiente" );
}

void SMTB_Status_Show(void)
{
	char statusList[7][20];
//Armado del mensaje
	strcpy(menu[0], "ESTADOS");								// Titulo
	strcpy(menu[1], "UCI LOCAL:       ");					// UCI Local
	if(getStatus(BIT_UCI_1) && !getStatus(BIT_UCI_2))
		strcat(menu[0],"#1");
	else if(!getStatus(BIT_UCI_1) && getStatus(BIT_UCI_2))
		strcat(menu[1],"#2");
	else
		strcat(menu[1], " ERROR");
	strcpy(menu[2], "UCI REMOTA:      ");					//UCI Remota
	if(getStatus(BIT_REMOTE))
		strcat(menu[2],"Ok");
	else
		strcat(menu[2],"NO");
	strcpy(menu[3], "BUS SENSORES:    ");					//Bus de sensores
	if(getStatus(BIT_BUS_1))
			strcat(menu[3],"Ok");
		else
			strcat(menu[3],"NO");
	strcpy(menu[4], "BUS FRONTAL:     ");					//Bus frontal
	if(getStatus(BIT_BUS_2))
			strcat(menu[4],"Ok");
		else
			strcat(menu[4],"NO");
	strcpy(menu[5], " ");
	strcpy(menu[6], " ");

	windowStart = 1;
	itemSelected = 1;

	lcdClear();
	lcdGoToXY( 0, 0);
	lcdSendStringRaw( menu + windowStart);
	lcdGoToXY( 0, 1);
	lcdSendStringRaw( menu + windowStart+1);
	lcdGoToXY( 0, 2);
	lcdSendStringRaw( menu + windowStart+2);
	lcdGoToXY( 0, 3);
	lcdSendStringRaw( menu + windowStart+3);
}

