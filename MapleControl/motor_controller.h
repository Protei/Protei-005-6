#include "motor.h"

class MotorController {
public:
  MotorController(); // constructor
  MotorController(Motor* theMotor, int theGain);
  void init(Motor* theMotor, int theGain);
  int runLoop();
  void setTarget(int newTarget);
  int getError();
  int getTarget();
private:
  int gain;
  Motor* motor;
  int targetPosition;
};

