/*
 * Synth.h
 *
 * Created: 27.12.2015 22:58:32
 *  Author: Андрей
 */ 


#ifndef SYNTH_H_
#define SYNTH_H_

// Управление динамиком сирены
#define BUZZER_DDR  DDRB
#define BUZZER_PORT PORTB
#define BUZZER_PIN1	1
#define BUZZER_PIN2	2

// Звуковые эффекты
#define SOUND_NONE 0
#define SOUND_SHORT 1
#define SOUND_LONG 2
#define SOUND_HORN 3
#define SOUND_ALARM 5
#define SOUND_WARNING 4

void SoundInit();
void SoundOff();
void Sound1(uint8_t isSilent);
void Sound2(uint8_t isSilent);
void Horn(uint8_t isSilent);
void Warning();
void Alarm();

#endif /* SYNTH_H_ */