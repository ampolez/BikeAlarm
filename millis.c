/* 
 *	Счетчик миллисекунд
 * Поддерживаемые чипы:
 *		ATMega328 @ 16Mhz Ext. Crystal
 *		ATTiny85 @ 8Mhz Int PPL
 */

#include "millis.h"
#include <avr/interrupt.h>
 
volatile unsigned long timer_millis;
 
ISR (TIMER1_COMPA_vect)
{
    timer_millis++;
}

// Получить текущее значение миллисекунд
unsigned long millis ()
{
	unsigned long millis_return;
	
	// Это нельзая прерывать!!!
	ATOMIC_BLOCK(ATOMIC_FORCEON) 
	{
		millis_return = timer_millis;
	}

	return millis_return;
}

void millis_reset() {
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		timer_millis = 0;
	}	
}

// Настройка таймера для счетчика миллисекунд
void millis_init()
{
	// Настройки для Atmega328 - Таймер 1
	#ifdef ATMega328
		TCCR1B |= (1 << WGM12) | (1 << CS11);		// Режим CTC, Предделитель 8
		OCR1AH = (CTC_MATCH_OVERFLOW >> 8);			// Старший байт значения для сравнения
		OCR1AL = CTC_MATCH_OVERFLOW;				// Младший байт значения для сравнения
		TIMSK1 |= (1 << OCIE1A);					// Вкл. прерывание по совпадению
	#endif

	#ifdef ATTiny85 
		TCCR0A |= (1 << WGM01);					// Режим CTC
		TCCR0B |= (1 << CS01 | 1 << CS00);		// Предделитель 64
		OCR0A = CTC_MATCH_OVERFLOW;				// Значение для сравнения
		TIMSK |= (1 << OCIE0A);					// Вкл. прерывание по совпадению
	#endif
}