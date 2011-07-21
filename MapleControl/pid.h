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

/** PID Controllers */

/* Function prototypes */

long PID(int motor); // computes the motor command according to a standard PID control loop.

/* End of prototypes */

// The following function calculate the error between the current motor position
// and the desired motor position, then drive the motor to reach the desired motor
// position, using a PID (Proportional + Integral + Differential) control loop to adjust
// the output intensity

long PID(int motor) {
  long output; // the output var
  int lastError = error[motor];
  int diffError;
  
  error[motor] = desiredRotations[motor] - motorRotations[motor]; // the current error
  integratedError[motor] += error[motor];
  if (integratedError[motor] > 655)     //  saturation of the integration value
        integratedError[motor] = 655;
  else if (integratedError[motor] < -655)
             integratedError[motor] = -655;
      
      
  diffError = (error[motor] - lastError) * 10; // (10 is the loop period)
  
  output = K_PRO[motor] * error[motor] + K_INT[motor] * integratedError[motor] + K_DER[motor] * diffError;
  
  if (abs(error[motor]) < 5) {
    output = 0;
  }
  
  if(output > 0) {
    motorDirection[motor]  = 1;
  } else {
    motorDirection[motor]  = 0;
  }
         
  if (output > 65535) {
    output = 65535;
  } else if (output < -65535) {
    output = -65535;
  } else if (abs(output) < 4000) {
    output = 0;
  }
  
  if(debug &&  (usbElapsCounter >= usbDebugRate))  {
    
     SerialUSB.print(error[motor]);
     SerialUSB.print('\t');
     SerialUSB.print(integratedError[motor]);
     SerialUSB.print('\t');
     SerialUSB.print(diffError);
     SerialUSB.print('\t');   
    SerialUSB.print(output); }
   return output;
}

