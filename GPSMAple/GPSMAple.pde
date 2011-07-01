#include <string.h>

char buffer[1024];
int bufferidx;

char byteIn;

long Lattitude = 0;
long Longitude = 0;
long Altitude =0 ;
long Ground_Speed=0;
long Ground_Course=0;
long Quality=0;
int NewData;
long Time;
long Fix;
long NumSats;
long HDOP;

int GPS_checksum;
boolean GPS_checksum_calc;

void setup() {
  Serial2.begin(38400);
  SerialUSB.begin();
  
  SerialUSB.println("test");
}

void loop() {
  char c;
  int numc;
  int i;
  
  numc = Serial2.available();
  if (numc > 0) {
    for (i=0; i < numc; i++) {
      c = Serial2.read();
      
      if (c == '$') { // NMEA start
        bufferidx = 0;
        buffer[bufferidx++] = c;
        GPS_checksum = 0;
        GPS_checksum_calc = true;
      } else if (c == '\r') { // NMEA end
        buffer[bufferidx++] = 0; // null terminate
        parse_nmea_gps();
        
        SerialUSB.print("Latitude: ");
        SerialUSB.println(Lattitude);
        SerialUSB.print("Longitude: ");
        SerialUSB.println(Longitude);
        
        SerialUSB.print("Ground speed: ");
        SerialUSB.println(Ground_Speed);
        
        SerialUSB.print("Altitude: ");
        SerialUSB.println(Altitude);
      } else {
        if (c== '*')
          GPS_checksum_calc = false;
        buffer[bufferidx++] = c;
        if (GPS_checksum_calc)
          GPS_checksum ^= c;
      }
    }
  }


}

void parse_nmea_gps(void)
{
  byte NMEA_check;
  long aux_deg;
  long aux_min;
  char *parseptr;
  
  if (bufferidx > 1022) { // buffer overflow, reset
    bufferidx = 0;
  }
  
  if (strncmp(buffer,"$GPGGA",6)==0){        // Check if sentence begins with $GPGGA
    if (buffer[bufferidx-4]=='*'){           // Check for the "*" character
      NMEA_check = parseHex(buffer[bufferidx-3])*16 + parseHex(buffer[bufferidx-2]);    // Read the checksums characters
      if (GPS_checksum == NMEA_check){      // Checksum validation
        //Serial.println("buffer");
		NewData = 1;  // New GPS Data
        parseptr = strchr(buffer, ',')+1;
        //parseptr = strchr(parseptr, ',')+1;
		Time = parsenumber(parseptr,2);          // GPS UTC time hhmmss.ss
		parseptr = strchr(parseptr, ',')+1;
		//
        aux_deg = parsedecimal(parseptr,2);      // degrees
        aux_min = parsenumber(parseptr+2,4);     // minutes (sexagesimal) => Convert to decimal
        Lattitude = aux_deg*10000000 + (aux_min*50)/3;   // degrees + minutes/0.6  (*10000000) (0.6 = 3/5)
        parseptr = strchr(parseptr, ',')+1;
		//
		if (*parseptr=='S')
		  Lattitude = -1*Lattitude;              // South Lattitudes are negative
		//
        parseptr = strchr(parseptr, ',')+1;
        // W Longitudes are Negative
        aux_deg = parsedecimal(parseptr,3);      // degrees
        aux_min = parsenumber(parseptr+3,4);     // minutes (sexagesimal)
        Longitude = aux_deg*10000000 + (aux_min*50)/3;  // degrees + minutes/0.6 (*10000000)

        parseptr = strchr(parseptr, ',')+1;
		//
		if (*parseptr=='W')
		  Longitude = -1*Longitude;              // West Longitudes are negative
		//
        parseptr = strchr(parseptr, ',')+1;
        Fix = parsedecimal(parseptr,1);
        parseptr = strchr(parseptr, ',')+1;
        NumSats = parsedecimal(parseptr,2);
        parseptr = strchr(parseptr, ',')+1; 
        HDOP = parsenumber(parseptr,1);          // HDOP * 10
        parseptr = strchr(parseptr, ',')+1;
        Altitude = parsenumber(parseptr,1)*100;  // Altitude in decimeters*100 = milimeters
		if (Fix < 1)
		  Quality = 0;      // No FIX
		else if(NumSats<5)
		  Quality = 1;      // Bad (Num sats < 5)
		else if(HDOP>30)
		  Quality = 2;      // Poor (HDOP > 30)
		else if(HDOP>25)
		  Quality = 3;      // Medium (HDOP > 25)
		else
		  Quality = 4;      // Good (HDOP < 25)
        }
	  else
	    {
	      SerialUSB.println("GPSERR: Checksum error!!");
	    }
      }
    }
  else if (strncmp(buffer,"$GPVTG",6)==0){        // Check if sentence begins with $GPVTG
    //Serial.println(buffer);
    if (buffer[bufferidx-4]=='*'){           // Check for the "*" character
      NMEA_check = parseHex(buffer[bufferidx-3])*16 + parseHex(buffer[bufferidx-2]);    // Read the checksums characters
      if (GPS_checksum == NMEA_check){      // Checksum validation
        parseptr = strchr(buffer, ',')+1;
        Ground_Course = parsenumber(parseptr,2);      // Ground course in degrees * 100
        parseptr = strchr(parseptr, ',')+1;
        parseptr = strchr(parseptr, ',')+1;
        parseptr = strchr(parseptr, ',')+1;
        parseptr = strchr(parseptr, ',')+1;
        parseptr = strchr(parseptr, ',')+1;
        parseptr = strchr(parseptr, ',')+1;
        Ground_Speed = parsenumber(parseptr,2)*10/36; // Convert Km/h to m/s (*100)
        //GPS_line = true;
        }
	  else
	    {
	      SerialUSB.println("GPSERR: Checksum error!!");
	    }
    }
  }
  else
    {
	bufferidx = 0;
	  SerialUSB.println("GPSERR: Bad sentence!!");
    }
}

byte parseHex(char c) {
    if (c < '0')
      return (0);
    if (c <= '9')
      return (c - '0');
    if (c < 'A')
       return (0);
    if (c <= 'F')
       return ((c - 'A')+10);
}

// Decimal number parser
long parsedecimal(char *str,byte num_car) {
  long d = 0;
  byte i;
  
  i = num_car;
  while ((str[0] != 0)&&(i>0)) {
     if ((str[0] > '9') || (str[0] < '0'))
       return d;
     d *= 10;
     d += str[0] - '0';
     str++;
     i--;
     }
  return d;
}

// Function to parse fixed point numbers (numdec=number of decimals)
long parsenumber(char *str,byte numdec) {
  long d = 0;
  byte ndec = 0;
  
  while (str[0] != 0) {
     if (str[0] == '.'){
       ndec = 1;
     }
     else {
       if ((str[0] > '9') || (str[0] < '0'))
         return d;
       d *= 10;
       d += str[0] - '0';
       if (ndec > 0)
         ndec++;
       if (ndec > numdec)   // we reach the number of decimals...
         return d;
       }
     str++;
     }
  return d;
}

