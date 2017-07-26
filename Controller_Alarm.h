/*
 * AvrSiren_Police.h
 *
 * Created: 11.12.2015 19:15:52
 *  Author: Андрей
 */ 


#ifndef AVRSIREN_POLICE_H_
#define AVRSIREN_POLICE_H_

//#define DEBUG_MODE

#define UART_BAUD_RATE 57600
#define RAD_TO_DEG 57.3
#define MAX_WAKEUP_TIMES 10

/*
 * Состояния батареи и генератора
 * Vcc > 4.2В - Питание от генератора + зарядка
 * 3.7 <= Vcc <= 4.2 - Батарея заряжена
 * 3.1 < Vcc < 3.7 - Батарея разряжается
 * Vcc < 3.1В - Батарея разряжена
 */
#define POWER_CHARGING 0			// питание от генератора
#define POWER_BATTERY_FULL 1		// питание от батареи - батарея заряжена
#define POWER_BATTERY_HALF 2		// питание от батареи - батарея разряжается
#define POWER_BATTERY_LOW 3			// питание от батареи - батарея разряжена

// Параметры регуляторов чувствительности гироскопа и акселерометра
#define GYRO_PRESCALER 1			// Делитель для гироскопа
#define ACCELEROMETER_PRESCALER 1	// Делитель для акселерометра
#define SENS_GYRO_PIN 6				// Канал АЦП гироскопа
#define SENS_ACCE_PIN 7				// Канал АЦП акселерометра

// Режимы работы устройства
#define MODE_DRIVE 0				// Режим движения. Сигнализация выключена
#define MODE_SECURITY 1				// Режим стоянки. Сигнализация включена

// Управление отключаемой шиной питания для датчиков (ток не более 15mA)
#define GY_SENSOR_POWER_DDR	 DDRD
#define GY_SENSOR_POWER_PORT PORTD
#define GY_SENSOR_POWER_PIN	 2		// Ключевой пин управления

// Ключ переключения режимов работы
#define BUTTON_MODE_DDR DDRD
#define BUTTON_MODE_PORT PORTD
#define BUTTON_MODE_PIN PIND
#define BUTTON_MODE 3			
#define BUTTON_MODE_INT INT1
#define BUTTON_MODE_VECTOR INT1_vect	

// Кнопка громкости

// Управление сиреной 
#define BUTTON_PORT  PORTC			
#define BUTTON_PIN   PINC			
#define BUTTON_DDR   DDRC			
#define BUTTON_PCKMS_REG PCMSK1		
#define BUTTON_INT_GROUP PCIE1		
#define BUTTON_INT_VECT PCINT0_vect	
#define BUTTON_LOUD 0
#define BUTTON_LOUD_INT PCINT8
#define BUTTON_HORN 1				
#define BUTTON_HORN_INT PCINT9		
#define BUTTON_SOUND1 2				
#define BUTTON_SOUND1_INT PCINT10	
#define BUTTON_SOUND2 3				
#define BUTTON_SOUND2_INT PCINT11	

#define ALARM_DELAY_BEFORE_START 1	// Задержка в секундах до включения сигнализации

// Режимы сигнализации
#define ALARM_NONE 0		// выключена
#define ALARM_WARNING 1		// предупреждение
#define ALARM_SIREN 2		// сирена

#endif /* AVRSIREN_POLICE_H_ */