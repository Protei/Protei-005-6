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

/*  The MotorController class creates an object for closed loop
    position control of the linear actuators on Protei. */
    
class MotorController {
public:
  MotorController(); // constructor
  void init(Motor* theMotor, int theGain);
  int runLoop();
  void setTarget(int newTarget);
  int getError();
  int getTarget();
  void printDebug();
private:
  int gain; // the proportional gain
  Motor* motor; // a reference to the motor that is controlled
  int targetPosition; // the desired motor rotations
  int lastRotations;
  int consecutive;
  int avg;
};

