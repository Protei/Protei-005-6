/*
	Protei — Remote Control and Motor Control
    Copyright (C) 2011  Logan Williams, Gabriella Levine, Qiuyang Zhou

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program (see COPYING).  If not, see <http://www.gnu.org/licenses/>.
*/

#include "consts.h" 	// File containing constants defininitions
#include "comm.h"	 	// File containing communication subroutines
#include "motors.h"		// File containing motor control subroutines
#include "pid.h"		// File containing PID control subroutines

// EXECUTION VARIABLES
boolean debug = false;
unsigned long lastExecuted;
unsigned long time;

volatile int motorRotations[3];
int desiredRotations[3];
int error[3];
int integratedError[3];
volatile int currentDrive[3];

void setup() {
  // initialize pins
  pinMode(EN_PINS[BOW], OUTPUT);
  pinMode(EN_PINS[STERN], OUTPUT);
  pinMode(EN_PINS[WINCH], OUTPUT);
  pinMode(XBEE_TX_PIN, OUTPUT);
  pinMode(XBEE_RX_PIN, INPUT);
  pinMode(ROT_PINS[BOW], INPUT);
  pinMode(ROT_PINS[STERN], INPUT);
  pinMode(ROT_PINS[WINCH], INPUT);
  pinMode(LIMIT_A_PINS[BOW], INPUT_PULLUP);
  pinMode(LIMIT_A_PINS[STERN], INPUT_PULLUP);
  pinMode(LIMIT_A_PINS[WINCH], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[BOW], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[STERN], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[WINCH], INPUT_PULLUP);
  
  // enable the motor drivers
  digitalWrite(EN_PINS[BOW], HIGH);
  digitalWrite(EN_PINS[STERN], HIGH);
  digitalWrite(EN_PINS[WINCH], HIGH);
  
  // add interrupts for the sensors
  attachInterrupt(ROT_PINS[BOW], countBow, RISING); 
  attachInterrupt(ROT_PINS[STERN], countStern, RISING);
  attachInterrupt(ROT_PINS[WINCH], countWinch, RISING);
  attachInterrupt(LIMIT_A_PINS[BOW], resetBowA, FALLING);
  attachInterrupt(LIMIT_A_PINS[STERN], resetSternA, FALLING);
  attachInterrupt(LIMIT_A_PINS[WINCH], resetWinchA, FALLING);
  attachInterrupt(LIMIT_B_PINS[BOW], resetBowB, FALLING);
  attachInterrupt(LIMIT_B_PINS[STERN], resetSternB, FALLING);
  attachInterrupt(LIMIT_B_PINS[WINCH], resetWinchB, FALLING);
  
  // start the interrupts
  interrupts(); 
  
  Serial1.begin(9600); // begin Xbee serial comms
  SerialUSB.begin(); // begin USB serial comms
}

/*** THE MAIN LOOP ***
The loop body executes every 50ms, for a 20Hz control loop. This is kept in time with millis().
*/
void loop() {
  char data1;
  char data2;
  
  time = millis();
  
  // The main control loop executes every 50 ms (CONTROL_LOOP_PERIOD)
  if (((time - lastExecuted) > CONTROL_LOOP_PERIOD) || ((time - lastExecuted) < 0)) {
    lastExecuted = time; // update our time watch
    
    debug = usbActive();
    
    // If we have received new data from the Xbee, use it to update desired position
    if (receive(&data1, &data2)) {
      desiredRotations[0] = map(data2, 0, 255, 0, MAX_MOTOR_ROTATIONS[0]);
      // banana shape
      desiredRotations[1] = MAX_MOTOR_ROTATIONS[1] - map(data2, 0, 255, 0, MAX_MOTOR_ROTATIONS[1]);
      desiredRotations[2] = map(data1, 0, 255, 0, MAX_MOTOR_ROTATIONS[2]);
    }
    
    // run the PID controllers
    motorWrite(BOW, PID(BOW));
    motorWrite(STERN, PID(STERN));
    motorWrite(WINCH, PID(WINCH));
  }
}

/** INTERRUPT SERVICE ROUTINES */
void countBow() {
  count(BOW);
}

void countStern() {
  count(STERN);
}

void countWinch() {
  count(WINCH);
}

void count(int motor) {
  // what direction are we turning
  if (currentDrive[motor] > 0) {
    motorRotations[motor]++;
  } else {
    motorRotations[motor]--;
  }
}

void resetCount(int motor, boolean direction) {
  motorBreak(motor);
  
  if (direction) {
    motorRotations[motor] = MAX_MOTOR_ROTATIONS[motor];
  } else {
    motorRotations[motor] = 0;
  }
}

void resetBowA() {
  resetCount(BOW, false);
}

void resetSternA() {
  resetCount(STERN, false);
}

void resetWinchA() {
  resetCount(WINCH, false);
}

void resetBowB() {
  resetCount(BOW, true);
}

void resetSternB() {
  resetCount(STERN, true);
}

void resetWinchB() {
  resetCount(WINCH, true);
}

