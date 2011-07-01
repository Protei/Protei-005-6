unsigned int adc_tmp;
static const unsigned char adc_cmd[9]=  { 0x87, 0xC7, 0x97, 0xD7, 0xA7, 0xE7, 0xB7, 0xF7, 0x00 };
unsigned int adc_value[8];

HardwareSPI spi(2);

void setup() {
  spi.begin(SPI_2_25MHZ, MSBFIRST, 0);
  pinMode(31, OUTPUT);
  digitalWrite(31, HIGH);
}

void loop() {
  digitalWrite(31, LOW);
  
  spi.transfer(adc_cmd[0]);
  
  for(int ch=0; ch < 8; ch++) {
    adc_tmp = spi.transfer(0) << 8;
    adc_tmp |= spi.transfer(adc_cmd[ch+1]);
    adc_value[ch] = adc_tmp >> 3;
  }
  
  digitalWrite(31, HIGH);
  
  for(int ch=0; ch < 8; ch++) {
    SerialUSB.print(ch);
    SerialUSB.print(": ");
    SerialUSB.println(adc_value[ch]);
  }
}
