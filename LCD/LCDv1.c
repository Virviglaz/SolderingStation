/* VERSION 2.00 */
/*  20.09.2012  */

#include "LCDv1.h"
#include "strings.h"

#define LCD_RS_PIN 		GPIO_PIN_7
#define LCD_E_PIN  		GPIO_PIN_6
#define LCD_D4_PIN 		GPIO_PIN_5
#define LCD_D5_PIN 		GPIO_PIN_4
#define LCD_D6_PIN 		GPIO_PIN_3
#define LCD_D7_PIN 		GPIO_PIN_2

#define LCD_RS_GPIO 	GPIOD
#define LCD_E_GPIO  	GPIOD
#define LCD_D4_GPIO 	GPIOD
#define LCD_D5_GPIO 	GPIOD
#define LCD_D6_GPIO 	GPIOD
#define LCD_D7_GPIO 	GPIOD

void LCD_WriteData(char data);
void LCD_Write(char CMD);

void LCD_init(void)
{
	PIN_OUT_PP(LCD_RS_GPIO, LCD_RS_PIN);
	PIN_OUT_PP(LCD_E_GPIO,  LCD_E_PIN);
	PIN_OUT_PP(LCD_D4_GPIO, LCD_D4_PIN);
	PIN_OUT_PP(LCD_D5_GPIO, LCD_D5_PIN);
	PIN_OUT_PP(LCD_D6_GPIO, LCD_D6_PIN);
	PIN_OUT_PP(LCD_D7_GPIO, LCD_D7_PIN);

	PIN_ON(LCD_RS_GPIO, LCD_RS_PIN);
	PIN_ON(LCD_E_GPIO,  LCD_E_PIN);
	PIN_ON(LCD_D4_GPIO, LCD_D4_PIN);
	PIN_ON(LCD_D5_GPIO, LCD_D5_PIN);
	PIN_ON(LCD_D6_GPIO, LCD_D6_PIN);
	PIN_ON(LCD_D7_GPIO, LCD_D7_PIN);
	delay_us(5);
	PIN_OFF(LCD_RS_GPIO, LCD_RS_PIN);
	PIN_OFF(LCD_E_GPIO,  LCD_E_PIN);
	PIN_OFF(LCD_D4_GPIO, LCD_D4_PIN);
	PIN_OFF(LCD_D5_GPIO, LCD_D5_PIN);
	PIN_OFF(LCD_D6_GPIO, LCD_D6_PIN);
	PIN_OFF(LCD_D7_GPIO, LCD_D7_PIN);
	
	delay_ms(17);
	LCD_Write(0x30);
	delay_ms(1);
	LCD_Write(0x30);
	delay_ms(1);
	LCD_Write(0x30);
	delay_ms(1);
	LCD_Write(0x20);  //4 bit wide bus
	  	
	LCD_Send_Cmd(0x28);	   //4bit wide bus 2 rows
	LCD_Send_Cmd(0x10);
	LCD_Send_Cmd(0x01);
	LCD_Send_Cmd(0x0F);

	LCD_Send_Cmd(LCD_CMD_Cursor_Off);
	LCD_Send_Cmd(LCD_CMD_Clear);
	delay_ms(10);
}

void LCD_Write(char CMD) 
{	 
	 if ((CMD&(1<<4))!=0) {PIN_ON(LCD_D4_GPIO,LCD_D4_PIN);} 
	 if ((CMD&(1<<5))!=0) {PIN_ON(LCD_D5_GPIO,LCD_D5_PIN);} 
	 if ((CMD&(1<<6))!=0) {PIN_ON(LCD_D6_GPIO,LCD_D6_PIN);} 
	 if ((CMD&(1<<7))!=0) {PIN_ON(LCD_D7_GPIO,LCD_D7_PIN);} 

	 PIN_ON(LCD_E_GPIO,LCD_E_PIN);
	 delay_us(5);
	 PIN_OFF(LCD_E_GPIO,LCD_E_PIN);
	 delay_us(5);

	PIN_OFF(LCD_D4_GPIO,LCD_D4_PIN);
	PIN_OFF(LCD_D5_GPIO,LCD_D5_PIN);
	PIN_OFF(LCD_D6_GPIO,LCD_D6_PIN);
	PIN_OFF(LCD_D7_GPIO,LCD_D7_PIN);
}
void LCD_Send_Cmd(char CMD)
{
	delay_ms(5);
	PIN_OFF(LCD_RS_GPIO,LCD_RS_PIN);
	LCD_Write((CMD&0xF0));
	LCD_Write(((CMD&0x0F)<<4));
}

void LCD_SetPosition(char row, char colum)
{
	u8 position = 0x80;
	if (row > 0)  position |= 0x40;
	LCD_Send_Cmd(position | colum);
}

void LCD_WriteData(char data)
{
	delay_us(50);
	PIN_ON(LCD_RS_GPIO,LCD_RS_PIN);
	LCD_Write((data&0xF0));
	LCD_Write(((data&0x0F)<<4));
}

void LCD_PrintChar(char data)
{
	if (data == '\n') LCD_Send_Cmd(0x80 | 0x40); else LCD_WriteData(data);
}

void LCD_PrintString(char * string)
{
	while (*string) LCD_PrintChar(*string++);
}

void LCD_PrintStringLen(char * string, unsigned char Len)
{
	while (Len--) LCD_PrintChar(*string++);
}

void LCD_PrintHEX_Byte (unsigned char HexValue)
{
	char String[5]="0x";
	ClearString(String+2,3,0);
	ValueToStringHEX_Byte(HexValue, String+2);
	LCD_PrintString(String);
}

void LCD_PrintValue_uChar (unsigned char Value)
{
	char String[4];
	ClearString(String,4,0);
	uCharToStr(Value, String);
	LCD_PrintString(String);
}

void LCD_PrintValue_uLong (unsigned long Value)
{
	char String[11];
	ClearString(String,11,0);
	uLongToStr(Value, String);
	LCD_PrintString(String);
}

void LCD_PrintValue_sChar (signed char Value)
{
	char String[5];
	ClearString(String,5,0);
	sCharToStr(Value, String);
	LCD_PrintString(String);
}

void LCD_PrintValue_sLong (signed long Value)
{
	char String[12];
	ClearString(String,12,0);
	sLongToStr(Value, String);
	LCD_PrintString(String);
}
