class Motor {
public:
  Motor(); // constructor
  Motor(int motor);
  void init(int motor);
  void brake();
  int move(int speed);
  int move(int speed, int calibrating);
  int getSpeed();
  int getDirection();
  int getRotations();
  int resetLimitLow();
  int resetLimitHigh();
  void count();
  int calibrate();
private:
  int motorNumber;
  int currentSpeed;
  int rotations;
  int lastCounted;
};

