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

/** CONSTANT DEFINITIONS */

// this needs to be measured and set manually
const int MAX_MOTOR_ROTATIONS[] = {
  100, 100, 100};

int dir = 1;

const int GAIN[] = {
  8000, 8000, 8000}; // the proportional gain

long ruptTimer = 0;

const int EN_PINS[] 	= {
  4, 5, 6};
const int RPWM_PINS[] 	= {
  0, 2, 11};
const int LPWM_PINS[] 	= {
  1, 3, 12};
const int LIMIT_A_PINS[] 	= {
  21, 22, 23};
const int LIMIT_B_PINS[] 	= {
  28, 37, 36};

int l;
int h;
void setup() {

    pinMode(21, INPUT_PULLUP);
    pinMode(28, INPUT_PULLUP);
  SerialUSB.begin();
  
  
}

void loop() {
    if (dir == 0) {
      pinMode(LPWM_PINS[0], OUTPUT);
      pinMode(RPWM_PINS[0], PWM);
      digitalWrite(LPWM_PINS[0], HIGH);
      pwmWrite(RPWM_PINS[0], 65535 - 65535);
    } else {
      pinMode(RPWM_PINS[0], OUTPUT);
      pinMode(LPWM_PINS[0], PWM);
      digitalWrite(RPWM_PINS[0], HIGH);
      pwmWrite(LPWM_PINS[0], 65535 - 65535);
    }
    
    //SerialUSB.println(digitalRead(21));
    //SerialUSB.print(".");
    
    l = digitalRead(21);
    h = digitalRead(28);
    
    if (l == 0) {
      dir = 1;
    }
    
    if (h == 0) {
      dir = 0;
    }

    
delay(20);
}
