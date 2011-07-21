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

/** Rotary Sensors */
#include "WProgram.h"
#include "rotary_sensor.h"

RotarySensor::RotarySensor(){
  
}

void RotarySensor::init(int pin) {
 numberOfRotations = 0;
 pinNumber = pin;
 pinMode(pinNumber, INPUT);
 
}

void RotarySensor::rotateCounter(int direction){
 if(direction ==1)
 numberOfRotations++;
 else 
 numberOfRotations--;
}

long RotarySensor::getCounter() {
 return numberOfRotations; 
}

void RotarySensor::resetCounter() {
 numberOfRotations = 0; 
}
