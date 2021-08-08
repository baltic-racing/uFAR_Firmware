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
#define CLUTCH_MAX_ANGLE 120 //°
#define BP_MIN 15 //Bar
#define BP_RELEASE 5 //BAR
#define CLUTCH_SLIP_ANGLE 90 //°
#define CLUTCH_SPEED_LAUNCH 2//seconds

//Shifting servo config
#define SERVO_MAXANGLE 130 //°
#define GEAR_SERVO_MIDDLE_ANGLE 62 //75 TY20
#define GEAR_SERVO_SHIFT_UP_ANGLE 68 //65 TY20
#define GEAR_SERVO_SHIFT_DOWN_ANGLE 62 //65 TY20
#define GEAR_SERVO_SHIFT_NEUTRAL_ANGLE 62 //65 TY20 

#define SHIFT_DEG_OFFSET 0
//this locktime is to prevent another shifting action before this set time in ms expires
#define LOCKTIME_SHIFT 300 //500
//time in ms before the flatshift and Antiblip is activated
#define FLATSHIT_OFFSET 90 //220
//time in ms before the Blipper is activated
#define BLIPPER_OFFSET 50 //220

#define SHIFT_DURATION_UP (uint16_t) 150//350
#define SHIFT_DURATION_DOWN (uint16_t) 150 //350
#define SHIFT_DURATION_MID (uint16_t) 150 //300
#define SHIFT_DURATION_NEUTRAL (uint16_t) 150 //300
//offset for the exact shift position in ticks

#define SHIFT_UP_RPM 9500

#define UP 1
#define DOWN 0

#define FLATSHIFT_PORT PORTB
#define FLATSHIFT_PIN PB2

#define SERVO_SHIFT_PORT PORTB
#define SERVO_SHIFT_PIN PB4
#define SERVO_CLUTCH_PORT PORTB
#define SERVO_CLUTCH_PIN PB5


//NEEDS a unsigned long called Sys_time

uint16_t calculate_Servo_ticks(double deg);
void calculate_general_ticks(void);
void shift_control(uint8_t shift_up, uint8_t shift_down, uint8_t gear, uint16_t rpm);
void clutch_control(uint8_t clutch, uint8_t clutch_speed, uint8_t gear, uint8_t LC_Active, uint16_t APPS1, uint16_t APPS2, uint16_t BP );
void servo_timer_config();
//this functions excepts to be called every 10ms
void calculate_locktimes();

ISR(TIMER1_COMPA_vect);


#endif /* SERVO_FUNCTIONS_H_ */