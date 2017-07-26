
// Created: 18.09.2015 22:11:14
// Author: Андрей

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "Controller_Alarm.h"
#include "millis.h"
#include "RGBLed.h"
#include "Synth.h"
#include "uart.h"
#include "mpu6050.h"
#include "i2cmaster.h"

volatile uint8_t ModeButton = 0;				// Положение ключа-переключателя режимов
volatile uint8_t IsSilent = 0;					// Громкость звуков (громко-тихо)
volatile uint8_t AlarmStatus = ALARM_NONE;		// Сигнализация выключена
volatile uint8_t CurrentMode = MODE_DRIVE;		// Режим работы: движение
volatile uint8_t CurrentSound = SOUND_NONE;		// Звук сирены отключен
volatile int WakeUpTimes = 0;					// Число успешных выходов из сна
float CurrentVoltage = 0;						// Напряжение на шине Vcc
volatile uint8_t PowerStatus = POWER_CHARGING;	// Текущее состояние шины питания
volatile t_RGBColor LEDColor = COLOR_NONE;		// Текущее состояние шины питания
volatile uint8_t _adcsra = 0;

// Запуск сканирования прерываний по смене состояния кнопок управления сиреной
void SoundINT_Enable()
{
	PCIFR |= 1<<PCIF0 | 1<<PCIF1 | 1<<PCIF2;
	PCICR |= 1<<BUTTON_INT_GROUP;	
	sei();
}

// Отключение скана прерываний по смене состояний кнопок управления сиреной	
void SoundINT_Disable()
{
	PCICR &= ~(1 << BUTTON_INT_GROUP);
}

// Включить питание датчиков	
void Sensor_Enable()
{
	GY_SENSOR_POWER_PORT |= (1 << GY_SENSOR_POWER_PIN);
}

// Отключение питания датчиков положения и удара	
void Sensor_Disable()
{
	GY_SENSOR_POWER_PORT &= ~(1 << GY_SENSOR_POWER_PIN);
}

// Отключить сигнализацию
void AlarmPowerOff()
{
	Sensor_Disable();
	SoundINT_Enable();
}

// Включить сигнализацию
void AlarmPowerOn()
{
	SoundINT_Disable();
	Sensor_Enable();
	while(!mpu6050_testConnection())						// Инициализация датчика
	{
		mpu6050_init();
		_delay_ms(250);
	}
}

void PeripheryDisable() 
{
	RGB_Disable();						// Отключить статусный RGB для экономии энергии	
	Sensor_Disable();					// Отключить питание датчика положения
	
	#ifdef DEBUG_MODE
		uart_disable();					// Отключить UART
	#endif 
	
	CurrentMode = !!(BUTTON_MODE_PIN & 1<<BUTTON_MODE);			// Получение состояния переключателя режимов		

	// Текущий режим - охрана
	if(CurrentMode == MODE_SECURITY) 
	{
		SoundINT_Disable();
	} 

	// Текущий режим - езда
	else if(CurrentMode == MODE_DRIVE)
	{
		SoundINT_Enable();
	}

}

void PeripheryEnable()
{
	RGB_Init();												// Подключить RGB

	#ifdef DEBUG_MODE
		uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));	// Включить UART
	#endif 

	CurrentMode = !!(BUTTON_MODE_PIN & 1<<BUTTON_MODE);		// Получение состояния переключателя режимов	

	// Текущий режим - езда
	if(CurrentMode == MODE_DRIVE)
	{
		AlarmPowerOff();
	} 

	// Текущий режим - охрана
	else if(CurrentMode == MODE_SECURITY) 
	{
		AlarmPowerOn();							// Включение питание и инициализация датчиков, отключения скана прерываний по кнопкам управления сиреной
	}	
}

void DisableWatchDog()
{
	MCUSR = 0;	
	wdt_disable();
}

