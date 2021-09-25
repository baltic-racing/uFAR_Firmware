/*
 * servo_functions.c
 *
 * Created: 05.04.2019 19:24:26
 *  Author: Ole Hannemann
 */ 

#include "servo_functions.h"
#include "fan_control.h"

volatile uint8_t servo_active = 0;

//time until the servos get deactivated
volatile uint8_t servo_locktime_gear = 0;
volatile uint8_t servo_locktime_clutch = 0;
volatile uint8_t shiftlock = FALSE;
volatile uint16_t Auto_shiftlock = 0;
//locktime for a new shifttime
volatile uint8_t shift = 0;
volatile uint8_t shift_indicator = 0;
volatile uint16_t shift_locktime = 0;
volatile uint8_t shift_complete = 0;
volatile unsigned long sys_time_launch = 0;

//shift_time are the ticks for the timer interrupt
//time_* are the ticks for the desired position
volatile uint16_t shift_time = 0;
volatile uint16_t time_neutral = 0;
volatile uint16_t time_down = 0;
volatile uint16_t time_up = 0;
volatile uint16_t time_mid = 0;

volatile uint8_t gear_desired = 0;
volatile uint8_t deg_set = FALSE;

volatile uint8_t Blipper_Enable;
volatile uint8_t Anti_Blipper_Enable;
volatile uint8_t LC_Ready = FALSE;
volatile uint8_t LC_Launch = FALSE;
volatile uint8_t EMULC_Active = FALSE;


uint8_t Launch_Flatshift_Active = 0;
uint8_t Shift_Flatshift_Active = 0;

extern unsigned long sys_time;
volatile unsigned long time_shift_started = 0;
volatile uint16_t shift_duration_current = 0;

//vars needed for the clutch_control
volatile uint16_t clutch_period = 0;
volatile double clutch_angle = 0;
//var where the ticks for the clutch is stored
//defaults to minimal position
volatile double pitch = 0;
volatile uint16_t clutch_time = 1800;

volatile uint8_t calculated_ticks = FALSE;

extern volatile int16_t fan_time;
extern double BP;

uint8_t gear_up_autmatic = 0;

void servo_timer_config(){
	
	//CONFIG FOR THE SERVO CONTROL
	//USING TIMER 1 COMPARE A INTERRUPT
	//16 bit Timer 1 config
	//CTC mode and a prescaler of 8
	TCCR1B |= (1<<CS11) | (1<<WGM12);
	TIMSK1 |= (1<<OCIE1A);
	
	if (calculated_ticks == FALSE){
		calculate_general_ticks();		
	}
}

void calculate_general_ticks(void){
	
	time_up = calculate_Servo_ticks(GEAR_SERVO_SHIFT_UP_ANGLE + GEAR_SERVO_MIDDLE_ANGLE);
	time_down = calculate_Servo_ticks(GEAR_SERVO_MIDDLE_ANGLE - GEAR_SERVO_SHIFT_DOWN_ANGLE);
	time_neutral = calculate_Servo_ticks(GEAR_SERVO_MIDDLE_ANGLE - GEAR_SERVO_SHIFT_NEUTRAL_ANGLE);
	time_mid = calculate_Servo_ticks(GEAR_SERVO_MIDDLE_ANGLE)+SHIFT_DEG_OFFSET;
	calculated_ticks = TRUE;
		
}

