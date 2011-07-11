/*
Hook up to arduino as follows
Arduino Uno/Duemilanove     LSM303DLH Carrier
                    5V  ->  VIN
                   GND  ->  GND
          Analog Pin 5  ->  SCL
          Analog Pin 4  ->  SDA
*/


#include <Wire.h>
#include <LSM303DLH.h>

LSM303DLH compass;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  compass.enable();
}

void loop() {
  compass.read();

  Serial.print("A ");
  Serial.print("X: ");
  Serial.print(compass.a.x);
  Serial.print(" Y: ");
  Serial.print(compass.a.y);
  Serial.print(" Z: ");
  Serial.print(compass.a.z);

  Serial.print(" M ");  
  Serial.print("X: ");
  Serial.print(compass.m.x);
  Serial.print(" Y: ");
  Serial.print(compass.m.y);
  Serial.print(" Z: ");
  Serial.println(compass.m.z);
  
  delay(100);
}
