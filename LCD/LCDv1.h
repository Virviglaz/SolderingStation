#include "STM8_GPIO.h"
#include "STM8_Delays.h"

#define LCD_CMD_Cursor_Off			   	0x0C
#define LCD_CMD_Blink_Cursor_On		   	        0x0F
#define LCD_CMD_Underline_Cursor_On		        0x0E
#define LCD_CMD_First_Row				0x80
#define LCD_CMD_Second_Row				0xC0
#define LCD_CMD_Third_Row				0x94
#define LCD_CMD_Fourth_Row				0xD4
#define LCD_CMD_Clear					0x01
#define LCD_CMD_Return_Home				0x02
#define LCD_CMD_Move_Cursor_Left		        0x10
#define LCD_CMD_Move_Cursor_Right		        0x14
#define LCD_CMD_Turn_On					0x0C
#define LCD_CMD_Turn_Off				0x08
#define LCD_CMD_Shift_Left				0x18
#define LCD_CMD_Shift_Right				0x1C	

void LCD_init(void);
void LCD_Send_Cmd(char CMD);
void LCD_SetPosition(char row, char colum);
void LCD_PrintChar(char data);
void LCD_PrintString(char * string);
void LCD_PrintStringLen(char * string, unsigned char Len);
void LCD_PrintHEX_Byte (unsigned char HexValue);
void LCD_PrintValue_uChar (unsigned char Value);
void LCD_PrintValue_uLong (unsigned long Value);
void LCD_PrintValue_sChar (signed char Value);
void LCD_PrintValue_sLong (signed long Value);