void shift_control(uint8_t shift_up, uint8_t shift_down, uint8_t gear, uint16_t rpm, uint8_t LC_Active, double GPS_Speed){

	if(LC_Active == TRUE && gear > 0 && Auto_shiftlock <= 0){
		if (gear == 1 && GPS_Speed >= FIRST_GEAR_SHIFT_SPEED && rpm >= SHIFT_UP_RPM_EINS){//Special case for shift into 2. gear because of wheel spin we use GPS Speed to verify that we actually want to shift !! Switch to friont wheelspeed when datalogger is not in car
			shift_up = TRUE;
			gear_up_autmatic = TRUE;
			Auto_shiftlock = AUTO_SHIFTLOCK_TIME;
		}
		if (gear > 1 && rpm >= SHIFT_UP_RPM){
			shift_up = TRUE;
			gear_up_autmatic = TRUE;
			Auto_shiftlock = AUTO_SHIFTLOCK_TIME;

		}
	}
	//Subtracts 10ms per 10ms cycle.
	if (Auto_shiftlock>0){
		Auto_shiftlock-=10;
	}

	//if shifting process wasn't started and a shifting signal is received
	if(!shiftlock && (shift_up == 1 || shift_down == 1)){
		shift_complete = FALSE;
		//set start timestamp
		time_shift_started = sys_time;
		//if shift up signal comes
		if( shift_up && gear < 4 ){
			shift_locktime = LOCKTIME_SHIFT;
			shiftlock = TRUE;
			shift = 0;
			shift_indicator = UP; //Indicates wether the shift up or shift down routine has been started
			servo_locktime_gear = SHIFT_DURATION_UP + SHIFT_DURATION_MID;
			shift_up = FALSE;
			gear_up_autmatic = FALSE;
			if(gear == 0){
				servo_locktime_gear = SHIFT_DURATION_DOWN + SHIFT_DURATION_MID + 50;
			}else if (gear == 1)
			{
				servo_locktime_gear = SHIFT_DURATION_UP_EINS + SHIFT_DURATION_MID;
			}
			gear_desired = gear + 1;
			if (gear == 1){
				shift_duration_current = SHIFT_DURATION_UP_EINS;	
			}else{
				shift_duration_current = SHIFT_DURATION_UP;	
			}
			
			//if we are in neutral and shift up we want gear 1
		}
		//if shift down signal is received
		if(shift_down && gear > 0 ){
			shift_locktime = LOCKTIME_SHIFT;
			shiftlock = TRUE;
			shift = 2;
			shift_indicator = DOWN; //Indicates wether the shift up or shift down routine has been started
			servo_locktime_gear = SHIFT_DURATION_DOWN+SHIFT_DURATION_MID;
			shift_duration_current = SHIFT_DURATION_DOWN;
			gear_desired = gear-1;
			//if we shift down in gear 1 we want neutral gear
			if(gear == 1){
				shift = 1;
				servo_locktime_gear = SHIFT_DURATION_MID+SHIFT_DURATION_NEUTRAL;
				gear_desired = 0;
				
			}
		}
	} else {
		//when the servo should move to desired position
		if((sys_time-time_shift_started) < shift_duration_current && gear_desired != gear){
			
			//if no shifting angle is set
			if(!deg_set){
				deg_set = 1;
				//set shift angle according to wished position
				switch (shift){
					case 0:
					shift_time = time_up;
					break;
					case 1:
					shift_time = time_neutral;
					break;
					case 2:
					shift_time = time_down;
					break;
				}
			}
			//if flatshift time elapsed and engine rpm are fitting activate flatshift
			if(((sys_time - time_shift_started)>FLATSHIT_OFFSET) && shift_indicator == UP && rpm > 5500){
				Shift_Flatshift_Active = TRUE;
				FLATSHIFT_PORT |= (1<<FLATSHIFT_PIN); //Flat shift on
			}
			
			if(((sys_time - time_shift_started)>ANTI_BLIP_OFFSET) && shift_indicator == UP && rpm > 10000){
				Anti_Blipper_Enable = TRUE;
			}			
			
			if(((sys_time - time_shift_started)>BLIPPER_OFFSET) && shift_indicator == DOWN && rpm < 8500){
				Blipper_Enable = TRUE;
			}
		}else{
			Blipper_Enable = FALSE;
			Anti_Blipper_Enable = FALSE;
			Blipper_Enable = FALSE;
			Anti_Blipper_Enable = FALSE;
			//set servo to middle position again
			shift_time = time_mid;
			deg_set = FALSE;	
			
		}
			//when servo should move to middle position again
	} if((shift_complete == FALSE && gear == gear_desired) || (shift_complete == FALSE && (sys_time-time_shift_started) >= shift_duration_current)){
			FLATSHIFT_PORT &= ~(1<<FLATSHIFT_PIN); //Flat shift off
			Shift_Flatshift_Active = FALSE;
			shift_complete = TRUE;
		}
	}


uint16_t calculate_Servo_ticks(double deg){
	
	return (uint16_t) (1800 + (deg * (2400.0 / SERVO_MAXANGLE)));
	
}

