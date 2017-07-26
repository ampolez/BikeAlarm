/*
 * Synth.c
 *
 * Created: 27.12.2015 22:58:19
 *  Author: Андрей
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "Synth.h"

#define EDGE_FALLING 0
#define EDGE_RISING 1

void Loud_Set () {
	BUZZER_PORT &= ~(1 << BUZZER_PIN2); 
	_delay_us(10);
	BUZZER_PORT |= 1 << BUZZER_PIN1;
}

void Loud_Reset () {
	BUZZER_PORT &= ~(1 << BUZZER_PIN1);
	_delay_us(10);
	BUZZER_PORT |= 1 << BUZZER_PIN2;
}

void Silent_Set () {
	BUZZER_PORT |= 1 << BUZZER_PIN1;
}

void Silent_Reset () {
	BUZZER_PORT &= ~(1 << BUZZER_PIN1);
}

void SoundHigh (uint8_t isSilent) 
{
	if(isSilent == 1) 
	{
		Silent_Set();	
	} else 
	{
		Loud_Set();	
	}	
}
void SoundLow (uint8_t isSilent)
{
	if(isSilent == 1)
	{
		Silent_Reset();
	} else
	{
		Loud_Reset();
	}	
}



void SoundOff()
{
	BUZZER_PORT &= ~(1 << BUZZER_PIN1 | 1 << BUZZER_PIN2);
}

void StopTimer() 
{
	TCCR0B &= ~(1 << CS01 | 1 << CS00);		// Таймер выключен
}

void StartTimer() 
{
	TCCR0B |= (1 << CS01) | (1 << CS00);		// Предделитель 64	
}

void SoundInit() 
{
	BUZZER_DDR |= 1<<BUZZER_PIN1 | 1<<BUZZER_PIN2;	// Пины для динамика - на выход
	SoundOff();

	// Таймер для генератора
	//TCCR0A |= 1 << WGM01;							// Режим CTC
	//TIMSK0 |= 1 << TOIE0;							// Прерывание по переполнению
}


uint16_t SoundFrames[] = {1000, 700, 600, 500, 400, 300, 250};
volatile uint8_t CurrentFrame = 0;
volatile uint8_t Edge = EDGE_FALLING;

ISR (TIMER0_OVF_vect)
{
	if(Edge == EDGE_FALLING)
	{
		Loud_Set();
		TCNT0 = 255-SoundFrames[CurrentFrame];
		Edge = EDGE_RISING;
	}
	else if (Edge == EDGE_RISING)
	{
		Loud_Reset();
		TCNT0 = 255-SoundFrames[CurrentFrame];
		Edge = EDGE_FALLING;
		if(CurrentFrame<7)
		{
			CurrentFrame++;
		}
		else
		{
			CurrentFrame = 0;
			StopTimer();
		}
	}
}

void Horn1()
{
	StartTimer();
}


void freq (int f, uint8_t isSilent)
{
	int i;
	SoundHigh(isSilent);
	for (i=0;i<f;i++)
	{
		_delay_us(5);
	}
	SoundLow(isSilent);
	for (i=0;i<f;i++)
	{
		_delay_us(5);
	}
}

void Sound1(uint8_t isSilent)
{
	int i;
	for(i=250;i<500;i=i+1)
	{
		freq(i, isSilent);
		freq(i, isSilent);
		freq(i, isSilent);
	}
	for(i=500;i>250;i=i-1)
	{
		freq(i, isSilent);
		freq(i, isSilent);
		freq(i, isSilent);
	}

}
void Sound2(uint8_t isSilent)
{
	int i;
	for(i=250;i<500;i=i+3)
	{
		freq(i, isSilent);
	}
	for(i=500;i>250;i=i-3)
	{
		freq(i, isSilent);
	}

}

void Alarm() 
{
	int i, a, b;
	for(b = 0; b < 5; b++) {
		for(a = 0; a < 5; a++)
		{
			for(i=400;i<455;i=i+5)
			{
				freq(i, 0);
			}
			_delay_ms(10);
			for(i=355;i>300;i=i-5)
			{
				freq(i, 0);
			}
			//_delay_ms(10);
		}
		_delay_ms(10);
		for(a = 0; a < 20; a++) {
			for(i=150;i<155;i=i+2)
			{
				freq(i, 0);
			}
			_delay_ms(10);			
		}
	}
}

void Warning() 
{
	int i, a;
	for(a = 0; a < 3; a++) {
		for(i=500;i<515;i=i+3)
		{
			freq(i, 0);
		}
		_delay_ms(120);		
	}
}


void Horn(uint8_t isSilent)
{
	// 16MHz - 1x
	// 8MHz - 2x	
	#if F_CPU == 16000000
		SoundHigh(isSilent);_delay_us(1000);
		SoundLow(isSilent);_delay_us(1000);
		SoundHigh(isSilent);_delay_us(700);
		SoundLow(isSilent);_delay_us(700);	
		SoundHigh(isSilent);_delay_us(600);
		SoundLow(isSilent);_delay_us(600);	
		SoundHigh(isSilent);_delay_us(500);
		SoundLow(isSilent);_delay_us(500);	
		SoundHigh(isSilent);_delay_us(400);
		SoundLow(isSilent);_delay_us(400);	
		SoundHigh(isSilent);_delay_us(300);
		SoundLow(isSilent);_delay_us(300);	
		SoundHigh(isSilent);_delay_us(250);
		SoundLow(isSilent);_delay_us(250);	
		SoundHigh(isSilent);_delay_us(250);
		SoundLow(isSilent);_delay_us(250);	
		SoundHigh(isSilent);_delay_us(300);
		SoundLow(isSilent);_delay_us(300);	
		SoundHigh(isSilent);_delay_us(400);
		SoundLow(isSilent);_delay_us(400);	
		SoundHigh(isSilent);_delay_us(500);
		SoundLow(isSilent);_delay_us(500);	
		SoundHigh(isSilent);_delay_us(600);
		SoundLow(isSilent);_delay_us(600);	
		SoundHigh(isSilent);_delay_us(700);
		SoundLow(isSilent);_delay_us(700);	
		SoundHigh(isSilent);_delay_us(1000);
		SoundLow(isSilent);_delay_us(1000);

	#elif F_CPU == 8000000
		SoundHigh(isSilent);_delay_us(2000);
		SoundLow(isSilent);_delay_us(2000);	
		SoundHigh(isSilent);_delay_us(1400);
		SoundLow(isSilent);_delay_us(1400);
		SoundHigh(isSilent);_delay_us(1200);
		SoundLow(isSilent);_delay_us(1200);		
		SoundHigh(isSilent);_delay_us(1000);
		SoundLow(isSilent);_delay_us(1000);		
		SoundHigh(isSilent);_delay_us(800);
		SoundLow(isSilent);_delay_us(800);		
		SoundHigh(isSilent);_delay_us(600);
		SoundLow(isSilent);_delay_us(600);		
		SoundHigh(isSilent);_delay_us(500);
		SoundLow(isSilent);_delay_us(500);		
		SoundHigh(isSilent);_delay_us(500);
		SoundLow(isSilent);_delay_us(500);		
		SoundHigh(isSilent);_delay_us(600);
		SoundLow(isSilent);_delay_us(600);		
		SoundHigh(isSilent);_delay_us(800);
		SoundLow(isSilent);_delay_us(800);		
		SoundHigh(isSilent);_delay_us(1000);
		SoundLow(isSilent);_delay_us(1000);		
		SoundHigh(isSilent);_delay_us(1200);
		SoundLow(isSilent);_delay_us(1200);		
		SoundHigh(isSilent);_delay_us(1400);
		SoundLow(isSilent);_delay_us(1400);		
		SoundHigh(isSilent);_delay_us(2000);
		SoundLow(isSilent);_delay_us(2000);
	#endif


}
