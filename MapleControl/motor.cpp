#include "motor.h"
#include "consts.h"
#include "WProgram.h"

Motor::Motor() {
}

Motor::Motor(int motor) {
  init(motor);
}

void Motor::init(int motor) {
  motorNumber = motor;
  pinMode(EN_PINS[motorNumber], INPUT);
  rotations = 0;
  pinMode(ROT_PINS[motorNumber], INPUT);
}

int Motor::move(int speed) {
  move(speed, false);
}

int Motor::move(int speed, int calibrating) {
  currentSpeed = speed;
 
  int limitA = digitalRead(LIMIT_A_PINS[motorNumber]);
  int limitB = digitalRead(LIMIT_B_PINS[motorNumber]);
  
  if ((limitA == LOW) || (limitB == LOW)) {
    if ((limitA == LOW) && ((*this).getDirection() == 0)) {
      brake();
      if (0 == calibrating) {
        resetLimitLow();
      }
      return -100000; 
    } else if ((limitB == LOW) && ((*this).getDirection() == 1)) {
      brake();
      if (0 == calibrating) {
        resetLimitHigh();
      }
      return 100000;
    } else {
      pinMode(EN_PINS[motorNumber], OUTPUT);
      digitalWrite(EN_PINS[motorNumber], HIGH);
    }
  } else {
    pinMode(EN_PINS[motorNumber], INPUT);
  }
  
  if (speed == 0) {
    (*this).brake();
  } else if (speed > 0) {
    pinMode(RPWM_PINS[motorNumber], OUTPUT);
    pinMode(LPWM_PINS[motorNumber], PWM);
    digitalWrite(RPWM_PINS[motorNumber], HIGH);
    pwmWrite(LPWM_PINS[motorNumber], 65535 - speed);
  } else {
    pinMode(RPWM_PINS[motorNumber], PWM);
    pinMode(LPWM_PINS[motorNumber], OUTPUT);
    digitalWrite(LPWM_PINS[motorNumber], HIGH);
    pwmWrite(RPWM_PINS[motorNumber], 65535 + speed);
  }
  
  return speed;
  
}



void Motor::brake() {
  pinMode(RPWM_PINS[motorNumber], OUTPUT);
  pinMode(LPWM_PINS[motorNumber], OUTPUT);
  digitalWrite(RPWM_PINS[motorNumber], HIGH);
  digitalWrite(LPWM_PINS[motorNumber], HIGH);
  currentSpeed = 0;
}

int Motor::getSpeed() {
  return currentSpeed;
}

int Motor::getDirection() {
  if (currentSpeed >= 0) {
    return 1;
  } 
  else {
    return 0;
  }
}

int Motor::getRotations() {
  return rotations;
}

int Motor::resetLimitLow() {
  rotations = 0;
  return 0;
}

int Motor::resetLimitHigh() {
  rotations = MAX_MOTOR_ROTATIONS[motorNumber];
}

void Motor::count() {
  if (getDirection() == 1) {
    rotations++;
  } 
  else {
    rotations--;
  }
}

int Motor::calibrate() {
  int totalRotations = 0;
  
  while (move(-20000) != -100000);
  
  while (move(20000, 1) != 100000);
  
  totalRotations += rotations;
  rotations = 0;
  
  while (move(-20000, 1) != -100000);
  
  totalRotations -= rotations;
  rotations = 0;
  
  while (move(65535, 1) != 100000);
  
  totalRotations += rotations;
  rotations = 0;
  
  while (move(-65535, 1) != -100000);
  
  totalRotations -= rotations;
  
  return (totalRotations/4);
  
}

