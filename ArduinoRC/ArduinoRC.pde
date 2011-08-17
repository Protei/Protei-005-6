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
 
// system constants
const int JOYSTICK_LEFT_IN = 0;
const int JOYSTICK_RIGHT_IN = 1;

// sketch variables
int sensorValueJSL;
int outputValueJSL;
int sensorValueJSR;
int outputValueJSR;
int outputValueButtons;

int toggle = HIGH;

int inc = 4;

void setup() {
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  pinMode(3, INPUT);
  digitalWrite(3, HIGH);
  
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  
  digitalWrite(10, HIGH); // power
  
  Serial.begin(9600); // initialize serial
}

void loop() {
  unsigned char senseData;
  int limitABOW = 0;
  int limitBBOW = 0;
  int rotBOW = 0;
  int limitASTERN = 0;
  int limitBSTERN = 0;
  int rotSTERN = 0;
  
    if (receive(&senseData)) {
    limitBSTERN = senseData & B00010000;
    rotSTERN = senseData & B00100000;
    limitABOW = senseData & B00001000;
    rotBOW = senseData & B00000100;
    limitBBOW = senseData & B00000010;
    limitASTERN = senseData & B00000001;
    
    digitalWrite(5, limitASTERN);
    digitalWrite(6, limitBSTERN);
    digitalWrite(7, rotSTERN);
    digitalWrite(8, limitBBOW);
    digitalWrite(9, rotBOW);
    digitalWrite(11, limitABOW);
    digitalWrite(4, toggle);
    
    if (toggle == HIGH) {
      toggle = LOW;
    } else {
      toggle = HIGH;
    }
  }
  
  sensorValueJSL = analogRead(JOYSTICK_LEFT_IN); // read the left joystick
  outputValueJSL = mapWell(sensorValueJSL, 70, 1023, 0, 255); // map it to 8 bit values
  
  sensorValueJSR = analogRead(JOYSTICK_RIGHT_IN); // repeat for right joystick
  outputValueJSR = mapWell(sensorValueJSR, 0, 1012, 0, 255);
  
  if (digitalRead(2) == LOW && digitalRead(3) == HIGH) {
    outputValueButtons = 22;
  } else if (digitalRead(3) == LOW && digitalRead(2) == HIGH) {
    outputValueButtons = 232;
  } else {
    outputValueButtons = 127;
  }
  
  sendBytes(outputValueJSL, outputValueJSR, outputValueButtons); // transmit the values
  

  
  delay(100);
  
    
}

boolean receive(unsigned char *data1) {
  unsigned char byteRead;
  unsigned char halfByte1A;
  unsigned char halfByte1B;
  unsigned char buffer[128];
  unsigned char toSend1;
  unsigned char toSend2;
  int i = 0;
  int start;
  int avail = Serial.available();

  if (avail > 8) {
    for (i = 0; (i < avail) && (i < 128); i++) {
      buffer[i] = Serial.read();
    }
  
    for (int j = 0; j < i; j++) {
      if (buffer[j] == 'S') {
        start = j;
      }
    }
    
    if (start+3 <= i) {
      halfByte1A = buffer[start+1];
      halfByte1B = buffer[start+2];
      byteRead = buffer[start+3];
      
      
          
      if (byteRead == 'E') { // end byte recieved successfully
        halfByte1A = hamming74Decode(halfByte1A);
        halfByte1B = hamming74Decode(halfByte1B);
      
        if (halfByte1A == 0xFF || halfByte1B == 0xFF) {
          // multiple bit errors, recovery impossible
      
          return false;
        } 
        else {
          *data1 = (halfByte1A & B00001111) + ((halfByte1B << 4) & B11110000);
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
  
// TRANSMITS JOYSTICK VALUES
//  uses a Hamming(7,4) code with overall parity bit
//  splits two input bytes into 4 bit halfbytes
//  transmits:
//    - 'S' // start bit
//    - [Hamming encoded Halfbyte1A]
//    - ...
//    - 'E' // stop bit
void sendBytes(char byte1, char byte2, char byte3) {
  char halfByte1A = byte1 & B00001111;
  char halfByte1B = (byte1 >> 4) & B00001111;
  char halfByte2A = byte2 & B00001111;
  char halfByte2B = (byte2 >> 4) & B00001111;
  char halfByte3A = byte3 & B00001111;
  char halfByte3B = (byte3 >> 4) & B00001111;
  
  Serial.write('S');
  Serial.write(hamming74Encode(halfByte1A));
  Serial.write(hamming74Encode(halfByte1B));
  Serial.write(hamming74Encode(halfByte2A));
  Serial.write(hamming74Encode(halfByte2B));
  Serial.write(hamming74Encode(halfByte3A));
  Serial.write(hamming74Encode(halfByte3B));
  Serial.write('E');
  
  return;
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

int mapWell(int val, int lval, int hval, int lout, int hout) {
  if (val < lval) {
    return lout;
  } else if (val > hval) {
    return hout;
  } else {
    return map(val, lval, hval, lout, hout);
  }
}

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
