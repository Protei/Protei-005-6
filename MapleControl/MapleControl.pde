/*
	Protei — Remote Control and Motor Control
 Copyright (C) 2011  Logan Williams, Gabriella Levine,
                     Qiuyang Zhou, Francois de la Taste
 
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
 
#include "consts.h" 	        // File containing constants defininitions
#include "comm.h"	 	// File containing communication subroutines
//#include "motor.h"		// File containing motor control subroutines
#include "motor_controller.h"

// EXECUTION VARIABLES
boolean debug = false;
unsigned long lastExecuted;
unsigned long time;

volatile int motorRotations[3];
int desiredRotations[3];
int error[3];
int integratedError[3];
int lastPIDvalue[3] = {
  0,0,0};
volatile int motorDirection[3]= {
  1,1,1};

// for 19 jul test

int motorNumber;
int stopwatch;

int lastBowCounted;
int lastSternCounted;

int usbElapsCounter = 0;

Motor bow;
MotorController bowController;

Motor stern;
MotorController sternController;

Motor winch;

void setup() {
  bow.init(BOW);
  bowController.init(&bow, GAIN[BOW]);
  pinMode(ROT_PINS[BOW], INPUT);
  attachInterrupt(ROT_PINS[BOW], countBow, CHANGE);

  stern.init(STERN);
  sternController.init(&stern, GAIN[STERN]);
  pinMode(ROT_PINS[STERN], INPUT);
  attachInterrupt(ROT_PINS[STERN], countStern, FALLING);
  
  winch.init(WINCH);
  pinMode(ROT_PINS[WINCH], INPUT);

  // initialize pins
  pinMode(EN_PINS[BOW], INPUT);
  pinMode(EN_PINS[STERN], INPUT);
  pinMode(EN_PINS[WINCH], INPUT);

  pinMode(LIMIT_A_PINS[BOW], INPUT_PULLUP);
  pinMode(LIMIT_A_PINS[STERN], INPUT_PULLUP);
  pinMode(LIMIT_A_PINS[WINCH], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[BOW], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[STERN], INPUT_PULLUP);

  // start the interrupts
  interrupts(); 

  Serial1.begin(57600); // begin Xbee serial comms
  SerialUSB.begin(); // begin USB serial comms
}

/*** THE MAIN LOOP ***
 * The loop body executes every 50ms, for a 20Hz control loop. This is kept in time with millis().
 */
void loop() {

  char data1 = 0x00;
  char data2 = 0x00;
  char data3 = 0x00;

  time = millis();
  
  // bow.calibrate();

  // The main control loop executes every 50 ms (CONTROL_LOOP_PERIOD)
  if (((time - lastExecuted) > CONTROL_LOOP_PERIOD) || ((time - lastExecuted) < 0)) {
    lastExecuted = time; // update our time watch
    debug = usbActive();

    // If we have received new data from the Xbee, use it to update desired position
    if (receive(&data1, &data2, &data3)) { //fixmelater
      bowController.setTarget(map(data1, 0, 255, -10, MAX_MOTOR_ROTATIONS[0] + 10));
      sternController.setTarget(map(data2, 0, 255, -10, MAX_MOTOR_ROTATIONS[1] + 10));
      winch.move(map(data3, 0, 255, -65535, 65535));
    }

    int output = bowController.runLoop();
    int output2 = sternController.runLoop();

    if(debug && (usbElapsCounter >=  usbDebugRate) ) {
      SerialUSB.println("error\toutput\trot\tdesire");
      SerialUSB.print(bowController.getError());
      SerialUSB.print("\t");
      SerialUSB.print(output);
      SerialUSB.print('\t');
      SerialUSB.print(bow.getRotations());
      SerialUSB.print('\t');
      SerialUSB.println(bowController.getTarget());
      usbElapsCounter = 0;
    }
    usbElapsCounter++;
  }
}

/** INTERRUPT SERVICE ROUTINES */

void countBow() {
  if (millis() - lastBowCounted > 40 || millis() - lastBowCounted < 0) { // this essentially "debounces" our rotation sensor
    if (digitalRead(ROT_PINS[BOW]) == LOW) { // double check
      lastBowCounted = millis(); // reset the timer
      bow.count(); // modify the counter accordingly
    }
  }
}

/* this is the same as above */
void countStern() {
  if (millis() - lastSternCounted > 20 || millis() - lastBowCounted < 0) {
    if (digitalRead(ROT_PINS[STERN]) == LOW) {
      lastSternCounted = millis();
      stern.count();
    }
  }
}
