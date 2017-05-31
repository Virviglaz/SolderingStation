/* СПИСОК ПОДКЛЮЧЕННЫХ ФАЙЛОВ */
#include "HT_Board.h"                   //настройки и конфигурация платы
#include "stm8s_conf.h"                 //конфигурация библиотек SPL
#include "LCDv1.h"                      //процедуры вывода на дисплей
#include "STM8_GPIO.h"                  //порты ввода-вывода
#include "STM8_Delays.h"                //задержки
#include "STM8_SPI.h"                   //SPI интерфейс
#include "strings.h"                    //преобразование данных
#include "STM8S_EEPROM.h"               //управление EEPROM
#include "PID.h"                        //ПИД алгоритм

/* ЗАДЕКЛАРИРОВАННЫЕ ЛОКАЛЬНЫЕ ФУНКЦИИ И ПРОЦЕДУРЫ */
unsigned int GetTempDig (void);
unsigned int GetTempAn (void);
void PrintTemp (unsigned int Value);
void ButtonHandle (void);
void StoreData (void);
void GetData (void);
void PrintValues (void);
void PID_Handle (void);
void CAL_Handle (void);
float V_FB_Read (void); //volts on output
float I_FB_Read (void); //amps  on output

/* ЗАДЕКЛАРИРОВАННЫЕ ЛОКАЛЬНЫЕ ПЕРЕМЕННЫЕ */
volatile u16 SetTemp;
u16 RealTemp = 0;
volatile FlagStatus SecFlag = RESET;
volatile FlagStatus ValueChanged = RESET;
bool PowerON = false;
u16 Adc_Read (u8 AdcChannel);
u16 SetPWM = PWM_Step;
u16 PWM_ACC = PWM_Step;
double Setpoint, Input, Output;
PidType PID;
bool MAX6675_Error = false;
typedef struct 
{
  int offset;                   //линейное смещение зависимости температура/код
  int scale1000;                //множитель наклона прямой, умноженный на 1000
  char calibration;             //флаг включения калибровки
  char mode_enter;              //флаг смены режима работы
}AN_K_StructTypeDef;
AN_K_StructTypeDef AN_K = {0,1000,0,1}; //начальные значения ПИД

