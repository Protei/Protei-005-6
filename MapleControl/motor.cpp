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
  rotations = 50las;
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
int Motor::move(int speed, int calibrating) {
  currentSpeed = speed;
 
  int limitA = digitalRead(LIMIT_A_PINS[motorNumber]);
  int limitB = digitalRead(LIMIT_B_PINS[motorNumber]);
  
  if ((limitA == LOW) || (limitB == LOW)) {
    if ((limitA == LOW) && ((*this).getDirection() == 0)) {
      brake();
      if (0 == calibrating) {
        resetLimitLow();
      }
      return -100000; 
    } else if ((limitB == LOW) && ((*this).getDirection() == 1)) {
      brake();
      if (0 == calibrating) {
        resetLimitHigh();
      }
      return 100000;
    } else {
      pinMode(EN_PINS[motorNumber], OUTPUT);
      digitalWrite(EN_PINS[motorNumber], HIGH);
    }
  } else {
    pinMode(EN_PINS[motorNumber], INPUT);
  }
  
  if (speed > 1000) {
    pinMode(RPWM_PINS[motorNumber], OUTPUT);
    pinMode(LPWM_PINS[motorNumber], PWM);
    digitalWrite(RPWM_PINS[motorNumber], HIGH);
    pwmWrite(LPWM_PINS[motorNumber], 65535 - speed);
  } else if (speed < 1000) {
    pinMode(RPWM_PINS[motorNumber], PWM);
    pinMode(LPWM_PINS[motorNumber], OUTPUT);
    digitalWrite(LPWM_PINS[motorNumber], HIGH);
    pwmWrite(RPWM_PINS[motorNumber], 65535 + speed);
  } else {
    (*this).brake();
  }
  
  return speed;
}

/* brake() commands the motor driver to brake
   the motor. */
void Motor::brake() {
  pinMode(RPWM_PINS[motorNumber], OUTPUT);
  pinMode(LPWM_PINS[motorNumber], OUTPUT);
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
  if (currentSpeed >= 0) {
    return 1;
  } 
  else {
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
  else {
    rotations--;
  }
}

/* calibrate() tests the linear actuators by moving
   the actuator from one end of its range to the other
   at both low speeds and high speeds, measuring the
   total rotations that the counter counts. */
int Motor::calibrate() {
  int totalRotations = 0;
  
  while (move(-20000) != -100000);
  
  while (move(20000, 1) != 100000);
  
  totalRotations += rotations;
  rotations = 0;
  
  while (move(-20000, 1) != -100000);
  
  totalRotations -= rotations;
  rotations = 0;
  
  while (move(65535, 1) != 100000);
  
  totalRotations += rotations;
  rotations = 0;
  
  while (move(-65535, 1) != -100000);
  
  totalRotations -= rotations;
  
  return (totalRotations/4);
}

