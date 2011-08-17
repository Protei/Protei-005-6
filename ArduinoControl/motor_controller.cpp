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

#include "motor_controller.h"
#include "consts.h"
#include "WProgram.h"

/* MotorController() -- default constructor */
MotorController::MotorController() {
}

/* The controller initializer. It takes two parameters:
    Motor* the motor - a reference to the motor that the
      controller will control
    int theGain - the proportional gain that the controller
      drives the motor with */
void MotorController::init(Motor*  theMotor, int theGain) {
  gain = theGain;
  motor = theMotor;
  targetPosition = (*theMotor).getRotations();
}

/* runLoop() compares the current rotations of the motor against
   the desired rotations, and drives the motor accordingly */
int MotorController::runLoop() {
  int output;
  int error = targetPosition - (*motor).getRotations();
  int rotations = (*motor).getRotations();

  if (abs(error) < 2) { // close enough
    output = 0;
  } 
  else {
    lastRotations = rotations;
    
    output = gain * error;
    if (output > 255) { // limit the output
      output = 255;
    } else if (output < -255) {
      output = -255;
    }
  }
  
  if (rotations == lastRotations) {
    if ((*motor).getDirection() != 0) {
      consecutive++;
    }
  } else {
    consecutive = 0;
  }
  
  if (consecutive > 5) {
    if ((*motor).getDirection() == 1) {
      tmpMax = rotations;
    } else if ((*motor).getDirection() == -1) {
      tmpMin = rotations;
    }
  }
  
  if (rotations < (tmpMax - 5)) {
    tmpMax = 200;
  } else {
    if (output > 0) {
      output = 0;
    }
  }
  
  if (rotations > (tmpMin + 5)) {
    tmpMin = -100;
  } else {
    if (output < 0) {
      output = 0;
    }
  }

  (*motor).move(output);
  return(output);
}

/* sets a new target value for the controller */
void MotorController::setTarget(int newTarget) {
  targetPosition = newTarget;
}

/* returns the current error signal value from
   the controller */
int MotorController::getError() {
  return targetPosition - (*motor).getRotations();
}

/* returns the current target position */
int MotorController::getTarget() {
  return targetPosition;
}

/* print debug information to the USB output */
/* this function should only be called if debug is true */
void MotorController::printDebug() {
  Serial.println("Rots\tTarget\tError\tOutput");
  Serial.print((*motor).getRotations());
  Serial.print("\t");
  Serial.print(targetPosition);
  Serial.print("\t");
  Serial.print(getError());
  Serial.print("\t");
  Serial.println((*motor).getSpeed());
}
  