/* ОСНОВНАЯ ПРОГРАММА */
/* ОПИСАНИЕ:    ГЛАВНАЯ ТОЧКА ВХОДА ПОСЛЕ КОМАНДЫ СБРОСА */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
int main( void )
{
    /* НАСТРОЙКА ВНУТРЕННЕГО RC ГЕНЕРАТОРА НА 16 МГц */
  CLK->CKDIVR = 0;  //16Mhz OSC RC
  CLK->PCKENR1 = CLK_PCKENR1_SPI | CLK_PCKENR1_TIM1 | CLK_PCKENR1_TIM2;
  CLK->PCKENR2 = CLK_PCKENR2_ADC;
  
  PIN_OUT_PP(LCD_BackLight);            //инитиализация пина подсветки
  PIN_OUT_PP(ADC_CS);                   //инитиализация пина выбора АЦП
  BackLightON();                        //включить подсветку
  PIN_ON(ADC_CS);                       //освободить АЦП
  PIN_OUT_PP(PA3);                      //PWM - выход ШИМ     
  PIN_OUT_PP(PC5);                      //MSCK - тактовый выход SPI
  PIN_IN_PU(PC7);                       //MISO - вход SPI
  GPIO_Init(BTN, GPIO_MODE_IN_PU_NO_IT);        //инитиализация пина кнопки - включена поддяжка к питанию
  GPIO_Init(ENCA, GPIO_MODE_IN_PU_IT);          //инитиазилация пинов энкодера с подтяжкой и прерыванием
  GPIO_Init(ENCB, GPIO_MODE_IN_PU_NO_IT);
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_FALL_ONLY); //настройка прерывания
  delays_init(4);                               //инитиализация задержек с предделителем на 16
  TIM2_TimeBaseInit(TIM2_PRESCALER_16384, PWM_Period);  //инитиализация таймера 2 на частоту 16000000/16384 = ~976 Гц
  TIM2_OC3Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, PWM_Step,
               TIM2_OCPOLARITY_HIGH);           //инитиализация выхода ШИМ на TIM2_CH3
  TIM2_Cmd(ENABLE);                             //запуск таймера
  TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);        //прерывание по переполнению таймера 2
  SPI_Init(SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_8, SPI_MODE_MASTER, 
           SPI_CLOCKPOLARITY_LOW, SPI_CLOCKPHASE_1EDGE, SPI_DATADIRECTION_2LINES_FULLDUPLEX,
           SPI_NSS_SOFT, 0);                    //инитиализация SPI
  SPI_Cmd(ENABLE);                              //запуска SPI
  GetData();                                    //вернуть настройки из EEPROM
  
  /* ИНИТИАЛИЗАЦИЯ АЦП */
  ADC1->TDRL = V_FB | I_FB | AN_TEMP;
  ADC1->CR1 = ADC1_PRESSEL_FCPU_D18;
  ADC1->CR2 = ADC1_ALIGN_RIGHT;
  ADC1->CR1 |= ADC1_CR1_ADON;
  delay_ms(100);                                //задержка 100мс
  LCD_init();                                   //инитиализация дисплея
  
  PID_init(&PID, 1, 0.05, 0.25, PID_Direction_Direct);  //инитиализация ПИД
  PID_SetMode(&PID, PID_Mode_Automatic);
  PID_SetOutputLimits(&PID, PWM_Step, PWM_Period);
  delay_ms(300);
  GetTempDig();
  GetTempDig();
  if (GetTempDig() > 700) MAX6675_Error = true;         //если MAX6675 не установлен, код вернет 0xFFFF, что явно больше 700
  if (PIN_SYG(BTN) == 0 && MAX6675_Error == true)       //если в момент включения нажата кнопка, войти в калибровку
  {
    AN_K.calibration = 1;                               //включить калибровку
    LCD_Send_Cmd(LCD_CMD_Clear);                        //очистить экран
    TIM2_SetCompare3(0);                                //выключить нагреватель
    LCD_SetPosition(0,0);                               //курсор на позицию 0,0
    LCD_PrintString("Callibration ");                   //вывести сообщение
    while(PIN_SYG(BTN) == 0);                           //пока кнопка не нажата, тупить здесь
  }
  rim();                                                //разрешить все прерывания
  while(1)                                              //вечный цикл
  {
    if (AN_K.calibration) CAL_Handle();                 //если включена калибровка, выполнять ее
      else PID_Handle();                                //в противном случае обрабатывать ПИД
    ButtonHandle();                                     //и не забывать про кнопки
  }
}

/* ОБРАБОТЧИК КАЛИБРОВКИ */
/* ОПИСАНИЕ:    ВЫПОЛНЯЕТСЯ В ЦИКЛЕ ДО ОКОНЧАНИЯ КАЛИБРОВКИ */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
void CAL_Handle (void)
{
  char txtt[5];
  BackLightON();
  if (AN_K.mode_enter)
  {
    static int x1,x2,y1,y2;
    AN_K.mode_enter = 0;
    LCD_SetPosition(0,14); 
    if (AN_K.calibration == 1)
    {
      LCD_PrintString("#1");
      SetTemp = 20;
    }
    if (AN_K.calibration == 2)
    {
      LCD_PrintString("#2");    //first calc
      Adc_Read(AN_TEMP);
      x1 = Adc_Read(AN_TEMP);
      y1 = SetTemp;
      SetTemp = 300;
      TIM2_SetCompare3(PWM_Period / 8);
    }
    LCD_SetPosition(1,0);
    LCD_PrintString("Temp:    ADC:");
    if (AN_K.calibration == 3) //second calc
    {
      Adc_Read(AN_TEMP);
      x2 = Adc_Read(AN_TEMP);
      y2 = SetTemp;
      x2 = x2 - x1;
      y2 = y2 - y1;
      AN_K.scale1000 = (s16)(1000 * (float)y2 / (float)x2);
      AN_K.offset = (s16)((x1 - y1) * (float)AN_K.scale1000 / 1000);
      AN_K.calibration = 0;
      LCD_Send_Cmd(LCD_CMD_Clear);
      LCD_SetPosition(0,0);
      StoreData();
      return;
    }
    AN_K.calibration++;
  }
  delay_ms(250);
  if (SetTemp < 20) SetTemp = 20;
  if (SetTemp > 400) SetTemp = 400;
  if (PIN_SYG(GPIOA,GPIO_PIN_3)) return;
  
  ClearString(txtt, sizeof(txtt), 32);
  Adc_Read(AN_TEMP);
  uIntToStr(Adc_Read(AN_TEMP), txtt);
  LCD_SetPosition(1,13);
  LCD_PrintStringLen(txtt,3);
  
  ClearString(txtt, sizeof(txtt), 32);
  uIntToStr(SetTemp, txtt);
  LCD_SetPosition(1,5);
  LCD_PrintStringLen(txtt,3);
}

