/* Maple/Xbee RC Control (Reciever
   Logan Williams
   Protei
   6/30/2011
   
   This sketch reads serial data from the Xbee, and decodes it using
   Hamming(7,4) encoding. It will correct any single bit error, and
   throw away any multiple bit errors.
 */

// the two output pins
const int OUTPUT_1 = 0;
const int OUTPUT_2 = 14;

boolean debug = false;

void setup() {
  pinMode(OUTPUT_1, PWM);
  pinMode(OUTPUT_2, PWM);
  
  Serial1.begin(9600);
  SerialUSB.begin();
}

void loop() {
  char data1;
  char data2;
  
  // If the USB isn't being monitored, the maple will run slowly.
  // This sets a debug flag to true if the USB is monitored.
  if (SerialUSB.isConnected() && (SerialUSB.getDTR() || SerialUSB.getRTS())) {
    debug = true;
  } else {
    debug = false;
  }
  
  if (receive(&data1, &data2)) {
    pwmWrite(OUTPUT_1, map(data1, 0, 255, 0, 65535));
    pwmWrite(OUTPUT_2, map(data2, 0, 255, 0, 65535));
  }
}

// this function checks for data to receive. if there is data, it reads it into
// data1 and data2 (pointers passed from the main loop).
// returns true if data read successfully, false otherwise
boolean receive(char *data1, char *data2) {
  char byteRead;
  char halfByte1A;
  char halfByte1B;
  char halfByte2A;
  char halfByte2B;

  if (Serial1.available() >  5) {
    byteRead = Serial1.read();
    
    if (byteRead == 'S') { // start byte recieved
      halfByte1A = Serial1.read();
      halfByte1B = Serial1.read();
      halfByte2A = Serial1.read();
      halfByte2B = Serial1.read();
      byteRead = Serial1.read();
    }
  }
  
  if (byteRead == 'E') { // end byte recieved
    if (debug) { SerialUSB.println("Recieved a complete packet. Data:"); }
    halfByte1A = hamming74Decode(halfByte1A);
    halfByte1B = hamming74Decode(halfByte1B);
    halfByte2A = hamming74Decode(halfByte2A);
    halfByte2B = hamming74Decode(halfByte2B);
    
    if (halfByte1A == 0xFF || halfByte2A == 0xFF || halfByte1B == 0xFF || halfByte2B == 0xFF) {
      // multiple bit errors, recovery impossible
      if (debug) { SerialUSB.println("Multiple bit errors. Recovery impossible."); }
      
      return false;
    } else {
      *data1 = (halfByte1A & B00001111) + ((halfByte1B << 4) & B11110000);
      *data2 = (halfByte2A & B00001111) + ((halfByte2B << 4) & B11110000);
      
      if (debug) {
        SerialUSB.print("Data1: ");   
        SerialUSB.println(((int) *data1));
        SerialUSB.print("Data2: ");
        SerialUSB.println(((int) *data2));
      }
      
      return true;      
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
  
