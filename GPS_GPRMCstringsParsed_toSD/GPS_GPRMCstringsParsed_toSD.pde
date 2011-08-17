#include <Wire.h>
#include <SD.h>
#include <string.h>
#include <ctype.h>
int ledPin = 13;                  // LED test pin
int ledState = LOW;
long previousMillise = 0;
long interval = 1000;
long previousMillis = 0;
const int ledPin12 = 12;
int rxPin = 0;                    // RX PIN 
int txPin = 1;                    // TX TX
int byteGPS=-1;
char linea[300] = "";
char comandoGPR[7] = "$GPRMC";
int cont=0;
int bien=0;
int conta=0;
int indices[13];
const int chipSelect = 53;//changed from 8
const int LOCATION_FILE_NUMBER_LSB = 0x00;
const int LOCATION_FILE_NUMBER_MSB = 0x01;
File dataFile;
void setup() {
  pinMode(53, OUTPUT);
  pinMode(ledPin12, OUTPUT);
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

  // start the wire and RTC libraries:
  Wire.begin();

  //newlog();
  dataFile = SD.open("NewGPS.txt", FILE_WRITE);
  //Serial.println("finishSetup");
  delay(20);

  for (int i=0;i<300;i++){       // Initialize a buffer for received data
    linea[i]=' ';
  }   
}
void loop() {
  digitalWrite(ledPin, HIGH);
  byteGPS=Serial.read();         // Read a byte of the serial port
  if (byteGPS == -1) {           // See if the port is empty yet
    delay(100); 
  } 
  else {
    linea[conta]=byteGPS;        // If there is serial port data, it is put in the buffer
    conta++;                      
    // Serial.print(byteGPS, BYTE); 
    // dataFile.print(byteGPS, BYTE);
    if (byteGPS==13){            // If the received byte is = to 13, end of transmission
      digitalWrite(ledPin, LOW); 
      cont=0;
      bien=0;
      for (int i=1;i<7;i++){     // Verifies if the received command starts with $GPR
        if (linea[i]==comandoGPR[i-1]){
          bien++;
        }
      }
      if(bien==6){               // If yes, continue and process the data
        for (int i=0;i<300;i++){
          if (linea[i]==','){    // check for the position of the  "," separator
            indices[cont]=i;
            cont++;
          }
          if (linea[i]=='*'){    // ... and the "*"
            indices[12]=i;
            cont++;
          }
        }
        Serial.println("");      // ... and write to the serial port

        for (int i=0;i<12;i++){
          switch(i){

          }
          for (int j=indices[i];j<(indices[i+1]-1);j++){
            Serial.print(linea[j+1]); 
          }
          Serial.print(",");

        }
        // Serial.println("---------------");
        Serial.println();

        if(dataFile)
        {
          //dataFile.println("");      // ... and write to the serial port
    digitalWrite(ledPin12, HIGH);
          dataFile.println();

          for (int i=0;i<12;i++){
            switch(i){
              //             case 0 :dataFile.print("Time in UTC (HhMmSs): ");break;
              //             case 1 :dataFile.print("Status (A=OK,V=KO): ");break;
              //             case 2 :dataFile.print("Latitude: ");break;
              //             case 3 :dataFile.print("Direction (N/S): ");break;
              //             case 4 :dataFile.print("Longitude: ");break;
              //             case 5 :dataFile.print("Direction (E/W): ");break;
              //             case 6 :dataFile.print("Velocity in knots: ");break;
              //             case 7 :dataFile.print("Heading in degrees: ");break;
              //             case 8 :dataFile.print("Date UTC (DdMmAa): ");break;
              //             case 9 :dataFile.print("Magnetic degrees: ");break;
              //             case 10 :dataFile.print("(E/W): ");break;
              //             case 11 :dataFile.print("Mode: ");break;
              //             case 12 :dataFile.print("Checksum: ");break;
            }
            for (int j=indices[i];j<(indices[i+1]-1);j++){
              dataFile.print(linea[j+1]); 
            }
            dataFile.print(",");
          }
          dataFile.println("");


        } 
        else{
          digitalWrite(ledPin12, LOW);
        }


        dataFile.flush();
        //Serial.flush();

      }
      conta=0;                    // Reset the buffer
      for (int i=0;i<300;i++){    //  
        linea[i]=' ';             
      }                 
    }
  }
}

