/*
 * gear_read.c
 *
 * Created: 05.04.2019 19:36:33
 *  Author: Ole Hannemann
 */ 

#include "gear_read.h"

uint8_t gear = 10;
volatile uint8_t calculated_adc_values = FALSE;
volatile uint16_t adc_gear_values[5];


void calculate_adc_values(){
		
	adc_gear_values[0] = GEAR_1_VOLTAGE*(ADC_MAX_VALUE/ADC_VOLTAGE_REF);
	adc_gear_values[1] = GEAR_2_VOLTAGE*(ADC_MAX_VALUE/ADC_VOLTAGE_REF);
	adc_gear_values[2] = GEAR_3_VOLTAGE*(ADC_MAX_VALUE/ADC_VOLTAGE_REF);
	adc_gear_values[3] = GEAR_4_VOLTAGE*(ADC_MAX_VALUE/ADC_VOLTAGE_REF);
	adc_gear_values[4] = GEAR_5_VOLTAGE*(ADC_MAX_VALUE/ADC_VOLTAGE_REF);
	calculated_adc_values = TRUE;
	
}
uint8_t gear_read(uint16_t adc_value){
	
	
	//if the adc Values for each gear Hasn't been calculated yet it needs to be done first
	if(calculated_adc_values == FALSE){
		calculate_adc_values();		
	}
	
	gear = 10;
	
	//if digital output is high (for neutral)
	//requires activated PULLDOWN for that pin
	if ((DIGITAL_IN_PORT_INPUT&(1<<DIGITAL_IN_PIN)) == 0){
		gear = 0;
	}
	else{
		//x is an index var to indicate the fitting values in the array
		uint8_t x = 0;
		//if no valid gear is recognized a invalid gear (10) will be transmitted,
		gear=10;
		//while no gear was detected and the index is smaller than 5
		while(gear == 10 && x < 5){
			if (x < 5 && x > 0 ) {
				//if gear is not 1 or 6 use this routine
				if (adc_value >= (adc_gear_values[x-1] - ADC_GEAR_TOLERANCE) && adc_value <= (adc_gear_values[x] + ADC_GEAR_TOLERANCE)){
					gear = x+1;
				}
			//special cases for first and last gear
			} else {
				
					if(adc_value >= adc_gear_values[4] - ADC_GEAR_TOLERANCE && adc_value <= adc_gear_values[4]+ADC_GEAR_TOLERANCE){
						gear = 5;
					}
				
					if(adc_value <= adc_gear_values[0] + ADC_GEAR_TOLERANCE){
						gear = 1;
					}
					//Superspecial function for if the 1. Gear is not welded but there is no Gear we assume we are in neutral.
					if (adc_value <= GEAR_0_VOLTAGE * (ADC_MAX_VALUE/ADC_VOLTAGE_REF)+ADC_GEAR_TOLERANCE/2 && adc_value >= GEAR_0_VOLTAGE * (ADC_MAX_VALUE/ADC_VOLTAGE_REF)-ADC_GEAR_TOLERANCE/2)
					{
						gear = 0;
					}
					
			}

			++x;
		}
	}
	return gear;
}