// Получение значения напряжения питания контроллера
float GetVoltage()
{
	// Настройка АЦП
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);		// Включить АЦП, предделитель - 128
	ADMUX = (1<<REFS0) | 1<<MUX3 | 1<<MUX2 | 1<<MUX1;			// Vcc - опорное напряжение,  Вход - внутренний источник 1.1В
	ADCSRA |= 1<<ADSC;											// Запуск измерения,

	while (ADCSRA & (1<<ADSC));			// Ожидание окончания измерения
	uint8_t low  = ADCL;				// младший байт
	uint8_t high = ADCH;				// старший байт
	float result = (high<<8) | low;		// полное значение АЦП
	result = 1125.3 / result;			// Расчет Vcc, В; 1125.3 = 1.1*1023

	return result;
}

// Читаем значение АЦП для регуляторов чувствительности датчиков
uint16_t ReadADC(uint8_t _SensorPin)
{
	uint8_t prescaler = 1;

	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	
	ADMUX = _SensorPin;
	ADMUX |= 1 << REFS0;				// Vcc - опорное значение
	if(_SensorPin == SENS_GYRO_PIN)
	{
		prescaler = GYRO_PRESCALER;
	}
	else if (_SensorPin == SENS_ACCE_PIN)
	{
		prescaler = ACCELEROMETER_PRESCALER;
	}
	
	ADCSRA |= (1<<ADSC);	    	// ADCSRA[6]: старт преобразования
	while(ADCSRA & (1<<ADSC));		// дожидаемся окончания преобразования

	uint16_t result = ADCL;
	uint16_t high = ADCH;
	result = result + (high<<8);
	
	return (result / prescaler);
}

void GoToSleep(uint8_t _OperationMode) 
{		
	_adcsra = ADCSRA;	
	ADCSRA = 0;
	PeripheryDisable();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();

	power_all_disable();
	sleep_bod_disable();						// Последовательность для отключения BOD на время сна
	
	// Просыпаться по вачдогу в режиме охраны будем безусловно.
	if(_OperationMode == MODE_SECURITY)
	{
		MCUSR = 0;								// Очистить все флаги источников сброса
		wdt_enable(WDTO_1S);					// Раз в секунду пёс будит всю деревню
		WDTCSR |= 1<<WDIE;						// разрешаем прерывания по ватчдогу. Иначе будет резет.		
	} 
	else if (_OperationMode == MODE_DRIVE)
	{
		// В режиме езды - только если просыпались меньше, чем MAX_WAKEUP_TIMES
		if(WakeUpTimes < MAX_WAKEUP_TIMES)
		{
			MCUSR = 0;
			wdt_enable(WDTO_4S);
			WDTCSR |= 1<<WDIE;
		} 

		// Если просыпались уже больше, чем MAX_WAKEUP_TIMES - засыпаем без вачдога и просыпаемся только по прерыванию с кнопок
		else
		{
			SoundINT_Enable();
			DisableWatchDog();		
		}

	}
	sei();		

	sleep_cpu();
	sleep_disable();

	ADCSRA = _adcsra;
	power_all_enable();
	PeripheryEnable();
	CurrentVoltage = GetVoltage();	
	WakeUpTimes++;
}

