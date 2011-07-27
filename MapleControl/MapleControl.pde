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

int usbElapsCounter;

#include "consts.h" 	        // File containing constants defininitions
#include "comm.h"	 	// File containing communication subroutines
//#include "motor.h"		// File containing motor control subroutines
#include "motor_controller.h"

Motor bow;
MotorController bowController;

Motor stern;
MotorController sternController;

void setup() {
  usbElapsCounter = 0;

  bow.init(BOW);
  bowController.init(&bow, GAIN[BOW]);
  pinMode(ROT_PINS[BOW], INPUT);
  attachInterrupt(ROT_PINS[BOW], countBow, CHANGE);

  stern.init(STERN);
  sternController.init(&stern, GAIN[STERN]);
  pinMode(ROT_PINS[STERN], INPUT);
  attachInterrupt(ROT_PINS[STERN], countStern, FALLING);

  // initialize pins
  pinMode(EN_PINS[BOW], INPUT);
  pinMode(EN_PINS[STERN], INPUT);
  pinMode(EN_PINS[WINCH], INPUT);

  //pinMode(XBEE_TX_PIN, OUTPUT);
  //pinMode(XBEE_RX_PIN, INPUT);

  pinMode(LIMIT_A_PINS[BOW], INPUT_PULLUP);
  pinMode(LIMIT_A_PINS[STERN], INPUT_PULLUP);
  pinMode(LIMIT_A_PINS[WINCH], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[BOW], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[STERN], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[WINCH], INPUT_PULLUP);

  // start the interrupts
  interrupts(); 

  Serial1.begin(9600); // begin Xbee serial comms
  SerialUSB.begin(); // begin USB serial comms


}

/*** THE MAIN LOOP ***
 * The loop body executes every 50ms, for a 20Hz control loop. This is kept in time with millis().
 */
void loop() {

  char data1 = 0x00;
  char data2 = 0x00;

  time = millis();
  
  // bow.calibrate();

  // The main control loop executes every 50 ms (CONTROL_LOOP_PERIOD)
  if (((time - lastExecuted) > CONTROL_LOOP_PERIOD) || ((time - lastExecuted) < 0)) {
    lastExecuted = time; // update our time watch
    debug = usbActive();

    // If we have received new data from the Xbee, use it to update desired position
    if (receive(&data1, &data2) || true) { //fixmelater
      bowController.setTarget(getTarget(millis()));
      sternController.setTarget(getTarget(millis()));

      //stern.move(map(data2, 0, 255, -65535, 65535));
      // banana shape
      //desiredRotations[1] = MAX_MOTOR_ROTATIONS[1] - map(data2, 0, 255, 0, MAX_MOTOR_ROTATIONS[1]);
      //desiredRotations[2] = map(data1, 0, 255, 0, MAX_MOTOR_ROTATIONS[2]);
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
    if (millis() - lastBowCounted > 40) {
      if (digitalRead(ROT_PINS[BOW]) == LOW) {
        lastBowCounted = millis();
        bow.count();
      }
    }
  
}

void countStern() {
  if (millis() - lastSternCounted > 20) {
    if (digitalRead(ROT_PINS[STERN]) == LOW) {
      lastSternCounted = millis();
      stern.count();
    }
  }
}

/* TESTING HELPER FUNCTION */

int getTarget(int time) {
  while (time > 30000) {
    time -= 30000;
  }

  if (time < 10000) {
    return 50;
  } 
  else if (time < 15000) {
    return 111;
  } 
  else if (time < 20000) {
    return 50;
  } 
  else if (time < 25000) {
    return -10;
  } 
  else {
    return 50;
  }
}


