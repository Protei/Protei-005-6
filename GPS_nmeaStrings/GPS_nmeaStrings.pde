#include <Wire.h>
#include <SD.h>
int ledPin = 13;                  // LED test pin to determine if we are successfully writing gps strings to a text file 
int ledState = LOW;

int rxPin = 0;                    // RX PIN 
int txPin = 1;                    // TX TX

const int chipSelect = 53; //chip select for Arduino mega
const int LOCATION_FILE_NUMBER_LSB = 0x00;
const int LOCATION_FILE_NUMBER_MSB = 0x01;

File dataFile; 

void setup() {
  pinMode(53, OUTPUT); //set the chip select to output 
  pinMode(ledPin, OUTPUT);       // Initialize LED pin
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Serial.begin(38400);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  Serial.println("Time, Status, Latitude, Direction, Longitude, Direction, Velocity, Heading, Date, Magnetic degrees, E/W, Checksum");
  delay(1000);

  // start the wire libraries:
  Wire.begin();

  dataFile = SD.open("GPS.txt", FILE_WRITE);//either create a text file, or open a previously 
  //existing file 
}

void loop() {
  if(Serial.available()>0){
    byte gps=Serial.read(); //echo incoming gps data 
  
  Serial.print(gps);
  if(dataFile){ //if a text file exists, begin writing the gps strings to the file
    dataFile.print(gps);
    digitalWrite(ledPin, HIGH);//turn on the status led
  }
        dataFile.flush();

}
else{digitalWrite(ledPin, LOW);}
}

