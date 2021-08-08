/*
 * uFAR.c
 *
 * Created: 04.04.2019 20:52:02
 * Author : Ole Hannemann
 */ 

#define SHIFT_UP swc_databytes[3]
#define SHIFT_DOWN swc_databytes[2]
#define BUTTON_LEFT swc_databytes[4]
#define BUTTON_RIGHT swc_databytes[5]
#define LEFT_ENCODER swc_databytes[1]
#define RIGHT_ENCODER swc_databytes[0]

#include <avr/io.h>
#include "canlib.h"
#include "misc_functions.h"
#include "servo_functions.h"
#include "adc_functions.h"
#include "gear_read.h"
#include "fan_control.h"
#include "Fuse_CTRL.h"
#include <avr/interrupt.h>

extern unsigned long sys_time;
volatile unsigned long time_old = 0;
volatile uint8_t time_old_100 = 0;
uint16_t rpm = 0;
volatile uint16_t fan_deadtime = 75;
extern volatile uint8_t Blipper_Enable;
extern volatile uint8_t Anti_Blipper_Enable;
extern uint16_t fan_time;
uint8_t LC_Active = 0;
double BP = 0;
uint8_t APPS1 = 0;
uint8_t APPS2 = 0;
extern LC_Launch;
extern LC_Ready;
extern EMULC_Active;

int main(void)
{
    can_cfg();
	adc_config();
	servo_timer_config();
	sys_timer_config();
	port_config();

	struct CAN_MOB can_SWC_mob;
	can_SWC_mob.mob_id = 0x100;
	can_SWC_mob.mob_idmask = 0xfff;
	can_SWC_mob.mob_number = 0;
	uint8_t swc_databytes[8];
	
	struct CAN_MOB can_CMC_mob;
	can_CMC_mob.mob_id = 0x200;
	can_CMC_mob.mob_idmask = 0; //We are sending this CAN Message Object (MOB) therfore wo do not need an ID MASK
	can_CMC_mob.mob_number = 1;
	uint8_t cmc_databytes[8];

	struct CAN_MOB can_ecu0_mob;
	can_ecu0_mob.mob_id = 0x600;
	can_ecu0_mob.mob_idmask = 0xffff;
	can_ecu0_mob.mob_number = 2;
	uint8_t ecu0_databytes[8];
	
	struct CAN_MOB can_ecu1_mob;
	can_ecu1_mob.mob_id = 0x602;
	can_ecu1_mob.mob_idmask = 0xffff;
	can_ecu1_mob.mob_number = 3;
	uint8_t ecu1_databytes[8];
	
	struct CAN_MOB can_DIC_mob;
	can_ecu1_mob.mob_id = 0x202;
	can_ecu1_mob.mob_idmask = 0xffff;
	can_ecu1_mob.mob_number = 4;
	uint8_t DIC_databytes[8];
	
	struct CAN_MOB can_ETC_mob;
	can_ETC_mob.mob_id = 0x201;
	can_ETC_mob.mob_idmask = 0xffff;
	can_ETC_mob.mob_number = 5;
	uint8_t ETC_databytes[8];
	
	struct CAN_MOB can_SHL_mob;
	can_SHL_mob.mob_id = 0x503;
	can_SHL_mob.mob_idmask = 0xffff;
	can_SHL_mob.mob_number = 6;
	uint8_t SHL_databytes[8];
	
	struct CAN_MOB can_EMUSW_mob;
	can_EMUSW_mob.mob_id = 0x606;
	can_EMUSW_mob.mob_idmask = 0xffff;
	can_EMUSW_mob.mob_number = 7;
	uint8_t EMUSW_databytes[8];	
		
	volatile uint8_t gear = 10;
	
	sei();
	
    while (1) {
		
		//10ms loop 100Hz
		if((sys_time - time_old) >= 10){
			time_old = sys_time;
			time_old_100++;
			
			gear = gear_read(adc_read());
			//send some additional data to make out if the gear gets transmitted correctly
			cmc_databytes[0] = gear;
			cmc_databytes[1] = fan_time & 0xff;
			cmc_databytes[2] = fan_time >> 8;
			cmc_databytes[3] = get_fuse_status()&0xff;//get LSB
			cmc_databytes[4] = get_fuse_status()>>8;//get MSB	
			cmc_databytes[5] = Blipper_Enable;
			cmc_databytes[6] = Anti_Blipper_Enable;
			cmc_databytes[7] = gear+1;
					
			can_tx(&can_CMC_mob, cmc_databytes);
			can_tx(&can_EMUSW_mob, EMUSW_databytes);
			
			can_rx(&can_SWC_mob, swc_databytes);
			can_rx(&can_ecu0_mob, ecu0_databytes);
			can_rx(&can_ecu1_mob, ecu1_databytes);
			can_rx(&can_DIC_mob, DIC_databytes);
			
			rpm = ecu0_databytes[1]<<8 | ecu0_databytes[0];
			LC_Active = DIC_databytes[0];
			BP = (SHL_databytes[3] <<8|SHL_databytes[2])/10; //gets Brake Pressure in 0.1 Bar
			APPS1 = ETC_databytes[0];
			APPS2 = ETC_databytes[1];
			
			//IF we are in the Launch control procedure we want to tell the ecumaster to switch 2step on via the CAN SW1 bit
			if (EMULC_Active = TRUE;)
			{
				EMUSW_databytes[5] = 1;
			}
			else
			{
				EMUSW_databytes[5] = 0;
			}

			calculate_locktimes();
			shift_control(SHIFT_UP,SHIFT_DOWN,gear,rpm);
			clutch_control(BUTTON_LEFT||BUTTON_RIGHT,LEFT_ENCODER+1,gear,LC_Active,APPS1,APPS2,BP);
			fuelpump_CTRL(rpm);
			
		}
		//100ms loop 10Hz
		if(time_old_100 >=10){
			time_old_100 = 0;
			sys_tick();
			
			if(fan_deadtime >0){
				fan_deadtime--;//wo wird sie zurückgesetzt wenn sie einmal kleiner 0 ist
				
		}	else {
				int16_t temp = ecu1_databytes[7]<<8 | ecu1_databytes[6];
				
				if(temp<0){
					temp = 0;
				}
				
				uint8_t temp_short = (uint8_t) temp;
				fan_speed_control(temp_short, rpm); //26min???
			}
		}
		
	}
}

