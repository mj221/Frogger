/*
 * joystick.c
 *
 * Author: Minjae Lee
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "timer0.h"
#include "joystick.h"
static volatile uint8_t check_if_x_y; 

static uint8_t last_joystick_zone;
static uint32_t last_joystick_time;

static volatile uint16_t prev_x_axis; 
static volatile uint16_t prev_y_axis; 
	
static uint8_t movement_value;

#define REPEAT_DELAY 200

void init_joystick(void) {

	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();

	ADMUX = (1<<REFS0)|(1<<MUX2)|(1<<MUX1);
	
	ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1);
	
	prev_x_axis = 512;
	prev_y_axis = 512;
	check_if_x_y = 0;
	last_joystick_zone = 4;
	last_joystick_time = 0;
	movement_value = 4;
	
	// Reenable interrupts
	if(interruptsOn) {
		sei();
	}
	
	// Start the ADC conversion
	ADCSRA |= (1<<ADSC);
}

uint8_t joystick_movement(void) {

	uint8_t interruptsOn = bit_is_set(SREG, SREG_I);
	cli();
	
	movement_value = 4;
	
	uint8_t current_zone = get_current_zone();
	
	if ((last_joystick_time != 0) && (get_current_time() > last_joystick_time + REPEAT_DELAY)) {
		if (current_zone == 4) {
			last_joystick_time = 0;
		} else {
			movement_value = current_zone;
			last_joystick_time = get_current_time();
		}
	} else if (last_joystick_time == 0) {
		if (last_joystick_zone == 4 && current_zone != 4) {
			movement_value = current_zone;
			last_joystick_time = get_current_time();
		}
	}
	
	last_joystick_zone = current_zone;
	
	if(interruptsOn) {
		sei();
	}
	return (movement_value != 4);
}

uint8_t update_latest_movement(void) {
	return movement_value;
}

uint16_t get_prev_x_axis(void) {
	return prev_x_axis;
}

uint16_t get_prev_y_axis(void) {
	return prev_y_axis;
}


uint8_t get_current_zone(void) {
	if (prev_y_axis < (JOY_Y_MID - SET_MID_Y)) {
		return TOP;
	}
	if (prev_x_axis < (JOY_X_MID - SET_DIAGONAL) && prev_y_axis < (JOY_Y_MID - SET_DIAGONAL)) {
		return TOPLEFT;
	}
	if (prev_x_axis > (JOY_X_MID + SET_DIAGONAL) && prev_y_axis < (JOY_Y_MID - SET_DIAGONAL)) {
		return TOPRIGHT;
	}
	if (prev_y_axis > (JOY_Y_MID + SET_MID_Y)) {
		return BOTTOM;
	}
	if (prev_x_axis < (JOY_X_MID - SET_DIAGONAL) && prev_y_axis > (JOY_Y_MID + SET_DIAGONAL)) {
		return BOTTOMLEFT;
	}
	if (prev_x_axis > (JOY_X_MID + SET_DIAGONAL) && prev_y_axis > (JOY_Y_MID + SET_DIAGONAL)) {
		return BOTTOMRIGHT;
	}
	if (prev_x_axis < (JOY_X_MID - SET_MID_X)) {
		return LEFT;
	}
	if (prev_x_axis > (JOY_X_MID + SET_MID_X)) {
		return RIGHT;
	}
	return CENTRE;
}

ISR(ADC_vect) {
	uint16_t value = ADC;
	
	if (check_if_x_y == 0) {
		prev_x_axis = value;
	} else {
		prev_y_axis = HEIGHT - value;
	}
	
	check_if_x_y = 1 - check_if_x_y;
	if (check_if_x_y== 0) {
		ADMUX &= ~1;
	} else {
		ADMUX |= 1;
	}
		
	ADCSRA |= (1<<ADSC);
}