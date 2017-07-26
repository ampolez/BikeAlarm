/*
 * RGBLed.c
 *
 * Created: 25.04.2016 23:16:25
 *  Author: Андрей
 */ 

#include <stdlib.h>
#include <avr/io.h>
#include "RGBLed.h"
#include "millis.h"

unsigned long msCount = 0;					// Счетчик миллисекунд для мерцания
uint8_t LedIsOn = 0;						// Флаг зажженного светодиода
uint8_t RGBPortState;						// Слепок состояния выводов светодиода

/*
 * Настройка подключения RGB-светодиода
 */
void RGB_Init() 
{
	RGB_STATUS_DDR |= 1 << RGB_STATUS_RED_PIN | 1 << RGB_STATUS_GREEN_PIN | 1 << RGB_STATUS_BLUE_PIN;		// Все пины на выход
}

/*
 * Отключение RGB-светодиода для режима энергосбережения
 */
void RGB_Disable() 
{
	RGB_STATUS_DDR &= ~(1 << RGB_STATUS_RED_PIN | 1 << RGB_STATUS_GREEN_PIN | 1 << RGB_STATUS_BLUE_PIN);	// Все пины на вход
	RGB_STATUS_PORT &= ~(1 << RGB_STATUS_RED_PIN | 1 << RGB_STATUS_GREEN_PIN | 1 << RGB_STATUS_BLUE_PIN);	// Вход - в логический ноль
}

/*
 * Получить сохраненное состояние порта RGB-светодиода
 */
uint8_t RGB_GetStoredState() 
{
	return RGBPortState;	
}

/*
 * Сохранить состояние порта, на котором находится RGB-светодиод
 * @_RGBPortState - оттиск состояния порта
 */
void RGB_StoreState(uint8_t _RGBPortState) 
{
	RGBPortState = _RGBPortState;	
}


/*
 * Зажечь светодиод, если установлена скорость - запустить процесс мерцания
 * @_color - цвет мерцания
 * @_duaration - длительность периодов "зажжен" и "потушен"
 */
void RGB_Blink(t_RGBColor _color, t_BlinkSpeed _speed)
{
	uint8_t _state = 1;
	
	if(_speed != SPEED_NONE) 
	{
		if (LedIsOn)
		{
			_state = 0;
			_color = COLOR_NONE;
		}

		if(millis() - msCount > _speed)
		{
			RGB_Light(_color);
			msCount = millis();
			LedIsOn = _state;
		}
		
	} else {
		RGB_Light(_color);	
	}

}


/*
 * Зажечь светодиод указанным зветом
 * @_color - цвет
 */
void RGB_Light(t_RGBColor _color)
{
	RGBPortState = RGB_STATUS_PORT;
	switch (_color)
	{
		case COLOR_NONE:
			RGBPortState |= (1 << RGB_STATUS_RED_PIN | 1 << RGB_STATUS_GREEN_PIN | 1 << RGB_STATUS_BLUE_PIN);
		break;
		case COLOR_WHITE:
			RGBPortState &= ~(1 << RGB_STATUS_RED_PIN | 1 << RGB_STATUS_GREEN_PIN | 1 << RGB_STATUS_BLUE_PIN);
		break;
		case COLOR_RED:
			RGBPortState &= ~(1 << RGB_STATUS_RED_PIN);
			RGBPortState |= (1 << RGB_STATUS_GREEN_PIN | 1 << RGB_STATUS_BLUE_PIN);
		break;
		case COLOR_GREEN:
			RGBPortState &= ~(1 << RGB_STATUS_GREEN_PIN);
			RGBPortState |= (1 << RGB_STATUS_RED_PIN | 1 << RGB_STATUS_BLUE_PIN);
		break;
		case COLOR_BLUE:
			RGBPortState &= ~(1 << RGB_STATUS_BLUE_PIN);
			RGBPortState |= (1 << RGB_STATUS_RED_PIN | 1 << RGB_STATUS_GREEN_PIN);
		break;
		case COLOR_YELLOW:
			RGBPortState &= ~(1 << RGB_STATUS_RED_PIN | 1 << RGB_STATUS_GREEN_PIN);
			RGBPortState |= (1 << RGB_STATUS_BLUE_PIN);
		break;
		case COLOR_VIOLET:
			RGBPortState &= ~(1 << RGB_STATUS_RED_PIN | 1 << RGB_STATUS_BLUE_PIN);
			RGBPortState |= (1 << RGB_STATUS_GREEN_PIN);
		break;
		case COLOR_NAVY:
			RGBPortState &= ~(1 << RGB_STATUS_GREEN_PIN | 1 << RGB_STATUS_BLUE_PIN);
			RGBPortState |= (1 << RGB_STATUS_RED_PIN);
		break;
	}
	RGB_STATUS_PORT = RGBPortState;
}