void clutch_control(uint8_t clutch, uint8_t clutch_speed, uint8_t gear, uint8_t LC_Active, uint16_t APPS1, uint16_t APPS2, uint16_t BP ){

	if(clutch==TRUE){
		
		clutch_angle = CLUTCH_MAX_ANGLE;
		clutch_time = calculate_Servo_ticks(clutch_angle); //Calculate servo PWM signal for fully pulled angle
		clutch_period = 500*(clutch_speed);  //Calculate the release speed
		pitch = (double)(CLUTCH_MAX_ANGLE)/(clutch_period); //Calculate the released Angle per tick
		LC_Ready = FALSE;
		FLATSHIFT_PORT &= ~(1<<FLATSHIFT_PIN); //Flat shift off
		Launch_Flatshift_Active = FALSE;
		LC_Launch = FALSE;
	}
	
	if(LC_Active==TRUE && clutch==FALSE){//Use LC Method if desired
		if (BP >= BP_MIN && gear == 1 && LC_Ready != TRUE && clutch_period > 0){//Only release servo tho slip position when car is hold with brakes, we are in the right gear & the driver is at least pushing half the throttle
			
			FLATSHIFT_PORT |= (1<<FLATSHIFT_PIN); //Flat shift on
			Launch_Flatshift_Active = TRUE;
			clutch_angle = CLUTCH_SLIP_ANGLE; //Set the Clutch angle to be the slip angle
			clutch_time = calculate_Servo_ticks(CLUTCH_SLIP_ANGLE); //Get the PWM Signal for the slip angle
			LC_Ready = TRUE; //Set the LC Setup Process to be finished
			
			clutch_period = 50*(CLUTCH_SPEED_LAUNCH);  //Calculate the release speed for Launch Control
			pitch = (double)(CLUTCH_MAX_ANGLE)/(clutch_period); //Calculate the released Angle per tick for launch control	
		
		}else if (BP >= BP_RELEASE && LC_Ready == TRUE) { 
		
		}else if (BP <= BP_RELEASE && LC_Ready == TRUE){ //IF we are ready to launch the car and th driver releases the brake pedal the car launches forward
			LC_Launch = TRUE; // start the Launch Procedure
			sys_time_launch = sys_time;
			LC_Ready = FALSE;
			
		
		}else if (LC_Launch == TRUE){
			if(clutch_period > 0){
				clutch_angle = clutch_angle-pitch; //Get the new angle by subtracting the released angle per tick
				clutch_time = calculate_Servo_ticks(clutch_angle); //Calculate the PWM signal for the new angle
				clutch_period -= 1;
				
				if (clutch_period <= 0){ // if the launch process is finished re enable new process to be initiated
					LC_Ready = FALSE;
				}
			}			
		}else{
			if(clutch_period > 0){
				clutch_angle = clutch_angle-pitch; //Get the new angle by subtracting the released angle per tick
				clutch_time = calculate_Servo_ticks(clutch_angle); //Calculate the PWM signal for the new angle
				clutch_period -= 1;
			}
		}
				
	}else{//Use standard Routine if no Launch control is desired
		if(clutch_period > 0){
			clutch_angle = clutch_angle-pitch; //Get the new angle by subtracting the released angle per tick
			clutch_time = calculate_Servo_ticks(clutch_angle); //Calculate the PWM signal for the new angle
			clutch_period -= 1;
			//LC_Launch == TRUE; //If we swicth LC active on during servo release we go into the release function of the LC Process resulting in the servo not stalling
			FLATSHIFT_PORT &= ~(1<<FLATSHIFT_PIN); //Flat shift off
			Launch_Flatshift_Active = FALSE;
		}
	}
	
	
	if (sys_time >= (sys_time_launch + TWO_STEP_OFFSET) && Launch_Flatshift_Active == TRUE && LC_Launch == TRUE)
	{
		FLATSHIFT_PORT &= ~(1<<FLATSHIFT_PIN); //Flat shift off
		Launch_Flatshift_Active = FALSE;
		LC_Launch = FALSE;
	}
}

void calculate_locktimes(){
	
	//locktime calculations
	if (servo_locktime_gear > 0){
		servo_locktime_gear-=10;
	}
	if (servo_locktime_clutch > 0)
	servo_locktime_clutch-=10;
	if(shift_locktime > 0){

		shift_locktime -= 10;
		}else{
		shiftlock = FALSE;
	}	
}

ISR(TIMER1_COMPA_vect){
	
	//disable interrupts
	cli();
	
	switch (servo_active)
	{	
		//shiftservo case
		case 0:
			//toggle old servo
			FAN2_PORT &= ~(1<<FAN2_PIN);
			//if locktime elapsed pull up the signal pin
			//if the servo is shifting
			if (shiftlock){
				SERVO_SHIFT_PORT |= (1<<SERVO_SHIFT_PIN);
			}
			//set the interrupt compare value to the desired time
			OCR1A = shift_time;
			//change var to get to the next case
			servo_active = 1;
			break;
		
		//clutchservo
		case 1:
			//toggle old servo
			SERVO_SHIFT_PORT &= ~(1<<SERVO_SHIFT_PIN);
			//if locktime elapsed pull up the signal pin
			if (clutch_period > 0){
				SERVO_CLUTCH_PORT |= (1<<SERVO_CLUTCH_PIN);
			}
			//set the interrupt compare value to the desired time
			OCR1A = clutch_time;
			//change var to get to the next case
			servo_active = 2;
			break;
		case 2:
			SERVO_CLUTCH_PORT &= ~(1<<SERVO_CLUTCH_PIN);
			FAN1_PORT |= (1<<FAN1_PIN);
			OCR1A = fan_time;
			servo_active = 3;
			break;
		case 3:
			FAN1_PORT &= ~(1<<FAN1_PIN);
			FAN2_PORT |= (1<<FAN2_PIN);
			OCR1A = fan_time;
			servo_active = 0;
			break;
	}
	//re enable interrupts
	sei();
}