/* ОБРАБОТЧИК РЕГУЛИРОВАНИЯ ПИД */
/* ОПИСАНИЕ:    ВЫПОЛНЯЕТСЯ В ЦИКЛЕ ДО ОТКЛЮЧЕНИЯ */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
void PID_Handle (void)
{
  if (ValueChanged == SET) PrintTemp(GetTempDig());
  ValueChanged = RESET;
  if (SecFlag == RESET) return;
  if (SetTemp > TempMAX) SetTemp = TempMAX;
  if (SetTemp < TempMIN) SetTemp = TempMIN;
  if (MAX6675_Error) RealTemp = GetTempAn();
    else RealTemp = GetTempDig(); 
  
  if (PowerON == false) 
  {
    SetPWM = 0;
    if (RealTemp < BackLightOffTemp) BackLightOFF();
  }
  else
  {
    BackLightON();
    PID.mySetpoint = SetTemp;    
    PID.myInput = RealTemp;
    PID_Compute(&PID);    
    SetPWM = (u16)PID.myOutput;
  }
  
  PrintTemp(RealTemp);
  TIM2_SetCompare3(SetPWM);
  PrintValues();
  SecFlag = RESET;
}

/* ОБРАБОТЧИК НАЖАТИЯ НА КНОПКУ */
/* ОПИСАНИЕ:    ВЫПОЛНЯЕТСЯ В ЦИКЛЕ */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
void ButtonHandle (void)
{
  static FlagStatus BTN_Trigger = RESET;
  static u8 ON_Cnt = 0;
  if (PIN_SYG(BTN) == 0 && BTN_Trigger == RESET)
  {
    BTN_Trigger = SET;
    if (AN_K.calibration == 0)
    {
      LCD_Send_Cmd(LCD_CMD_Clear);
      if (PowerON) 
      {
        StoreData();  
        PowerON = false;
        ON_Cnt = 0;
      }
      else
      {
        BackLightON();
        while(PIN_SYG(BTN) == 0)
        {
          delay_ms(100);
          if (ON_Cnt < 16) 
          {
            LCD_PrintChar(0xFF);
            ON_Cnt++;
          }
          else 
          {
            PowerON = true;
            delay_ms(100);
            ON_Cnt = 0;
            return;
          }
        }
      }
    }
    else
      AN_K.mode_enter = 1;  
  }
  if (PIN_SYG(BTN) && BTN_Trigger == SET) 
  {
    BTN_Trigger = RESET;
    ON_Cnt = 0;
    delay_ms(100);
  }
}

/* ОПИСАНИЕ:    ВОЗВРАЩАЕТ ЗНАЧЕНИЕ ТЕМПЕРАТУРЫ С ВНЕШНЕГО АЦП ПО SPI */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     16 bit значение температуры 0...32768 */
unsigned int GetTempDig (void)
{
  unsigned int res; 
  PIN_OFF(ADC_CS);
  res = (SPI_ReadByte(0)<<8);
  res|= SPI_ReadByte(0);
  PIN_ON(ADC_CS);
  return ((res&ADCBitMask)>>5);
}

/* ОПИСАНИЕ:    ВОЗВРАЩАЕТ ЗНАЧЕНИЕ ТЕМПЕРАТУРЫ С ВНУТРЕННЕГО АЦП */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     16 БИТ ЗНАЧЕНИЕ ТЕМПЕРАТУРЫ 0...32768 */
unsigned int GetTempAn (void)
{
  Adc_Read(AN_TEMP);
  return (int)((float)((float)AN_K.scale1000 / 1000) * Adc_Read(AN_TEMP)) - AN_K.offset;
}

