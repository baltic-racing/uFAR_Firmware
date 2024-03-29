/*
 * servo_functions.h
 *
 * Created: 05.04.2019 19:24:07
 *  Author: Ole Hannemann
 */ 


#ifndef SERVO_FUNCTIONS_H_
#define SERVO_FUNCTIONS_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define TRUE 1
#define FALSE 0

//Clutch servo config
#define CLUTCH_MAX_ANGLE 120 //�
#define BP_MIN 15 //Bar
#define BP_RELEASE 3 //BAR
#define CLUTCH_SLIP_ANGLE 48 //�
#define CLUTCH_SPEED_LAUNCH 4//100ms
#define TWO_STEP_OFFSET 600
#define FIRST_GEAR_SHIFT_SPEED 43

//Shifting servo config
#define SERVO_MAXANGLE 120 //�
#define GEAR_SERVO_MIDDLE_ANGLE 70 //75 TY20
#define GEAR_SERVO_SHIFT_UP_ANGLE 80 //80 65 TY20
#define GEAR_SERVO_SHIFT_DOWN_ANGLE 65 //65 TY20
#define GEAR_SERVO_SHIFT_NEUTRAL_ANGLE 65 //65 TY20 

#define Savox 1
#define JXC 0


#define SHIFT_DEG_OFFSET 0
//this locktime is to prevent another shifting action before this set time in ms expires
#define LOCKTIME_SHIFT 400 //400
//time in ms before the flatshift and Antiblip is activated
#define FLATSHIT_OFFSET 100 //100
//time in ms before the Blipper is activated
#define BLIPPER_OFFSET 20 //40
 //Time in ms before the antiblipper is activated
#define ANTI_BLIP_OFFSET 120 //100

//Time in ms for the servo to move to the shift up position and hold
#define SHIFT_DURATION_UP (uint16_t) 180	// 120 //Load: 260	//Idle: 250

#define SHIFT_DURATION_UP_EINS (uint16_t) SHIFT_DURATION_UP	//Load: 250	//Idle: 250
//Time in ms for the servo to move to the shift down position and hold
#define SHIFT_DURATION_DOWN (uint16_t) 200		// 200 //Load: 250	//Idle: 200
//Time in ms for the servo to move back to the middle position and hold
#define SHIFT_DURATION_MID (uint16_t) 100		// 100 //Old 200
//Time in ms for the servo to move to the shift Neutral position and hold
#define SHIFT_DURATION_NEUTRAL (uint16_t) 100	//100	/*300*/

//offset for the exact shift position in ticks

#define SHIFT_UP_RPM 9800
#define SHIFT_UP_RPM_EINS 8000
#define AUTO_SHIFTLOCK_TIME 600
#define TWOSTEP_RPM 6000

#define UP 1
#define DOWN 0

#define FLATSHIFT_PORT PORTB
#define FLATSHIFT_PIN PB2

#define SERVO_SHIFT_PORT PORTB
#define SERVO_SHIFT_PIN PB4
#define SERVO_CLUTCH_PORT PORTB
#define SERVO_CLUTCH_PIN PB5


//NEEDS a unsigned long called Sys_time

uint16_t calculate_Servo_ticks(double deg, uint8_t Servo_type);
void calculate_general_ticks(void);
void shift_control(uint8_t shift_up, uint8_t shift_down, uint8_t gear, uint16_t rpm, uint8_t LC_Active, double GPS_Speed);
void servo_timer_config();
//this functions excepts to be called every 10ms
void calculate_locktimes();

ISR(TIMER1_COMPA_vect);


#endif /* SERVO_FUNCTIONS_H_ */