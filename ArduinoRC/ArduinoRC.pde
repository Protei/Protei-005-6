/* Arduino/Xbee RC Control
   Gabriella Levine and Logan Williams
   Protei
   6/29/2011
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

void setup() {
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  pinMode(3, INPUT);
  digitalWrite(3, HIGH);
  
  Serial.begin(9600); // initialize serial
}

void loop() {
  sensorValueJSL = analogRead(JOYSTICK_LEFT_IN); // read the left joystick
  outputValueJSL = mapWell(sensorValueJSL, 70, 1023, 0, 255); // map it to 8 bit values
  
  sensorValueJSR = analogRead(JOYSTICK_RIGHT_IN); // repeat for right joystick
  outputValueJSR = mapWell(sensorValueJSR, 0, 1012, 0, 255);
  
  if (digitalRead(2) == LOW && digitalRead(3) == HIGH) {
    outputValueButtons = 0;
  } else if (digitalRead(3) == LOW && digitalRead(2) == HIGH) {
    outputValueButtons = 255;
  } else {
    outputValueButtons = 127;
  }
  
  sendBytes(outputValueJSL, outputValueJSR, outputValueButtons); // transmit the values
  
  delay(100);
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
