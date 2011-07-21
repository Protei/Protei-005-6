class Motor {
public:
  Motor(); // constructor
  Motor(int motor);
  void init(int motor);
  void brake();
  void move(int speed);
  int getSpeed();
  int getDirection();
  int getRotations();
  int resetLimitLow();
  int resetLimitHigh();
  void count();
private:
  int motorNumber;
  int currentSpeed;
  int rotations;
};