/* ОПИСАНИЕ:    ВЫВОДИТ ЗНАЧЕНИЕ ТЕМПЕРАТУРЫ НА ЭКРАН */
/* ПАРАМЕТРЫ:   16 БИТ ЗНАЧЕНИЕ ТЕМПЕРАТУРЫ 0...32768 */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
void PrintTemp (unsigned int Value)
{
  char txtt[5];
  char txtc[4] = { 0xDF, 'C', 32, 0}; 
  
  LCD_SetPosition(1,0);
  LCD_SetPosition(1,0);
  ClearString(txtt, sizeof(txtt), 0);
  uIntToStr(SetTemp, txtt);
  LCD_PrintString(txtt);
  LCD_PrintString(txtc);
  
  LCD_SetPosition(1,11);
  if (Value > TempMAX*2) 
  {
    TIM2_SetCompare3(0);
    PowerON = false;
    LCD_PrintString("---");
    return;
  }
  ClearString(txtt, sizeof(txtt), 0);
  uIntToStr(Value, txtt);
  LCD_PrintString(txtt);
  LCD_PrintString(txtc);
}

/* ПРЕРЫВАНИЕ ПО ИЗМЕНЕНИЮ ФРОНТА НА ПОРТУ В */
/* ОПИСАНИЕ:    ОБРАБАТЫВАЕТ СОБЫТИЕ ПОВОРОТ ЭНКОДЕРА */
/* ПАРАМЕТРЫ:   ФЛАГ ПРЕРЫВАНИЯ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
INTERRUPT_HANDLER(EXTI_PORTB_IRQHandler, 4)
{
  if (PIN_SYG(ENCB)) SetTemp -= 2;
  else SetTemp += 2;
  ValueChanged = SET;
}

/* ПРЕРЫВАНИЕ ПО ПЕРЕПОЛНЕНИЮ ТАЙМЕРА 2 */
/* ОПИСАНИЕ:    ПОДНИМАЕТ ФЛАГ SecFlag каждую секунду */
/* ПАРАМЕТРЫ:   ФЛАГ ПРЕРЫВАНИЯ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
 INTERRUPT_HANDLER(TIM2_UPD_OVF_BRK_IRQHandler, 13)
 {
  SecFlag = SET;
  TIM2_ClearITPendingBit(TIM2_IT_UPDATE);
 }

/* ПРЕРЫВАНИЕ ПО ПЕРЕПОЛНЕНИЮ ТАЙМЕРА 1 */
/* ОПИСАНИЕ:    В ДАННОЙ РЕАЛИЗАЦИИ НЕ ИСПОЛЬЗУЕТСЯ */
/* ПАРАМЕТРЫ:   ФЛАГ ПРЕРЫВАНИЯ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
INTERRUPT_HANDLER(TIM1_UPD_OVF_TRG_BRK_IRQHandler, 11)
{
  TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
}

/* ЧИТАЕТ ДАННЫЕ АЦП ПО ОПРЕДЕЛЕННОМУ КАНАЛУ */
/* ОПИСАНИЕ:    ЗАПУСКАЕТ ПРЕОБРАЗОВАНИЕ И ЧИТАЕТ РЕЗУЛЬТАТ */
/* ПАРАМЕТРЫ:   8 БИТ КАНАЛ ВСТРОЕННОГО АЦП */
/* ВОЗВРАТ:     16 БИТ ЗНАЧЕНИЕ РЕЗУЛЬТАТА */
u16 Adc_Read (u8 AdcChannel)
{
  u16 Result;
  ADC1->CSR = AdcChannel;
  ADC1->CR1 |= ADC1_CR1_ADON;
  while ((ADC1->CSR&ADC1_CSR_EOC)==0);
  Result = (ADC1->DRH << 8);
  Result |= ADC1->DRL;
  return Result;
}

