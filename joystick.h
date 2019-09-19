/*
 * joystick.h
 *
 * by Minjae Lee
*/


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <stdint.h>
#define TOP 1

#define TOPLEFT 0
#define TOPRIGHT 2
#define LEFT 3
#define CENTRE 4
#define BOTTOMLEFT 6
#define BOTTOMRIGHT 8
#define RIGHT 5

#define WIDTH 1024
#define HEIGHT 1024

#define JOY_X_MID 515
#define JOY_Y_MID 515
#define BOTTOM 7


#define SET_MID_X 50
#define SET_MID_Y 50
#define SET_DIAGONAL 31

void init_joystick(void);

uint8_t joystick_movement(void);

uint8_t update_latest_movement(void);

uint16_t get_last_x(void);
uint16_t get_last_y(void);


uint8_t get_current_zone(void);

#endif /* JOYSTICK_H_ */