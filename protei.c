#define   BOW   0
#define   STERN 1
#define   WINCH 2

// motors are indexed as follow:
//  0 - bow articulation motor
//  1 - stern articulation motor
//  2 - sail whinch motor
const int MAX_MOTOR_ROTATIONS[] = {100, 100, 100};

// PID GAIN DEFINITIONS
// *** THESE HAVE NOT BEEN TUNED YET AND MIGHT BE UNSTABLE ***
const int K_PRO[] = {100, 100, 100}; // the proportional gain
const int K_DER[] = {1, 1, 1}; // the differential gain
const int K_INT[] = {1, 1, 1}; // the intergral gain

// PIN DEFINITIONS
// motor drivers
const int EN_PINS[] = {4, 5, 6};
const int RPWM_PINS[] = {0, 2, 11};
const int LPWM_PINS[] = {1, 3, 12};
// xbee
const int XBEE_TX_PIN = 7;
const int XBEE_RX_PIN = 8;

// motor feedback interrupt pins
const int ROT_PINS[] = {29, 30, 31};
const int LIMIT_A_PINS[] = {21, 22, 23};
const int LIMIT_B_PINS[] = {32, 33, 34};

// EXECUTION VARIABLES
boolean debug = false;
unsigned long lastExecuted;
unsigned long time;

volatile int motorRotations[3];
int desiredRotations[3];
int error[3];
int integratedError[3];
volatile int currentDrive[3];

void setup() {
  pinMode(EN_PINS[BOW], OUTPUT);
  pinMode(EN_PINS[STERN], OUTPUT);
  pinMode(EN_PINS[WINCH], OUTPUT);
  pinMode(XBEE_TX_PIN, OUTPUT);
  pinMode(XBEE_RX_PIN, INPUT);
  pinMode(ROT_PINS[BOW], INPUT);
  pinMode(ROT_PINS[STERN], INPUT);
  pinMode(ROT_PINS[WINCH], INPUT);
  pinMode(LIMIT_A_PINS[BOW], INPUT_PULLUP);
  pinMode(LIMIT_A_PINS[STERN], INPUT_PULLUP);
  pinMode(LIMIT_A_PINS[WINCH], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[BOW], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[STERN], INPUT_PULLUP);
  pinMode(LIMIT_B_PINS[WINCH], INPUT_PULLUP);
  
  digitalWrite(EN_PINS[BOW], HIGH); // enable the motor drivers
  digitalWrite(EN_PINS[STERN], HIGH);
  digitalWrite(EN_PINS[WINCH], HIGH);
  
  attachInterrupt(ROT_PINS[BOW], countBow, RISING); // add interrupts for the sensors
  attachInterrupt(ROT_PINS[STERN], countStern, RISING);
  attachInterrupt(ROT_PINS[WINCH], countWinch, RISING);
  attachInterrupt(LIMIT_A_PINS[BOW], resetBow, FALLING);
  attachInterrupt(LIMIT_A_PINS[STERN], resetStern, FALLING);
  attachInterrupt(LIMIT_A_PINS[WINCH], resetWinch, FALLING);
  attachInterrupt(LIMIT_B_PINS[BOW], resetBow, FALLING);
  attachInterrupt(LIMIT_B_PINS[STERN], resetStern, FALLING);
  attachInterrupt(LIMIT_B_PINS[WINCH], resetWinch, FALLING);
  interrupts(); // start the interrupts
  
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
      desiredRotations[0] = map(data2, 0, 255, 0, MAX_MOTOR_ROTATIONS[0]);
      // banana shape
      desiredRotations[1] = MAX_MOTOR_ROTATIONS[1] - map(data2, 0, 255, 0, MAX_MOTOR_ROTATIONS[1]);
      desiredRotations[2] = map(data1, 0, 255, 0, MAX_MOTOR_ROTATIONS[2]);
    }
    
    // run the PID controllers
    motorWrite(BOW, PID(BOW));
    motorWrite(STERN, PID(STERN));
    motorWrite(WINCH, PID(WINCH));
  }
}

/** MOTOR DRIVING MACROS */
//  motor - int between 0 and 2
//  val - between -65535 and 65535, the vaue we want to drive the motor at
void motorWrite(int motor, int val) {
  if (val > 0) {
    pinMode(RPWM_PINS[motor], OUTPUT);
    pinMode(LPWM_PINS[motor], PWM);
    digitalWrite(RPWM_PINS[motor], HIGH);
    pwmWrite(LPWM_PINS[motor], 65535 - (int) val);
  } else {
    pinMode(RPWM_PINS[motor], PWM);
    pinMode(LPWM_PINS[motor], OUTPUT);
    digitalWrite(LPWM_PINS[motor], HIGH);
    pwmWrite(RPWM_PINS[motor], 0 - (int) val);
  }
}

// this function causes the motor to break
void motorBreak(int motor) {
  pinMode(RPWM_PINS[motor], OUTPUT);
  pinMode(LPWM_PINS[motor], OUTPUT);
  digitalWrite(RPWM_PINS[motor], HIGH);
  digitalWrite(LPWM_PINS[motor], HIGH);
  currentDrive[motor] = 0;
}

/*** PID Controllers
The following functions calculate the error between the current motor position
and the desired motor position, then drive the motor to reach the desired motor
position, using a PID (Proportional + Integral + Differential) control loop to adjust
the output intensity */
long PID(int motor) {
  long output; // the output var
  int lastError = error[motor];
  int diffError;
  
  error[motor] = desiredRotations[motor] - motorRotations[motor]; // the current error
  integratedError[motor] += error[motor];
  diffError = error[motor] - lastError;
  
  output = K_PRO[motor] * error[motor] + K_INT[motor] * integratedError[motor] + K_DER[motor] * diffError;
  
  if (output > 65535) {
    output = 65535;
  } else if (output < -65535) {
    output = -65535;
  }
  
  return output;
}

/** INTERRUPT SERVICE ROUTINES */
void countBow() {
  count(BOW);
}

void countStern() {
  count(STERN);
}

void countWinch() {
  count(WINCH);
}

void count(int motor) {
  // what direction are we turning
  if (currentDrive[motor] > 0) {
    motorRotations[motor]++;
  } else {
    motorRotations[motor]--;
  }
}

void resetCount(int motor) {
  motorBreak(motor);
  
  if (digitalRead(LIMIT_A_PINS[motor]) == LOW) {
    motorRotations[motor] = 0;
  } else {
    motorRotations[motor] = MAX_MOTOR_ROTATIONS[motor];
  }
}

void resetBow() {
  resetCount(BOW);
}

void resetStern() {
  resetCount(STERN);
}

void resetWinch() {
  resetCount(WINCH);
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