/*uint8_t
 * misc_functions.c
 *
 * Created: 05.04.2019 18:51:39
 *  Author: Ole Hannemann
 * last change: 21.02.2021
 *  Author: Lukas Deeken
 */ 

#include "misc_functions.h"
#include "servo_functions.h"

volatile unsigned long sys_time = 0;
uint8_t on_time;
unsigned long systime_PWM = 0;
uint8_t PWM_on = 0;
uint16_t esum = 0;
extern uint8_t Flatshift_Active;
extern uint16_t rpm;

extern uint8_t gear;
extern uint8_t LC_State;
extern uint8_t APPS1;
extern uint8_t APPS2;
extern double BP;

extern uint8_t swc_databytes[8];

#define SHIFT_UP swc_databytes[3]
#define SHIFT_DOWN swc_databytes[2]
#define BUTTON_LEFT swc_databytes[4]
#define BUTTON_RIGHT swc_databytes[5]
#define LEFT_ENCODER swc_databytes[1]
#define RIGHT_ENCODER swc_databytes[0]

void port_config(){
	DDRA = 0 | (1<<PA0);									//All either generic Digital Inputs or Fuse Inputs Mark PA0 as Flatshift out
	DDRB = 0 | (1<<PB4) | (1<<PB5) | (1<<PB2) | (1<<PB1);	//Mark PB1, PB2, PB4 and PB5 as output (Fuelpump_CTRL, Shiftsensor, Clutchsensor)
	DDRC = 0 | (1<<PC0) | (1<<PC2);							//PC0 and PC2 as Output for Heart and Foult LED otherwise unused
	DDRD = 0 | (1<<PD0) ;												//Either Fuse or can so all Input
	DDRE = 0 | (1<<PE3) | (1<<PE4);							//PE3 and PE4 as FAN_CTRL outputs otherwise Fuse Inputs
	DDRF = 0;												//mark analog inputs on Port F as input
}

void sys_tick(){
	PORTD ^= 1<<PD0; //toggling the Status of PC2 to make the Heart LED blink every SYStick
}

void sys_timer_config(){
	
	//8 bit Timer 0 configuration
	//ctc mode and 32 as prescaler for 8Mhz Quartz, for 16Mhz use 64
	TCCR0A = 0 | (1<<WGM01) | (1<<CS01) | (1<<CS00);
	TIMSK0 = 0 | (1<<OCF0A);	//compare interrupt enable
	OCR0A = 250-1;				// compare value for 1ms;
	
}

void fuelpump_CTRL(uint16_t rpm){
	//PORTB |= (1<<PB1); //turn on PB1 where the FuelpumpCTRL is connected to to activate the Fuelpump when Engine RPM is high enough
	/*if (rpm >= 350)
	{
		PORTB |= (1<<PB1); //turn on PB1 where the FuelpumpCTRL is connected to to activate the Fuelpump when Engine RPM is high enough
	}
	else{
		PORTB &= ~(1<<PB1);
	}*/
}

//ISR for Timer 0 compare interrupt
ISR(TIMER0_COMP_vect){
	//1ms loop
	sys_time++; //system time generation
	//for every time the timer equals 249 an interrupt is generated resulting in invreasing the SYStime
	
	clutch_control(BUTTON_RIGHT|BUTTON_LEFT,LEFT_ENCODER+1,gear,LC_State,APPS1,APPS2,BP);
	
}
