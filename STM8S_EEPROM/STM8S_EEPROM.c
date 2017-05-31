#include "STM8S_EEPROM.h"
#include "STM8_Delays.h"
void STM8S_EEPROM_Init (void)
{
  FLASH_Unlock(FLASH_MEMTYPE_DATA);
  FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);
}

void STM8S_EEPROM_WriteString (u16 address, u8 * String)
{
  FLASH_ProgramBlock(0, FLASH_MEMTYPE_DATA, FLASH_PROGRAMMODE_STANDARD, String);
}

void STM8S_EEPROM_Read_String (u8 * String, u16 Address, u8 Len)
{
  while (Len--) * String++ = FLASH_ReadByte (Address++);
}

u8 GenerateUniqueID8 (u16 UID_Address)
{
 u8 ID[12];
 STM8S_EEPROM_Read_String(ID, UID_Address, 12);
 return Crc8(12, ID);
}