#include "STM8_SPI.h"

u8 SPI_ReadByte (unsigned char Dat)
{
  //while (SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET);
  
  //write data to SPI_DR reg
  SPI->DR = Dat; 
  
  //wait TXE flag
  while (!(SPI->SR&SPI_SR_TXE));
  
  //wait RXNE flag
  
  while (!(SPI->SR&SPI_SR_RXNE));
  //while (SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET);
  return SPI->DR; 
}

void SPI_SendByte (unsigned char Dat)
{
  SPI_ReadByte (Dat);
}