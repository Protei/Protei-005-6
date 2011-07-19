/*
	Protei — Remote Control and Motor Control
    Copyright (C) 2011  Logan Williams, Gabriella Levine, Qiuyang Zhou

	This file is part of Protei.
	
	Protei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Protei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program (see COPYING).  If not, see <http://www.gnu.org/licenses/>.
*/

/** MOTOR DRIVING MACROS */

/* Function prototypes */

void motorWrite(int motor, int val); // writes val to the designated motor 
void motorBreak(int motor); // breaks the designated motor

/* End of prototypes */

//  motor - int between 0 and 2
//  val - between -65535 and 65535, the vaue we want to drive the motor at

void motorWrite(int motor, int val) {
  if (val > 0) {
    pinMode(RPWM_PINS[motor], OUTPUT);
    pinMode(LPWM_PINS[motor], PWM);
    digitalWrite(RPWM_PINS[motor], HIGH);
    pwmWrite(LPWM_PINS[motor], 65535 - (int) val);
  } 
  else {
    pinMode(RPWM_PINS[motor], PWM);
    pinMode(LPWM_PINS[motor], OUTPUT);
    digitalWrite(LPWM_PINS[motor], HIGH);
    pwmWrite(RPWM_PINS[motor], 0 - (int) val);
  }
}

// this function causes the motor to break
void motorBreak(int motor) {
  pinMode(RPWM_PINS[motor], OUTPUT);
  pinMode(LPWM_PINS[motor], OUTPUT);
  digitalWrite(RPWM_PINS[motor], HIGH);
  digitalWrite(LPWM_PINS[motor], HIGH);
  currentDrive[motor] = 0;
}
