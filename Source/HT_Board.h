/* НАСТРОЙКИ */
#define BackLightOffTemp                70              //ниже этой температуры выключается подсветка экрана при остывании
#define TempMAX                         420             //максимально допустимая температура (защита жала)
#define TempMIN                         180             //минимально допустимая температура (защита от дибила)

/* КОНФИГУРАЦИЯ ПЛАТЫ */
#define LCD_BackLight                   PD0
#define AN_TEMP                         ADC1_CHANNEL_0
#define V_FB                            ADC1_CHANNEL_1
#define I_FB                            ADC1_CHANNEL_2
#define ENCA                            PB6
#define ENCB                            PB7
#define BTN                             PF4
#define ADC_CS                          PC4
#define ADCBitMask                      0x7FF8
#define PWM_Period                      500
#define PWM_Step                        50
#define PWM_Min                         100
#define BackLightON()                     PIN_ON(LCD_BackLight)
#define BackLightOFF()                     PIN_OFF(LCD_BackLight)