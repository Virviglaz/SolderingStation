#include "stm8s_flash.h"
#include "CRC.h"

void STM8S_EEPROM_Init (void);
void STM8S_EEPROM_WriteString (u16 address, u8 * String);
void STM8S_EEPROM_Read_String (u8 * String, u16 Address, u8 Len);
u8 GenerateUniqueID8 (u16 UID_Address);