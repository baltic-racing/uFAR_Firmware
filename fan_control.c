/*
 * fan_control.c
 *
 * Created: 02.04.2021 15:35:56
 *  Author: lukas
 */ 
#include "fan_control.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "servo_functions.h"

volatile uint16_t fan_time = 1800;

void fan_speed_control(uint8_t temperature, uint16_t rpm){
	
	if(temperature > CLT_MAX){
		temperature = CLT_MAX;
	}
	
	if(rpm > 750 && temperature >= CLT_MIN){
		fan_time = calculate_Servo_ticks(26+(SERVO_MAXANGLE-FAN_MIN)/(CLT_MAX-CLT_MIN)*(temperature-CLT_MIN));
	}
	else {
		fan_time = 1800;
	}
	
}