/* НАПРЯЖЕНИЕ ПИТАНИЯ УСТРОЙСТВА */
/* ОПИСАНИЕ:    СЧИТАЕТ НАПРЯЖЕНИЕ ПИТАНИЯ */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     НАПРЯЖЕНИЕ В ВОЛЬТАХ С ПЛАВАЮЩЕЙ ТОЧКОЙ */
float V_FB_Read (void)
{
  const u8 R9 = 100;
  const u8 R10 = 10;
  static u16 ResV;
  Adc_Read(V_FB);
  ResV = Adc_Read(V_FB);
  return (float)ResV * 5 / 1024 / R10 * (R9 + R10);
}

/* ТОК ЧЕРЕЗ НАГРЕВАТЕЛЬ */
/* ОПИСАНИЕ:    СЧИТАЕТ ТОК, ПРОХОДЯЩИЙ ЧЕРЕЗ НАГРЕВАТЕЛЬНЫЙ ЭЛЕМЕНТ */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     ТОК В АМПЕРАХ С ПЛАВАЮЩЕЙ ТОЧКОЙ */
float I_FB_Read (void)
{
  static u16 ResI;
  Adc_Read(I_FB);
  ResI = Adc_Read(I_FB);
  return (float)ResI * 50 / 1024;
}

/* ОПИСАНИЕ:    ВЫВОДИТ ДАННЫЕ НА ЭКРАН */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
void PrintValues (void)
{
  static float Volts, Amps, Power;
  static char txt[6];
  Amps = I_FB_Read();
  Volts = V_FB_Read();
  
  Power = Volts * Amps * TIM2_GetCapture3() / PWM_Period;
  
  LCD_SetPosition(0,0); 
  if (PowerON == false) LCD_PrintString(" OFF ");
  else
  {   
    ClearString(txt, sizeof(txt), 0); 
    ftoa(Volts, txt, 1);
    LCD_PrintString(txt);
    LCD_PrintString("V     ");
  }
  
  
  LCD_SetPosition(0,6); 
  ClearString(txt, sizeof(txt), 0); 
  ftoa(Amps, txt, 1);
  if (txt[0] == 0) txt[0] = '0';
  LCD_PrintString(txt); 
  LCD_PrintString("A     ");
  
  LCD_SetPosition(0,11); 
  ClearString(txt, sizeof(txt), 0); 
  ftoa(Power, txt, 1);
  if (txt[0] == 0) txt[0] = '0';
  LCD_PrintString(txt);
  LCD_PrintString("W ");
}

/* ОПИСАНИЕ:    СОХРАНЯЕТ НАСТРОЙКИ В EEPROM */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
void StoreData (void)
{
  FLASH_Unlock(FLASH_MEMTYPE_DATA);
  FLASH_ProgramByte(0x4000, SetTemp >> 8);
  FLASH_ProgramByte(0x4001, (u8)SetTemp);
  FLASH_ProgramByte(0x4002, AN_K.offset >> 8);
  FLASH_ProgramByte(0x4003, (u8)AN_K.offset);
  FLASH_ProgramByte(0x4004, AN_K.scale1000 >> 8);
  FLASH_ProgramByte(0x4005, (u8)AN_K.scale1000);
  FLASH_Lock(FLASH_MEMTYPE_DATA);
}

/* ОПИСАНИЕ:    ВОЗВРАЩАЕТ НАСТРОЙКИ ИЗ EEPROM */
/* ПАРАМЕТРЫ:   ОТСУТСТВУЮТ */
/* ВОЗВРАТ:     ОТСУТСТВУЕТ */
void GetData (void)
{
  SetTemp = FLASH_ReadByte(0x4000) << 8 | FLASH_ReadByte(0x4001);
  AN_K.offset = FLASH_ReadByte(0x4002) << 8 | FLASH_ReadByte(0x4003);
  AN_K.scale1000 = FLASH_ReadByte(0x4004) << 8 | FLASH_ReadByte(0x4005);
  
  if (SetTemp == 0 || SetTemp == 0xFFFF) SetTemp = 320; //default value
  if (AN_K.offset == 0 || AN_K.offset == 0xFFFF) AN_K.offset = 0x0037; //default value
  if (AN_K.scale1000 == 0 || AN_K.scale1000 == 0xFFFF) AN_K.scale1000 = 0x02A6; //default value
}