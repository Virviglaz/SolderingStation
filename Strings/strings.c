#include "strings.h"

void uLongToStr (unsigned long Value, char * String)
{
  char zero=0;
  char tmp=0;
  unsigned long cnt=1000000000;
  while (cnt!=1)
  {
    while (Value>=cnt)
    {
      Value-=cnt;
      tmp++;
    }
    if (tmp) zero=1;
    if (zero) * String++ = tmp+48;
    tmp=0;
    cnt/=10;
  }
  * String = Value+48;
}

void uIntToStr (unsigned int Value, char * String)
{
  char zero=0;
  char tmp=0;
  unsigned int cnt=10000;
  while (cnt!=1)
  {
    while (Value>=cnt)
    {
      Value-=cnt;
      tmp++;
    }
    if (tmp) zero=1;
    if (zero) * String++ = tmp+48;
    tmp=0;
    cnt/=10;
  }
  * String = Value+48;
}

void sLongToStr (signed long Value, char * String)
{
	unsigned long tmp;
	if (Value<0) 
	{
		Value=-Value;
		tmp=Value;
		String[0]='-';
		uLongToStr(tmp, String+1);
	}
	else
	{
		tmp=Value;
		uLongToStr(tmp, String);
	}
}

void uCharToStr (unsigned char Value, char * String)
{
  char zero=0;
  char tmp=0;
  unsigned long cnt=100;
  while (cnt!=1)
  {
    while (Value>=cnt)
    {
      Value-=cnt;
      tmp++;
    }
    if (tmp) zero=1;
    if (zero) * String++ = tmp+48;
    tmp=0;
    cnt/=10;
  }
  * String = Value+48;
}

void sCharToStr (signed char Value, char * String)
{
	unsigned char tmp;
	if (Value<0) 
	{
		Value=-Value;
		tmp=Value;
		String[0]='-';
		uCharToStr(tmp, String+1);
	}
	else
	{
		tmp=Value;
		uCharToStr(tmp, String);
	}
}

void FillString (char * Data, char * String)
{
  while (* Data) * String++=* Data++;
}


void ClearString (char * String, char Len, char Data)
{
  while (Len--) * String++=Data;
}

void StringMoveLeft (char * String, char Len)
{
        char tmp;
        char dat = String[0];
        char cnt;
        if (Len) tmp = Len-1;
                else tmp = StringLen(String)-1;
        for (cnt=0; cnt!=tmp; cnt++) String[cnt]=String[cnt+1];
        String[tmp]=dat;
}

void StringMoveRight (char * String, char Len)
{
        char tmp;
        char dat;
        char cnt;
        if (Len) tmp = Len-1;
                else tmp = StringLen(String)-1;
        dat = String[Len-1];
        for (cnt=tmp; cnt!=0; cnt--) String[cnt]=String[cnt-1];
        String[0]=dat;
}

unsigned char StringLen (char * String)
{
        char cnt=0;
        while (* String++) cnt++;
        return cnt;
}

void ValueToStringHEX_Byte (unsigned char Value, char * String)
{
   const unsigned char HEX_Var[17]={"0123456789ABCDEF"};
   * String++ =   ((Value&0xF0)>>4)[HEX_Var];
   * String   =    (Value&0x0F)[HEX_Var];
}

void ValueToStringHEX_Word (unsigned int Value, char * String)
{
   const unsigned char HEX_Var[17]={"0123456789ABCDEF"};
   * String++ =   ((Value&0xF000)>>12)[HEX_Var];
   * String++ =   ((Value&0x0F00)>>8)[HEX_Var];
   * String++ =   ((Value&0x00F0)>>4)[HEX_Var];
   * String   =    (Value&0x000F)[HEX_Var];
}

void ValueToStringHEX_Long (unsigned long Value, char * String)
{
   const unsigned char HEX_Var[17]={"0123456789ABCDEF"};
   * String++ =   ((Value&0xF0000000)>>28)[HEX_Var];
   * String++ =   ((Value&0x0F000000)>>24)[HEX_Var];
   * String++ =   ((Value&0x00F00000)>>20)[HEX_Var];
   * String++ =   ((Value&0x000F0000)>>16)[HEX_Var];
   * String++ =   ((Value&0x0000F000)>>12)[HEX_Var];
   * String++ =   ((Value&0x00000F00)>>8)[HEX_Var];
   * String++ =   ((Value&0x000000F0)>>4)[HEX_Var];
   * String   =    (Value&0x0000000F)[HEX_Var];
}

signed char uCharTosChar (unsigned char Value)
{
	signed char Result;
	if ((Value)&(1<<7)) Result = - (0xFF - Value);
	else Result = Value;
	return Result;
}

signed int uIntTosInt (unsigned int Value)
{
	signed int Result;
	if ((Value)&(1<<15)) Result = - (0xFFFF - Value);
	else Result = Value;
	return Result;
}

signed long uLongTosLong (unsigned long Value)
{
	signed long Result;
	if ((Value)&(1<<30)) Result = - (0xFFFFFFFF - Value);
	else Result = Value;
	return Result;
}

unsigned char sCharTouChar (signed char Value)
{
	unsigned char tmp;
	if (Value<0) 
	{
		Value=-Value;
		tmp=Value;
	}
	else tmp=Value;
	return tmp;
}

void FloatToString (float Value, char * String, char Accuracy)
{
        signed long k = 1;
        signed long tmp;
        char Len, Len2, i=0;
        char AftDot[6];
        for (tmp = Accuracy; tmp; tmp--)  k *= 10;
        tmp = MOD(((signed long)Value)*k - Value*k);
        if (Value<0) 
        {
         *String++ = '-';
         sLongToStr((int)(-Value), String);
        }
        else
        sLongToStr((int)(-Value), String);
        Len = StringLen(String);
        String[Len++]='.';
        sLongToStr(tmp, AftDot);
        Len2 =  StringLen(AftDot);

        while (Accuracy--)
        {
              if (Accuracy>=Len2) String[Len++] = '0';
              else String[Len++] = AftDot[i++];
        }
}

void StringReplaceChar (char * String, char Char1, char Char2)
{
	char cnt = sizeof(String);
	while (cnt--)
		if (String[cnt]==Char1) String[cnt]=Char2;
}

unsigned long MOD (signed long Value)
{
        if (Value>0) return Value;
        else return -Value;
}

int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
    char m = 0;
    int ipart;
    float fpart;
    int i, p = afterpoint;
    if (n < 0)
    {
      n = -n;
      m = 1;
      res[0] = '-';
    }
    if ((char)n == 0)
    {
      res[1] = '0';
      m++;
    }
    ipart = (int)n;
    fpart = n - (float)ipart;
    i = intToStr(ipart, res + m, 0);
    if (afterpoint != 0)
    {
        res[i+m] = '.';  // add dot
        while (p--) fpart *= 10;
        intToStr((int)fpart, res + i + 1 + m, afterpoint);
    }
}
void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}