/*
 * Fuse_CTRL.c
 *
 * Created: 02.04.2021 16:10:47
 *  Author: lukas
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Fuse_CTRL.h"


uint8_t DCDC_in_Fuse = 0;
uint8_t Servo1_Fuse = 0;
uint8_t Servo2_Fuse = 0;
uint8_t Servo3_Fuse = 0;
uint8_t Emu_Fuse = 0;
uint8_t Brakelight_Fuse = 0;
uint8_t FAN1_Fuse = 0;
uint8_t FAN2_Fuse = 0;
uint8_t Ignition_Fuse = 0;
uint8_t Injection_Fuse = 0;
uint8_t Fuelpump_Fuse = 0;
uint8_t Datalogger_Fuse = 0;
uint8_t Controller_Fuse = 0;

uint16_t get_fuse_status(){
	
	//shift to only get the fuse bit from all the bits we get from calling the register [fuse location]
	//PINA = 0b0100[1]100 << 4
	//0b11000000 >> 7
	//0b00000001
	
	DCDC_in_Fuse =		(PINA & 0b00001000)>>3;
	Servo1_Fuse =		(PINA & 0b00010000)>>4;
	Servo2_Fuse =		(PINA & 0b00100000)>>5;
	Servo3_Fuse =		(PINA & 0b01000000)>>6;
	Emu_Fuse =			(PINA & 0b10000000)>>7;
	
	Brakelight_Fuse =	(PINE & 0b00000001);
	FAN1_Fuse =			(PINE & 0b00000010)>>1;
	FAN2_Fuse =			(PINE & 0b00000100)>>2;
	Ignition_Fuse =		(PINE & 0b01000000)>>6;
	Injection_Fuse =	(PINE & 0b10000000)>>7;
	
	Fuelpump_Fuse =		(PIND & 0b00000001);
	Datalogger_Fuse =	(PIND & 0b00000010)>>1;
	Controller_Fuse =	(PIND & 0b00000100)>>2;
	
	//package all the data into a 16bit integer for sending over CAN
	uint16_t Fuses=0;
	Fuses |= (DCDC_in_Fuse) | (Servo1_Fuse<<1) | (Servo2_Fuse<<2) | (Servo3_Fuse<<3) | (Emu_Fuse<<4) | (FAN1_Fuse<<5) | (FAN2_Fuse<<6) | (Ignition_Fuse<<7) | (Ignition_Fuse<<8) | (Injection_Fuse<<9) | (Fuelpump_Fuse<<10) | (Datalogger_Fuse<<11) | (Controller_Fuse<<12);
	return Fuses;
	}