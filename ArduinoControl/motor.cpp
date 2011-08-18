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

#include "motor.h"
#include "consts.h"
#include "WProgram.h"

/* empty constructor */
Motor::Motor() {
}

/* helper constructor */
Motor::Motor(int motor) {
  init(motor);
}

/* init() takes one argument:
     int motor -- the motorNumber that the object
       is commanding */
void Motor::init(int motor) {
  motorNumber = motor;
  pinMode(EN_PINS[motorNumber], INPUT);
  rotations = MAX_MOTOR_ROTATIONS[motorNumber]/2;
  pinMode(ROT_PINS[motorNumber], INPUT);
}

/* a helper function for when we are not
   calibrating the motor */
int Motor::move(int speed) {
  move(speed, false);
}

/* move(int speed, int calibrating) is a function
   for driving the motor at a specified value. it
   takes two arguments:
     int speed -- the speed, between -65535 and 65535
       that we wish to drive the motor
     int calibrating -- whether or not we are calibrating
       the motor. 0 (false) if not calibrating, 1 (true)
       otherwise */
int Motor::move(int moveSpeed, int calibrating) {
  // are we changing direction?
  if ((currentSpeed > 0 && moveSpeed < 0) || (currentSpeed < 0 && moveSpeed > 0)) {
    if (digitalRead(ROT_PINS[motorNumber]) == LOW) {
      // don't change direction if the rotation sensor is on
      moveSpeed = currentSpeed;
    } else { 
      // otherwise, make sure we stop for at least 250 milliseconds before reversing
      // (to ensure that rotations are counted correctly)
      if (moveSpeed > 4) {
        moveSpeed = 4;
      } else if (moveSpeed > 0) {
        moveSpeed -= 1;
      } else if (moveSpeed < -4) {
        moveSpeed = -4;
      } else if (moveSpeed < 0) {
        moveSpeed += 1;
      }
    }
  }
  
  // if we are not changing direction, we accept the
  // speed provided
  currentSpeed = moveSpeed;
 
   // read the status of the limit switches
  int limitA = digitalRead(LIMIT_A_PINS[motorNumber]);
  int limitB = digitalRead(LIMIT_B_PINS[motorNumber]);
  
  if ((limitA == LOW) || (limitB == LOW)) {
    if ((limitA == LOW) && ((*this).getDirection() == -1)) {
      // if we are running into limit A, brake, and reset the rotation
      // sensor to the lower limit (unless in calibration mode)
      brake();
      if (0 == calibrating) {
        resetLimitLow();
      }
      
      // return -1000 to indicate that we ran into the lower limit sensor (A)
      return -1000; 
    } else if ((limitB == LOW) && ((*this).getDirection() == 1)) {
      // if we are running into Limit B, brake, and reset the rotaiton
      // sensor to the upper limit (unless in calibration mode)
      brake();
      if (0 == calibrating) {
        resetLimitHigh();
      }
      
      // return 1000 to indicate that we ran into the lower limit sensor (B)
      return 1000;
    } else {
      // other wise, we are moving away from the limit sensor that has been pressed.
      
      // this is okay, so re-enable the motor driver
      pinMode(EN_PINS[motorNumber], OUTPUT);
      digitalWrite(EN_PINS[motorNumber], HIGH);
    }
  } else {
    // if no limit switches are pressed, the motor driver enable mode should be passive
    // (controlled by limit switches)
    pinMode(EN_PINS[motorNumber], INPUT);
  }
  
  // send PWM data to the motor controller, unless the speed is very slow, in which case
  // we tell the motor controller to brake
  if (moveSpeed > 30) {
    digitalWrite(RPWM_PINS[motorNumber], HIGH);
    analogWrite(LPWM_PINS[motorNumber], 255 - moveSpeed);
  } else if (moveSpeed < -30) {
    digitalWrite(LPWM_PINS[motorNumber], HIGH);
    analogWrite(RPWM_PINS[motorNumber], 255 + moveSpeed);
  } else {
    brake();
  }
  
  return moveSpeed;
}

/* brake() commands the motor driver to brake
   the motor. */
void Motor::brake() {
  digitalWrite(RPWM_PINS[motorNumber], HIGH);
  digitalWrite(LPWM_PINS[motorNumber], HIGH);
  currentSpeed = 0;
}

/* return the current speed */
int Motor::getSpeed() {
  return currentSpeed;
}

/* by checking the current output speed, this returns
   1 if moving in a positive direction (towards limit B)
   and 0 otherwise */
int Motor::getDirection() {
  if (currentSpeed > 0) {
    return 1;
  } else if (currentSpeed < 0) {
    return -1;
  } else {
    return 0;
  }
}

/* return the current number of rotations */
int Motor::getRotations() {
  return rotations;
}

/* reset the current number of rotations to minimum */
int Motor::resetLimitLow() {
  rotations = 0;
  return 0;
}

/* reset the current number of rotations to maximum */
int Motor::resetLimitHigh() {
  rotations = MAX_MOTOR_ROTATIONS[motorNumber];
  return rotations;
}

/* count() increments or decrements the rotation counter
   depending on the direction the motor is spinning */
void Motor::count() {
  if (getDirection() == 1) {
    rotations++;
  } 
  else if (getDirection() == -1) {
    rotations--;
  }
  
  cntFlag = 1; // set the cntFlag for the status information
}

/* Because the rotation sensor only goes low for a very
   short period of time, this function returns 1 (true)
   if the function has been called for the first time since
   the sensor has been triggered. This is used for sending
   status information back to the joystick controller. */
int Motor::getCntFlag() {
  if (cntFlag == 1) {
    cntFlag = 0;
    return true;
  }
  
  return false;
}

/* calibrate() tests the linear actuators by moving
   the actuator from one end of its range to the other
   at both low speeds and high speeds, measuring the
   total rotations that the counter counts. */
int Motor::calibrate() {
  int totalRotations = 0;
  
  // move the carriage to the low position
  while (move(-170) != -1000);
  
  // then, move it back towards the high position (in calibration mode)
  while (move(170, 1) != 1000);
  
  Serial.println(rotations);
  totalRotations += rotations;
  rotations = 0;
  
  // count again, going back to the low position
  while (move(-170, 1) != -1000);
  
  Serial.println(rotations);
  totalRotations -= rotations;
  rotations = 0;
  
  // repeat the above again, this time at full speed
  while (move(255, 1) != 1000);
  
  Serial.println(rotations);
  totalRotations += rotations;
  rotations = 0;
  
  while (move(-255, 1) != -1000);
  
  Serial.println(rotations);
  totalRotations -= rotations;
 
  // return the average rotation count
  return (totalRotations/4);
}
