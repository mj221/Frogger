# Frogger (UQ- CSSE2010 Assessment Item)
C programming for AVR ATmega324A microcontroller based on the Frogger arcade game. Requires .hex file to download to the ATmega324A AVR microcontroller program memory. The specification of IO devices and ports used in this project is available in `IO Ports.png` 

## Usage Description
This version of frogger runs on the Atmega324A microcontroller which receives number of inputs (joystick, push button) and outputs the display on the LED display board (+ Seven segment display, other LEDS), with additional information being outputted to the serial terminal (score, lives, etc).

## The Game
The objective of the game is to cross the road, avoiding incoming traffic and safely crossing the river on logs to the other side.
The game ends when the frog dies (3 lives, as indicated on the LEDs) or has crossed the road. 
Colliding with the traffic, falling into the river or leaving the field will result in the loss of a life point. 
The frog scores 1 point every time it progresses forward, and 10 points for getting across to the other side.

### Movement
This game allows you to move the frog freely in omni directions using the joystick or push buttons. Holding down button/joystick exhibits auto-repeat behaviour after a short delay such that it is as if being repeatedly pressed. 

### Traffic lanes
The program scrolls traffic lanes at varying speeds (between 750ms to one pixel per 1.3s) to make the play through more difficult. Frog loses a life point at collision, and is moved back to the starting point. 

### Logs
The program scrolls logs at varying speeds. Logs can be used to cross the 'river' which is necessary to reach the other side. Falling into the river means life point loss.

### Pause Game
Press 'p' or 'P' key on the serial terminal to pause the game, or resume if already paused. Traffic and log speed is unaffected by this action.

### Time Limit
Frog must cross to the other side before the time limit of 15 seconds, which is displayed on the seven-segment display. 
