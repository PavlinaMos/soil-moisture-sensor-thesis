//    FILE: AD9833_minimal.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo AD9833 wave form generator
//URL: https://github.com/RobTillaart/AD9833


#include "AD9833.h"
#include <Adafruit_ADS1X15.h>

const int controlPin = 13;
int16_t adc0, adc2;
float Vmag1, Vphase1, Vmag2, Vphase2;
float Z, r, du;
float du_rad;
float Rp, Cp;
float Cp_nF;

// VSPI uses default   SCLK=18, MISO=19, MOSI=23, SELECT= 4 & 5
SPIClass * myspi = new SPIClass(VSPI);
AD9833 AD(5, myspi);
AD9833 AD2(4,myspi);


Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */



void setup() {
  Serial.begin(115200);

  myspi->begin();

  AD.begin();
  AD2.begin();

  AD.reset();
  AD2.reset();

  AD.setFrequency(1000000, 0);   //  1000 Hz.
  AD2.setFrequency(1000000,0);

  AD.setPhase(0);
  AD2.setPhase(90);

  AD.setWave(AD9833_SINE);
  AD2.setWave(AD9833_SINE);

  AD.writeControlRegister(0x2000);   // B28=1, RESET=0
  AD2.writeControlRegister(0x2000);

  Serial.println(AD.getWave());

  //delay(9000); switching off the sine wave
  //AD.setWave(AD9833_OFF);
  //AD2.setWave(AD9833_OFF);
  //delay(9000);
  //AD.setWave(AD9833_SINE);
  //AD2.setWave(AD9833_SINE);





  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }



pinMode(controlPin, OUTPUT);



}

void loop() {
  

  digitalWrite(controlPin, HIGH);
  delay(2500);

  adc0 = ads.readADC_SingleEnded(0);
  adc2 = ads.readADC_SingleEnded(2);

  Vmag1 = ads.computeVolts(adc2);
  Vphase1 = ads.computeVolts(adc0);


  digitalWrite(controlPin, LOW);
  delay(2500);

  adc0 = ads.readADC_SingleEnded(0);
  adc2 = ads.readADC_SingleEnded(2);

  Vmag2 = ads.computeVolts(adc2);
  Vphase2 = ads.computeVolts(adc0);

  r= 47 * pow(10, ( Vmag1 - Vmag2 ) / 0.6);
  du = (Vphase2 - Vphase1) / 0.01;

  du_rad= ( du *  PI ) / 180.0 ;
  Rp = r / cos(du_rad);
  Cp= sin(du_rad) / (2 * PI * 1000000  * r );
  Cp_nF= Cp * pow(10,9);


  Serial.println("-----------------------------------------------------------");
  Serial.print("Vmag1: "); Serial.print(adc0); Serial.print("  "); Serial.print(Vmag1); Serial.println("V");
  Serial.print("Vphase1: "); Serial.print(adc2); Serial.print("  "); Serial.print(Vphase1); Serial.println("V");
  Serial.print("Vmag2: "); Serial.print(adc0); Serial.print("  "); Serial.print(Vmag2); Serial.println("V");
  Serial.print("Vphase2: "); Serial.print(adc2); Serial.print("  "); Serial.print(Vphase2); Serial.println("V");
  
  Serial.println("-  -  -  -");
  Serial.print("Rp: ");    Serial.print(Rp);     Serial.println("Ohms");
  Serial.print("Cp: ");  Serial.print(Cp_nF);   Serial.println("nF");
  delay(4000); 


}