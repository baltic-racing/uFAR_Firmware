/*
 * fan_control.h
 *
 * Created: 08.06.2019 17:51:19
 *  Author: Konstruktion
 */ 


#ifndef FAN_CONTROL_H_
#define FAN_CONTROL_H_

#define FAN1_DDR	DDRE
#define FAN1_PORT	PORTE
#define FAN1_PIN	PE3

#define FAN2_DDR	DDRE
#define FAN2_PORT	PORTE
#define FAN2_PIN	PE4
#define FAN_MIN		26
#define CLT_MAX		105
#define CLT_MIN		60

void fan_speed_control(uint8_t temperature, uint16_t engine_running);

#endif /* FAN_CONTROL_H_ */