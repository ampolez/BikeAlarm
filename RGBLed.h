/*
 * RGBLed.h
 *
 * Created: 25.04.2016 23:16:44
 *  Author: Андрей
 */ 

#ifndef RGBLED_H_
#define RGBLED_H_

// Настройки подключения статусного светодиода
#define RGB_STATUS_PORT PORTD	//PORTD
#define RGB_STATUS_PIN PIND
#define RGB_STATUS_DDR DDRD
#define RGB_STATUS_RED_PIN 5		//4
#define RGB_STATUS_GREEN_PIN 6		//2
#define RGB_STATUS_BLUE_PIN 7		//7

// Шорткоды для цветов статусного диода
typedef uint8_t t_RGBColor;
#define COLOR_NONE 0
#define COLOR_WHITE 1	// r+g+b
#define COLOR_RED 2
#define COLOR_GREEN 3
#define COLOR_BLUE 4
#define COLOR_YELLOW 5	// r+g
#define COLOR_VIOLET 6	// r+b
#define COLOR_NAVY 7	// g+b

// Шорткоды для режима мерцания статусного диода
typedef uint16_t t_BlinkSpeed;
#define SPEED_NONE 0		// диод выключен
#define SPEED_STILL 1		// диод включен постоянно
#define SPEED_SLOW 600		// часота мерцания
#define SPEED_MEDIUM 300	// часота мерцания
#define SPEED_FAST 100		// часота мерцания

void RGB_Init();										// Настройка подключения RGB-светодиода
void RGB_Disable();										// Отключение RGB-светодиода для режима энергосбережения		
void RGB_Blink(t_RGBColor _color, t_BlinkSpeed _speed);	// Зажечь светодиод, либо мигать им с указанной скоростью
void RGB_Light(t_RGBColor _color);							// Зачечь светодиод указанным цветом					
void RGB_StoreState(uint8_t _RGBPortState);				// Сохранить состояние порта, на котором находится RGB-светодиод
uint8_t RGB_GetStoredState();							// Получить сохраненное состояние порта RGB-светодиода

#endif /* RGBLED_H_ */