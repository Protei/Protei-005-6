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

boolean receive(unsigned char *data1, unsigned char *data2, unsigned char *data3); // checks for data and reads it into data1 and data2
boolean sendStatus(int bowCntFlag, int sternCntFlag); // sends current status information to the transmitter

char hamming74Decode(char halfByte); // decodes bits. Returns 0xFF in case of error.
char hamming74Encode(char halfByte); // encodes a 4 bit word

/* End of prototypes */

// this function checks for data to receive. if there is data, it reads it into
// data1 and data2 (pointers passed from the main loop).
// returns true if data read successfully, false otherwise
boolean receive(unsigned char *data1, unsigned char *data2, unsigned char *data3) {
  // create some variables for storing the data as we process it
  unsigned char byteRead;
  unsigned char halfByte1A;
  unsigned char halfByte1B;
  unsigned char halfByte2A;
  unsigned char halfByte2B;
  unsigned char halfByte3A;
  unsigned char halfByte3B;
  unsigned char buffer[128];
  
  int i = 0;
  int start;
  int avail = Serial1.available();
  
  if (avail > 7) {
    if (debug > 5) {
      Serial.print("Read into buffer: ");
    }
    
    // read all available data (up to 128 bits, then buffer empties
    // and restarts) into a buffer
    for (i = 0; (i < avail) && (i < 128); i++) {
      buffer[i] = Serial1.read();
      if (debug > 5) {
        Serial.write(buffer[i]);
      }
    }
    
    if (debug > 5) {
      Serial.println();
    }
  
    // search buffer for the start byte
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
          Serial.println("Recieved a complete packet. Data:"); 
        }
        
        // decode the half bytes that we have read
        halfByte1A = hamming74Decode(halfByte1A);
        halfByte1B = hamming74Decode(halfByte1B);
        halfByte2A = hamming74Decode(halfByte2A);
        halfByte2B = hamming74Decode(halfByte2B);
        halfByte3A = hamming74Decode(halfByte3A);
        halfByte3B = hamming74Decode(halfByte3B);
      
        // check if any of the decode processes returned a failure
        if (halfByte1A == 0xFF || halfByte2A == 0xFF || halfByte1B == 0xFF || halfByte2B == 0xFF || halfByte3A == 0xFF || halfByte3B == 0xFF) {
          // multiple bit errors, recovery impossible
          if (debug) { 
            Serial.println("Multiple bit errors. Recovery impossible."); 
          }
      
          return false;
        } 
        else {
          // reconstruct the original bytes from the half bytes
          *data1 = (halfByte1A & B00001111) + ((halfByte1B << 4) & B11110000);
          *data2 = (halfByte2A & B00001111) + ((halfByte2B << 4) & B11110000);
          *data3 = (halfByte3A & B00001111) + ((halfByte3B << 4) & B11110000);
      
          if (debug) {
            Serial.print("Data1: ");   
            Serial.println(((int) *data1));
            Serial.print("Data2: ");
            Serial.println(((int) *data2));
            Serial.print("Data3: ");
            Serial.println(((int) *data3));
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

/* This function sends current status information from the
   motor controllers to the transmitter. It accepts two
   arguments, the current values of the count flags for
   the bow and the stern. These can be found by calling
   Motor::getCntFlag() */
boolean sendStatus(int bowCntFlag, int sternCntFlag) {
  unsigned char toSend1;
  unsigned char toSend2;
  
  // construct the two half bytes
  toSend1 = 0x00;
  toSend2 = 0x00;
  toSend1 += (digitalRead(LIMIT_A_PINS[BOW]) << 3);
  toSend1 += (bowCntFlag << 2);
  toSend1 += (digitalRead(LIMIT_B_PINS[BOW]) << 1);
  toSend1 += (digitalRead(LIMIT_A_PINS[STERN]) << 0);
  toSend2 += (sternCntFlag << 1);
  toSend2 += (digitalRead(LIMIT_B_PINS[STERN]) << 0);
  
  // send the data, with a slight delay so that the
  // Arduino on the other end can recieve successfully
  // (the delay might be unnecessary)
  Serial1.write('S');
  delay(1);
  Serial1.write(hamming74Encode(toSend1));
  delay(1);
  Serial1.write(hamming74Encode(toSend2));
  delay(1);
  Serial1.write('E');
  
  return true;
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

// converts a 4 bit halfbyte to its 8 bit hamming encoded version
// I know, it's a lookup table. whatever, I'll fix it if I have to
char hamming74Encode(char halfByte) {
  halfByte = halfByte & B00001111;
  
  switch(halfByte) {
    case B0000:
      return B00000000;
    case B0001:
      return B00010111;
    case B0010:
      return B00101011;
    case B0011:
      return B00111100;
    case B0100:
      return B01001101;
    case B0101:
      return B01011010;
    case B0110:
      return B01100110;
    case B0111:
      return B01110001;
    case B1000:
      return B10001110;
    case B1001:
      return B10011001;
    case B1010:
      return B10100101;
    case B1011:
      return B10110010;
    case B1100:
      return B11000011;
    case B1101:
      return B11010100;
    case B1110:
      return B11101000;
    case B1111:
      return B11111111;
    return B0000;
  }
}