// Подготовка необходимого железа
void HardwareSetup()
{
	// Ключ отключения нагрузки
	GY_SENSOR_POWER_DDR |= 1 << GY_SENSOR_POWER_PIN;	// пин на выход
	
	// Настройка статусного RGB-светодиода (с общим анодом)
	RGB_Init();							// Настройка портов
	RGB_Blink(COLOR_NONE, SPEED_NONE);	// Не горит
	
	// Настройка ключа управления режимами работы
	BUTTON_MODE_DDR &= ~(1<<BUTTON_MODE);	// вход
	//BUTTON_MODE_PORT |= 1<<BUTTON_MODE;	// подтяжка

	// Настройка прерываний для ключа режимов
	EICRA |= (1 << ISC10);					// прерывание срабатывает по любому изменению состояния порта
	EIMSK |= (1 << BUTTON_MODE_INT);		// включить прерывание на указанном порту

	// Кнопки управления сиреной
    BUTTON_DDR &= ~(1<<BUTTON_HORN | 1<<BUTTON_SOUND1 | 1<<BUTTON_SOUND2 | 1<<BUTTON_LOUD);		// пины на вход
	//BUTTON_PORT |= 1<<BUTTON_HORN | 1<<BUTTON_SOUND1 | 1<<BUTTON_SOUND2;						// подтягивающие резисторы
	
	// Настройка прерываний для кнопок управления сиреной
	PCICR |= 1 << BUTTON_INT_GROUP;				// Старт сканирования прерываний нужного регистра PCMSKx (PCINTx-PCINTx)
	BUTTON_PCKMS_REG |=  (1 << BUTTON_HORN_INT | 1 << BUTTON_SOUND1_INT | 1 << BUTTON_SOUND2_INT); /* | 1 << BUTTON_LOUD_INT);*/	// Включаем прерывания по смене состояния соответствующих пинов

	// Настройка счетчика миллисекунд
	millis_init();
		
	// Настройка параметров генератора звуков
	SoundInit();
	
	// Настройка uart. Только в режиме отладки
	#ifdef DEBUG_MODE
		uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));
	#endif

	sei();				// Включить прерывания
}

// Проверка состояния кнопок управление сиреной. Установка режима звучания
uint8_t CheckSignalButtons(void) 
{
	uint8_t _Sound = SOUND_NONE;
	if ((BUTTON_PIN & 1<<BUTTON_HORN))
	{
		_Sound = SOUND_HORN;
		#ifdef DEBUG_MODE
			uart_puts("Horn pressed!\r\n");
		#endif
	} 
	else if ((BUTTON_PIN & 1<<BUTTON_SOUND1))
	{
		_Sound = SOUND_SHORT;
		#ifdef DEBUG_MODE
			uart_puts("Sound1 pressed!\r\n");
		#endif
	} else if ((BUTTON_PIN & 1<<BUTTON_SOUND2))
	{
		_Sound = SOUND_LONG;
		#ifdef DEBUG_MODE
			uart_puts("Sound2 pressed!\r\n");
		#endif
	}
	
	return _Sound;
}

/*
 * Прерывание по вачдогу - отключаем вачдог
 */
ISR (WDT_vect)
{
	DisableWatchDog();
}

/*
 * Прерывание по нажатию на одну из кнопок управления сиреной
 */
ISR (BUTTON_INT_VECT) 
{
	DisableWatchDog();		
	CurrentSound = CheckSignalButtons();
	WakeUpTimes = 0;
}

/*
 * Прерывание по изменению положения ключа режимов работы
 */
ISR(BUTTON_MODE_VECTOR) 
{
	DisableWatchDog();
	ModeButton = !!(BUTTON_MODE_PIN & 1<<BUTTON_MODE);
	WakeUpTimes = 0;
}

/*
 * Прерывание по изменению громкости звуков
 */

