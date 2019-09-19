/*
 * FroggerProject.c
 *
 * Main file
 *
 * Author: Peter Sutton. Modified by <Minjae Lee>
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "timer0.h"
#include "game.h"
#include "joystick.h"
#include "pixel_colour.h"
#include <inttypes.h>

#define F_CPU 8000000L
#include <util/delay.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

// ASCII code for Escape character
#define ESCAPE_CHAR 27

#define SET_TIMER_COUNT 20
int score;
int lives = 4;
int level = 3;
int paused = 0;

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
	
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}
void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	
	
	
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	DDRC = 0xFF;
	DDRD = 0b00000001;
	init_timer0();
	init_joystick();
	// Turn on global interrupts
	sei();

	
}

void splash_screen(void) {
	// Clear terminal screen and output a message
	clear_terminal();
	move_cursor(10,10);
	printf_P(PSTR("Frogger"));
	move_cursor(10,12);
	printf_P(PSTR("CSSE2010/7201 project by Minjae Lee 45363809"));
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	DDRA = 0b00001111;
	PORTA = DDRA;
	init_score();
	while(1) {
		set_scrolling_display_text("FROGGER BY MINJAE LEE 45363809", COLOUR_GREEN);
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed
		while(scroll_display()) {
			_delay_ms(150);
			if(button_pushed() != NO_BUTTON_PUSHED) {
				return;
			}
		}
	}
}

void new_game(void) {
	// Initialise the game and display
	initialise_game();
	
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the score
	
	init_countdown();
	move_cursor(10,20);
	printf("Score");
	move_cursor(20,20);
	score = get_score();
	printf("%3d", score);
	PORTA = DDRA;
	commence_ingame_timer();
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	
	clear_serial_input_buffer();
}

void play_game(void) {
	
	uint32_t current_time, last_move_time;
	uint32_t current_time_2, last_move_time_2;
	uint32_t current_time_3, last_move_time_3;
	uint32_t current_time_4, last_move_time_4;
	uint32_t current_time_5, last_move_time_5;
	
	int8_t button;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
	
	// Get the current time and remember this as the last time the vehicles
	// and logs were moved.

	current_time = get_current_time();
	current_time_2 = get_current_time();
	current_time_3 = get_current_time();
	current_time_4 = get_current_time();
	current_time_5 = get_current_time();
	
	last_move_time = current_time;
	last_move_time_2 = current_time_2;
	last_move_time_3 = current_time_3;
	last_move_time_4 = current_time_4;
	last_move_time_5 = current_time_5;
	// We play the game while the frog is alive and we haven't filled up the 
	// far riverbank
	countdown_set(SET_TIMER_COUNT);
	while(!is_frog_dead() && !is_riverbank_full()) {
		
		if(check_countdown()){
			kill_frog();			
			ledmatrix_update_pixel(get_frog_column(),get_frog_row(), COLOUR_LIGHT_YELLOW);
			_delay_ms(1000);
			countdown_clear();
		}
		//while(!is_frog_dead() && !frog_has_reached_riverbank()){
		if(!is_frog_dead() && frog_has_reached_riverbank()) {
			// Frog reached the other side successfully but the
			// riverbank isn't full, put a new frog at the start			
			put_frog_in_start_position();
			countdown_set(SET_TIMER_COUNT);
		}
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. At most one of the following three
		// variables will be set to a value other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// Button pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop
		serial_input = 1;
		escape_sequence_char = -1;
		button = button_pushed();
		
		if(button == NO_BUTTON_PUSHED) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
				// Check if the character is part of an escape sequence
				if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
					// We've hit the first character in an escape sequence (escape)
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
					// We've hit the second character in an escape sequence
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 2) {
					// Third (and last) character in the escape sequence
					escape_sequence_char = serial_input;
					serial_input = -1;  // Don't further process this character - we
										// deal with it as part of the escape sequence
					characters_into_escape_sequence = 0;
				} else {
					// Character was not part of an escape sequence (or we received
					// an invalid second character in the sequence). We'll process 
					// the data in the serial_input variable.
					characters_into_escape_sequence = 0;
				}
			}
		}
		
		// Process the input. 
		if(button==3 || escape_sequence_char=='D' || serial_input=='L' || serial_input=='l') {
			// Attempt to move left
			if (paused == 0){
			move_frog_to_left();
			}
		} else if(button==2 || escape_sequence_char=='A' || serial_input=='U' || serial_input=='u') {
			// Attempt to move forward
			if (paused == 0){
			move_frog_forward();
			clear_terminal();
			move_cursor(10,20);
			printf("Score");
			move_cursor(20, 20);
			score = get_score();
			printf("%3d", score);
			}
		} else if(button==1 || escape_sequence_char=='B' || serial_input=='D' || serial_input=='d') {
			// Attempt to move down
			if (paused ==0){
			move_frog_backward();
			}
		} else if(button==0 || escape_sequence_char=='C' || serial_input=='R' || serial_input=='r') {
			// Attempt to move right
			if (paused ==0){
			move_frog_to_right();
			}
			
		} else if(serial_input == 'p' || serial_input == 'P') {
			// Unimplemented feature - pause/unpause the game until 'p' or 'P' is
			// pressed again
			if (paused == 1){
				paused = 0;
			}else if (paused ==0){
				paused =1;
			}
		}else if(joystick_movement()){
		switch (update_latest_movement()) {
			case TOPLEFT:
			if (paused ==0){
			move_frog_forward_left();
			}
			break;
			case TOP:
			if (paused ==0){
			move_frog_forward();
			}
			break;
			case TOPRIGHT:
			if (paused ==0){
			move_frog_forward_right();
			}
			break;
			case LEFT:
			if (paused ==0){
			move_frog_to_left();
			}
			break;
			case RIGHT:
			if (paused ==0){
			move_frog_to_right();
			}
			break;
			case BOTTOMLEFT:
			if (paused ==0){
			move_frog_backward_left();
			}
			break;
			case BOTTOM:
			if (paused ==0){
			move_frog_backward();
			}
			break;
			case BOTTOMRIGHT:
			if (paused ==0){
			move_frog_backward_right();
			}
			break;
		
		}
		} else{
			switch (button_repeat()) {
				case 0:
				if (paused ==0){
				move_frog_to_right();
				}
				break;
				case 1:
				if (paused ==0){
				move_frog_backward();
				}
				break;
				case 2:
				if (paused ==0){
				move_frog_forward();
				}
				break;
				case 3:
				if (paused ==0){
				move_frog_to_left();
				}
				break;
			}
		}
		// else - invalid input or we're part way through an escape sequence -
		// do nothing
		if (paused == 0){
		commence_ingame_timer();
		current_time = get_current_time();
		current_time_2 = get_current_time();
		current_time_3 = get_current_time();
		current_time_4 = get_current_time();
		current_time_5 = get_current_time();
		if(!is_frog_dead() && current_time >= last_move_time + 1000) {
				// 1000ms (1 second) has passed since the last time we moved
				// the vehicles and logs - move them again and keep track of
				// the time when we did this.
				scroll_vehicle_lane(0, 1);
				last_move_time = current_time;
		}
		if (!is_frog_dead() && current_time_2 >= last_move_time_2 + 1300){
				scroll_vehicle_lane(1, -1);
				last_move_time_2 = current_time_2;
		}
		if (!is_frog_dead() && current_time_3 >= last_move_time_3 + 750){
				scroll_vehicle_lane(2, 1);
				last_move_time_3 = current_time_3;
		}
		if (!is_frog_dead() && current_time >= last_move_time_4 + 890){
				scroll_river_channel(0, -1);
				last_move_time_4 = current_time_4;
		}
		if (!is_frog_dead() && current_time >= last_move_time_5 + 1100){
				scroll_river_channel(1, 1);
				last_move_time_5 = current_time_5;
		}
		}else if (paused ==1){
			stop_ingame_timer();
			current_time = get_current_time();
			current_time_2 = get_current_time();
			current_time_3 = get_current_time();
			current_time_4 = get_current_time();
			current_time_5 = get_current_time();
			if(!is_frog_dead() && current_time >= last_move_time + 1000) {
				// 1000ms (1 second) has passed since the last time we moved
				// the vehicles and logs - move them again and keep track of
				// the time when we did this.
				scroll_vehicle_lane(0, 0);
				last_move_time = current_time;
			}
			if (!is_frog_dead() && current_time_2 >= last_move_time_2 + 1300){
				scroll_vehicle_lane(1, 0);
				last_move_time_2 = current_time_2;
			}
			if (!is_frog_dead() && current_time_3 >= last_move_time_3 + 750){
				scroll_vehicle_lane(2, 0);
				last_move_time_3 = current_time_3;
			}
			if (!is_frog_dead() && current_time >= last_move_time_4 + 890){
				scroll_river_channel(0, 0);
				last_move_time_4 = current_time_4;
			}
			if (!is_frog_dead() && current_time >= last_move_time_5 + 1100){
				scroll_river_channel(1, 0);
				last_move_time_5 = current_time_5;
			}
		}
		//}
}
	// We get here if the frog is dead or the riverbank is full (not really due to my code structure)
	// The game is over.
	
	if (lives != 0 && !is_riverbank_full()){
		stop_ingame_timer();
		move_cursor(10,20);
		printf("Score");
		move_cursor(20,20);
		score = get_score();
		printf("%3d", score);
		lives -= 1;
		_delay_ms(1000);
		//put_frog_in_start_position();
		DDRA = (DDRA >> 1);
		PORTA = (DDRA);
	}else if (lives != 0 && is_riverbank_full()){
	_delay_ms(1000);
	move_cursor(10,20);
	printf("Score");
	move_cursor(20,20);
	score = get_score();
	printf("%3d", score);
	put_frog_in_start_position();
	}
}

void handle_game_over() {
	if (lives == 0){	
	stop_ingame_timer();
	PORTA = 0;
	move_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_cursor(10,15);
	printf_P(PSTR("Press a button to start again"));
	move_cursor(10,20);
	printf("Score");
	move_cursor(20,20);
	score = get_score();
	printf("%3d", score);
	lives = 4;
	DDRA = 0b00001111;
	init_score();
	while(button_pushed() == NO_BUTTON_PUSHED) {
		; // wait
	}
	}

	
}