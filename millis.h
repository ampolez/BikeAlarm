/*
 * millis.h
 *
 *  Author: Андрей
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#ifndef MILLIS_H_
#define MILLIS_H_

#define ATMega328
//#define ATTiny85

#ifdef ATMega328
	#define CTC_MATCH_OVERFLOW ((F_CPU / 1000) / 8)		// Счетчик для миллисекунд Atmega328 @ 16Mhz
#endif

#ifdef ATTiny85
	#define CTC_MATCH_OVERFLOW ((F_CPU / 1000) / 64)	// Счетчик для миллисекунд Attiny85 @ 8Mhz
#endif

void millis_init();
void millis_reset();
unsigned long millis ();


#endif /* MILLIS_H_ */