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

/* The Motor class creates an object for each motor
   on Protei. The Motor objects can track their rotations
   and the class contains functions for commanding
   rotation at specific speeds (open loop) and for
   limit switches */

class Motor {
public:
  Motor(); // constructor
  Motor(int motor);
  void init(int motor);
  void brake();
  int move(int speed);
  int move(int speed, int calibrating);
  int getSpeed();
  int getDirection();
  int getRotations();
  int resetLimitLow();
  int resetLimitHigh();
  void count();
  int calibrate();
private:
  int motorNumber; // the motor number (0=BOW, 1=STERN, 2=WINCH)
  int currentSpeed; // the current motor speed (output)
  int rotations; // the current # of rotations
};

