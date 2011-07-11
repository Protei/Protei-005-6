// PID GAIN DEFINITIONS
// *** THESE HAVE NOT BEEN TUNED YET AND MIGHT BE UNSTABLE ***
const int K_PRO_HOR = 100; // the proportional gain
const int K_DER_HOR = 1; // the differential gain
const int K_INT_HOR = 1; // the intergral gain
const int K_PRO_VER = 100; // the same, but for the vertical articulation motor
const int K_DER_VER = 1;
const int K_INT_VER = 1;

// PIN DEFINITIONS
const int R_PWM_H = 2; // Motor driver PWM pins
const int L_PWM_H = 3;
const int R_PWM_V = 0;
const int R_PWM_V = 6;
const int EN_H = 4; // motor drive enable pins
const int EN_V = 10;
const int XBEE_RX = 8; // xbee pins
const int XBEE_TX = 7;
const int H_ROT = 32; // sensing pins, the rotation and the limit switch
const int V_ROT = 33;
const int H_LIMIT = 23;
const int V_LIMIT = 34;

boolean debug = false;
unsigned long lastExecuted;
unsigned long time;

unsigned int hArtMotorRotations;
unsigned int vArtMotorRotations;
unsigned int hArtDesiredRotations;
unsigned int vArtDesiredRotations;
int hArtError;
int vArtError;
int hTotalError;
int vTotalError;
int currentHOut;
int currentVOut;

void setup() {
  pinMode(XBEE_RX, INPUT);
  pinMode(XBEE_TX, OUTPUT);
  pinMode(H_ROT, INPUT);
  pinMode(V_ROT, INPUT);
  pinMode(H_LIMIT, INPUT);
  pinMode(V_LIMIT, INPUT);
  pinMode(EN_H, OUTPUT);
  pinMode(EN_V, OUTPUT);
  
  digitalWrite(EN_H, HIGH); // enable the motor drivers
  digitalWrite(EN_V, HIGH);
  
  attachInterrupt(H_ROT, countH, CHANGE); // add interrupts for the sensors
  attachInterrupt(V_ROT, countV, CHANGE);
  attachInterrupt(H_LIMIT, resetH, FALLING);
  attachInterrupt(V_LIMIT, resetV, FALLING);
  interrupts(); // start the interrupts
  
  // get the timer ready to help with the hardware interrupts
  Timer3.setChannel1Mode(TIMER_OUTPUTCOMPARE);
  Timer3.setPeriod(10000);
  Timer3.setCompare1(1);
  Timer3.attachInterrupt(1, beginExtInts);
  Timer3.pause();
  
  Serial1.begin(9600); // begin Xbee serial comms
  SerialUSB.begin(); // begin USB serial comms
}

/*** THE MAIN LOOP ***
The loop body executes every 50ms, for a 20Hz control loop. This is kept in time with millis().
*/
void loop() {
  char data1;
  char data2;
  
  time = millis();
  
  // The main control loop executes every 50 ms
  if (((time - lastExecuted) > 50) || ((time - lastExecuted) < 0)) {
    lastExecuted = time; // update our time watch
    
    debug = usbActive();
    
    // If we have received new data from the Xbee, use it to update desired position
    if (receive(&data1, &data2)) {
      hArtDesiredRotations = data1;
      vArtDesiredRotations = data2;
    }
    
    // run the PID controllers
    PIDHart();
    PIDVart();
  }
}

/** MOTOR DRIVING MACRO *//
void motorWrite(int pinR, int pinL, int val) {
  if (val > 0) {
    pinMode(pinR, OUTPUT);
    pinMode(pinL, PWM);
    digitalWrite(pinR, HIGH);
    pwmWrite(pinL, 65535 - (int) val);
  } else {
    pinMode(pinR, PWM);
    pinMode(pinL, OUTPUT);
    digitalWrite(pinL, HIGH);
    pwmWrite(pinR, 0 - (int) val);
  }
}

/*** PID Controllers
The following two functions calculate the error between the current motor position
and the desired motor position, then drive the motor to reach the desired motor
position, using a PID (Proportional + Integral + Differential) control loop to adjust
the output intensity */
void PIDHArt() {
  long output; // the output variable
  int lastError = hArtError;
  hArtError = hArtDesiredRotations - hArtMotorRotations; // the current error 
  hTotalError += hArtError; // the integrated error
  
  output = K_PRO_HOR * hArtError + K_DER_HOR * (hArtError - lastError) + K_INT_HOR * hTotalError;
  
  if (output > 65535) {
    output = 65535;
  } else if (output < -65535) {
    output = -65535;
  }
  
  currentHOut = output;
  motorWrite(R_PWM_H, L_PWM_H, output); // write the output
}

void PIDVArt() {
  long output;
  int lastError = vArtError;
  vArtError = vArtDesiredRotations - vArtMotorRotations;
  vTotalError += vArtError;
  
  output = K_PRO_VER * vArtError + K_DER_VER * (vArtError - lastError) + K_INT_VER * vTotalError;
  
  if (output > 65535) {
    output = 65535;
  } else if (output < -65535) {
    output = -65535;
  }
  
  currentVOut = output;
  
  motorWrite(R_PWM_V, L_PWM_V, output);
}

/** INTERRUPT SERVICE ROUTINES */
void countH() {
  if (currentHOut > 0) {
    hArtMotorRotations++;
  } else {
    hArtMotorRotations--;
  }
  
  detachInterrupt(H_ROT);
  Timer3.setCount(0);
  Timer3.resume();
}

void countV() {
  if (currentVOut > 0) {
    vArtMotorRotations++;
  } else {
    vArtMotorRotations--;
  }
  
  detachInterrupt(V_ROT);
  Timer3.setCount(0);
  Timer3.resume();
}


void resetH() {
  // reset the rotations count
  hArtMotorRotations = 0;
  
  // stop the motor;
  motorWrite(R_PWM_H, L_PWM_H, 0);
  currentHOut = 0;
}

void resetV() {
  // reset the rotations count
  vArtMotorRotations = 0;
  
  // stop the motor
  motorWrite(R_PWM_V, L_PWM_V, 0);
  currentVOut = 0;
}

void beginExtInts() {
  attachInterrupt(H_ROT, countH, CHANGE);
  attachInterrupt(V_ROT, countV, CHANGE);
  Timer3.pause();
}

/*** COMMUNICATION SUBROUTINES */

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

boolean usbActive() {
  return (SerialUSB.isConnected() && (SerialUSB.getDTR() || SerialUSB.getRTS()));
}