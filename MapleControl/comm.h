/*
	Protei — Remote Control and Motor Control
 Copyright (C) 2011  Logan Williams, Gabriella Levine,
                     Qiuyang Zhou, Francois de la Taste
 
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

/** COMMUNICATION SUBROUTINES */

/* Function prototypes */

#include "WProgram.h"

boolean receive(char *data1, char *data2, char *data3); // checks for data and reads it into data1 and data2
char hamming74Decode(char halfByte); // decodes bits. Returns 0xFF in case of error.
boolean usbActive(); // check if USB is active

/* End of prototypes */

// this function checks for data to receive. if there is data, it reads it into
// data1 and data2 (pointers passed from the main loop).
// returns true if data read successfully, false otherwise

boolean receive(char *data1, char *data2, char *data3) {
  boolean debug = usbActive();
  char byteRead;
  char halfByte1A;
  char halfByte1B;
  char halfByte2A;
  char halfByte2B;
  char halfByte3A;
  char halfByte3B;
  char buffer[128];
  int i = 0;
  int start;
  int avail = Serial1.available();
  
  if (avail > 7) {
    if (debug > 5) {
      SerialUSB.print("Read into buffer: ");
    }
    for (i = 0; (i < avail) && (i < 128); i++) {
      buffer[i] = Serial1.read();
      if (debug > 5) {
        SerialUSB.write(buffer[i]);
      }
    }
    if (debug > 5) {
      SerialUSB.println();
    }
  
    for (int j = 0; j < i; j++) {
      if (buffer[j] == 'S') {
        start = j;
      }
    }
    
    if (start+7 <= i) {
      halfByte1A = buffer[start+1];
      halfByte1B = buffer[start+2];
      halfByte2A = buffer[start+3];
      halfByte2B = buffer[start+4];
      halfByte3A = buffer[start+5];
      halfByte3B = buffer[start+6];
      byteRead = buffer[start+7];
          
      if (byteRead == 'E') { // end byte recieved successfully
        Serial1.flush();
        if (debug) { 
          SerialUSB.println("Recieved a complete packet. Data:"); 
        }
        halfByte1A = hamming74Decode(halfByte1A);
        halfByte1B = hamming74Decode(halfByte1B);
        halfByte2A = hamming74Decode(halfByte2A);
        halfByte2B = hamming74Decode(halfByte2B);
        halfByte3A = hamming74Decode(halfByte3A);
        halfByte3B = hamming74Decode(halfByte3B);
      
        if (halfByte1A == 0xFF || halfByte2A == 0xFF || halfByte1B == 0xFF || halfByte2B == 0xFF || halfByte3A == 0xFF || halfByte3B == 0xFF) {
          // multiple bit errors, recovery impossible
          if (debug) { 
            SerialUSB.println("Multiple bit errors. Recovery impossible."); 
          }
      
          return false;
        } 
        else {
          *data1 = (halfByte1A & B00001111) + ((halfByte1B << 4) & B11110000);
          *data2 = (halfByte2A & B00001111) + ((halfByte2B << 4) & B11110000);
          *data3 = (halfByte3A & B00001111) + ((halfByte3B << 4) & B11110000);
      
          if (debug) {
            SerialUSB.print("Data1: ");   
            SerialUSB.println(((int) *data1));
            SerialUSB.print("Data2: ");
            SerialUSB.println(((int) *data2));
            SerialUSB.print("Data3: ");
            SerialUSB.println(((int) *data3));
          }
      
          return true;      
        }
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

// This function takes a byte that has 4 data bits, 3 Hamming(7,4) parity bits and
// one overall parity bit, in that order (LSB is the overall parity bit), and decodes
// it into the original 4 data bits, correcting any single bit error. It then returns
// a 4 bit char. In the event of unrecoverable bit errors, this function returns 0xFF

char hamming74Decode(char halfByte) {
  int bits[8];
  int e1, e2, e3;
  char syndrome;
  char correctedHalfByte;
  char parity;

  // load the bit array with each of the bits in halfByte
  for (int i = 0; i < 8; i++) {
    bits[i] = halfByte & B00000001;
    halfByte = halfByte >> 1;
  }

  // compute the syndrome bits
  e1 = (bits[4] + bits[5] + bits[7] + bits[1]) % 2;
  e2 = (bits[4] + bits[6] + bits[7] + bits[2]) % 2;
  e3 = (bits[5] + bits[6] + bits[7] + bits[3]) % 2;

  syndrome = (e3 << 2) + (e2 << 1) + e1;

  // fix the problems detected by the syndrome bits
  switch (syndrome) {
  case B000:
    break;
  case B001:
    bits[1] = ~bits[1] & B00000001;
    break;
  case B010:
    bits[2] = ~bits[2] & B00000001;
    break;
  case B011:
    bits[4] = ~bits[4] & B00000001;
    break;
  case B100:
    bits[3] = ~bits[3] & B00000001;
    break;
  case B101:
    bits[5] = ~bits[5] & B00000001;
    break;
  case B110:
    bits[6] = ~bits[6] & B00000001;
    break;
  case B111:
    bits[7] = ~bits[7] & B00000001;
    break;
  }

  // compute the overall parity bit
  parity = (bits[7] + bits[1] + bits[2] + bits[3] + bits[4] + bits[5] + bits[6]) % 2;

  // check for multiple bit errors
  if (parity & B00000001 != bits[0] & B0000001) { // more than one bad bit, we can't recover errors
    return 0xFF; // Every other returned value will be only be 4 bits, so this error value is
    // easily detectable
  }

  correctedHalfByte = ((bits[7] << 3) + (bits[6] << 2) + (bits[5] << 1) + bits[4]) & B00001111;

  return correctedHalfByte;
}

boolean usbActive() {
  return (SerialUSB.isConnected() && (SerialUSB.getDTR() || SerialUSB.getRTS()));
}

