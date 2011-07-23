#include "motor.h"
#include "consts.h"
#include "WProgram.h"

Motor::Motor() {
}

Motor::Motor(int motor) {
  this->init(motor);
}

void Motor::init(int motor) {
  motorNumber = motor;
  pinMode(EN_PINS[motorNumber], OUTPUT);
  rotations = 0;
  pinMode(ROT_PINS[motorNumber], INPUT);
}

void Motor::move(int speed) {
  currentSpeed = speed;
  int limitA = digitalRead(LIMIT_A_PINS[motorNumber]);
  int limitB = digitalRead(LIMIT_B_PINS[motorNumber]);
  
  if ((limitA == LOW) && ((*this).getDirection() == 0)) {
    brake();
    resetLimitLow();
  } else if ((limitB == LOW) && ((*this).getDirection() == 1)) {
    brake();
    resetLimitHigh();
    //SerialUSB.println(rotations);
  } else {
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
  }
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