int main()
{
	uint16_t accThreshold = 20;
	uint16_t gyrThreshold = 50;
	
	uint8_t AlarmInitialized = 0;

	double axg0 = 0;
	double ayg0 = 0;
	double azg0 = 0;
	double axg = 0;
	double ayg = 0;
	double azg = 0;
	double gxds = 0;
	double gyds = 0;
	double gzds = 0;


	#ifdef DEBUG_MODE
		char charBuffer[10];
	#endif
	
	HardwareSetup();
	
	while (1)
	{
		DisableWatchDog();
		
		ModeButton = !!(BUTTON_MODE_PIN & 1<<BUTTON_MODE);		// Получение состояния переключателя режимов	
		CurrentVoltage = GetVoltage();							// Измерение напряжения Vcc и установка соответствующего режима питания		
		if(CurrentVoltage >= 4)
		{
			PowerStatus = POWER_CHARGING;
			LEDColor = COLOR_VIOLET;
			WakeUpTimes = 0;
		}
		else if (CurrentVoltage > 3.7 && CurrentVoltage < 4)
		{
			PowerStatus = POWER_BATTERY_FULL;
			LEDColor = COLOR_GREEN;
		}
		else if (CurrentVoltage > 3.5 && CurrentVoltage <= 3.7)
		{
			PowerStatus = POWER_BATTERY_HALF;
			LEDColor = COLOR_BLUE;
		}
		else if (CurrentVoltage <= 3.5)
		{
			PowerStatus = POWER_BATTERY_LOW;
			LEDColor = COLOR_RED;
		}
		
		#ifdef DEBUG_MODE
			uart_puts("Vcc: "); dtostrf(CurrentVoltage, 3, 1, charBuffer); uart_puts(charBuffer);
			uart_puts(" POWER: "); dtostrf(PowerStatus, 1, 0, charBuffer); uart_puts(charBuffer);
			uart_puts(" Wake: "); dtostrf(WakeUpTimes, 1, 0, charBuffer); uart_puts(charBuffer);
			uart_puts(" ACC: "); itoa(accThreshold, charBuffer, 10); uart_puts(charBuffer);
			uart_puts(" GYR: "); itoa(gyrThreshold, charBuffer, 10); uart_puts(charBuffer);uart_putc('\n');
		#endif
					

		// Режим поездки - сигализация отключена	
		if (ModeButton == 0) 
		{
			if(CurrentMode == MODE_SECURITY) 
			{
				AlarmPowerOff();
				AlarmInitialized = 0;
				
				#ifdef DEBUG_MODE
					uart_puts("\r\nDRIVE MODE SELECTED\r\n");
				#endif
			}
			CurrentMode = MODE_DRIVE;
			CurrentSound = CheckSignalButtons();								
						
			// Звуковые сигналы
			switch(CurrentSound)
			{
				case SOUND_SHORT:
					Sound1(0);
				break;

				case SOUND_LONG:
					Sound2(0);
				break;
				
				case SOUND_HORN:
					Horn(0);
				break;
				
				case SOUND_NONE:
				default:
					SoundOff();
				break;
			}
			
			if(CurrentSound == SOUND_NONE)
			{				
				RGB_Blink(LEDColor, SPEED_NONE);
				if(PowerStatus > POWER_CHARGING)
				{
					_delay_ms(50);
					GoToSleep(MODE_DRIVE);					
				}
			} 
					
		} 

		// Режим охраны - сигализация включена
		else if (ModeButton == 1) 
		{			
			// Получение начального положения датчиков - после первой активации или после срабатывания сигнала
			if(CurrentMode == MODE_DRIVE || (CurrentMode == MODE_SECURITY && AlarmStatus == ALARM_SIREN)) 
			{							
				// Сигнализацию активированна - включается датчик удара и наклона
				if(CurrentMode == MODE_DRIVE)
				{
					#ifdef DEBUG_MODE
						uart_puts("\r\nSECURITY MODE SELECTED\r\n");
					#endif
					AlarmPowerOn();
				}
				
				CurrentMode = MODE_SECURITY;				
										
				#ifdef DEBUG_MODE
					int AlarmPowerUpDelay = ALARM_DELAY_BEFORE_START * 1000 / 7;		// Задержка перед считыванием начального положения, чтобы можно было вынуть ключ. RGB-радуга
					int _color = 0;
					uart_puts("Getting initial position...\r\n");
					for(_color = 1; _color < 8; _color++)
					{
						RGB_Blink(_color, SPEED_NONE);
						_delay_ms(AlarmPowerUpDelay);
					}
				#else
					RGB_Blink(COLOR_RED, SPEED_NONE);
					_delay_ms(ALARM_DELAY_BEFORE_START * 1000);
					RGB_Blink(COLOR_NONE, SPEED_NONE);
				#endif

			
				mpu6050_getConvData(&axg0, &ayg0, &azg0, &gxds, &gyds, &gzds);	// Начальное положение датчиков
				axg0 *= RAD_TO_DEG;	ayg0 *= RAD_TO_DEG;	azg0 *= RAD_TO_DEG;		// Перевод радиан в градусы
				
				AlarmInitialized = 1;
				
				#ifdef DEBUG_MODE
					uart_puts("axg0: "); dtostrf(axg0, 3, 1, charBuffer); uart_puts(charBuffer); uart_putc(' ');
					uart_puts("ayg0: "); dtostrf(ayg0, 3, 1, charBuffer); uart_puts(charBuffer); uart_putc(' ');
					uart_puts("azg0: "); dtostrf(azg0, 3, 1, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				#endif
			}
							
			RGB_Blink(COLOR_NAVY, SPEED_NONE);

			CurrentMode = MODE_SECURITY;
			AlarmStatus = ALARM_NONE;

			mpu6050_getConvData(&axg, &ayg, &azg, &gxds, &gyds, &gzds);		// Текущие значения датчиков		
			axg *= RAD_TO_DEG;	ayg *= RAD_TO_DEG;	azg *= RAD_TO_DEG;		// Преобразование радиан в градусы
						
			// Чтение значения чувствительности датчиков
			//accThreshold = ReadADC(SENS_ACCE_PIN);		// макс. значение покоя > 64
			//gyrThreshold = ReadADC(SENS_GYRO_PIN) + 100;		// макс. значение покоя > 41
	
			#ifdef DEBUG_MODE
				uart_puts("\r\n\r\n------------------------------------------\r\n");	
				//uart_puts("ADC: ");	itoa(adcValue, charBuffer, 10);	uart_puts(charBuffer);
				uart_puts(" ACC: "); itoa(accThreshold, charBuffer, 10); uart_puts(charBuffer);
				uart_puts(" GYR: "); itoa(gyrThreshold, charBuffer, 10); uart_puts(charBuffer);
				uart_puts("\r\n");	
				uart_puts("AX: "); dtostrf(axg, 3, 1, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				uart_puts("AY: "); dtostrf(ayg, 3, 1, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				uart_puts("AZ: "); dtostrf(azg, 3, 1, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				uart_puts("\r\n");
				uart_puts("dAX: "); dtostrf(abs(axg0 - axg), 3, 1, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				uart_puts("dAY: "); dtostrf(abs(ayg0 - ayg), 3, 1, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				uart_puts("Delta azg: "); dtostrf(abs(azg0 - azg), 3, 1, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				uart_puts("\r\n");
				uart_puts("GX: "); dtostrf(gxds, 3, 2, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				uart_puts("GY: "); dtostrf(gyds, 3, 2, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				uart_puts("GZ: "); dtostrf(gzds, 3, 2, charBuffer); uart_puts(charBuffer); uart_putc(' ');
				uart_puts("\r\n");
			#endif
			
			if(AlarmInitialized == 1)
			{
				// Проверка значений акселерометра - сигнал "Тревога"
				if(accThreshold < abs(axg0 - axg) || accThreshold < abs(ayg0 - ayg) || accThreshold < abs(azg0 - azg))
				{
					#ifdef DEBUG_MODE
					uart_puts("\r\nAlarm!!!\r\n");
					#endif
				
					AlarmStatus = ALARM_SIREN;
					RGB_Blink(COLOR_RED, SPEED_NONE);
					Alarm();
				}

				// Проверка значений гироскопа - сигнал "Предупреждение"
				if((gyrThreshold < abs(gxds) || gyrThreshold < abs(gyds) || gyrThreshold < abs(gzds)) && AlarmStatus != ALARM_SIREN)
				{
					#ifdef DEBUG_MODE
					uart_puts("\r\nWarning!!!\r\n");
					#endif

					AlarmStatus = ALARM_WARNING;
					RGB_Blink(COLOR_YELLOW, SPEED_NONE);
					Warning();
				}				
			}


			if(AlarmStatus == ALARM_NONE && PowerStatus > POWER_CHARGING)
			{
				_delay_ms(100);
				GoToSleep(MODE_SECURITY);
			}				
		}
				

	}

		
}
