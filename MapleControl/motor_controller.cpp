#include "motor_controller.h"
#include "consts.h"
#include "WProgram.h"

MotorController::MotorController() {
}

void MotorController::init(Motor*  theMotor, int theGain) {
  gain = theGain;
  motor = theMotor;
  targetPosition = 0;
}

int MotorController::runLoop() {
  int output;
  int error = targetPosition - (*motor).getRotations();

  if (abs(error) < 3) {
    output = 0;
  } 
  else {
    output = gain * error;
    if (output > 65535) {
      output = 65535;
    } 
    else if (output < -65535) {
      output = -65535;
    }
  }

  (*motor).move(output);
  return(output);
}

void MotorController::setTarget(int newTarget) {
  targetPosition = newTarget;
}

int MotorController::getError() {
  return targetPosition - (*motor).getRotations();
}

int MotorController::getTarget() {
  return targetPosition;
}
