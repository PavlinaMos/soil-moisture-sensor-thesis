#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>

#include "AD9833.h"
#include <Adafruit_ADS1X15.h>
#include "thingProperties.h"

const int controlPin = 13;
int16_t adc0, adc2;
float Vmag1, Vphase1, Vmag2, Vphase2;
float Z, r, du;
float du_rad;
float Rp, Cp;
float Cp_nF;

// VSPI uses default SCLK=18, MISO=19, MOSI=23, SELECT=4 & 5.
SPIClass *myspi = new SPIClass(VSPI);
AD9833 AD(5, myspi);
AD9833 AD2(4, myspi);
Adafruit_ADS1115 ads;

bool connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  const unsigned long deadline = millis() + 20000;
  while (WiFi.status() != WL_CONNECTED && millis() < deadline) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wi-Fi connected: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("Wi-Fi connection timed out.");
  return false;
}

void cloudDelay(unsigned long durationMs) {
  const unsigned long start = millis();
  while (millis() - start < durationMs) {
    ArduinoCloud.update();
    delay(50);
  }
}

void setup() {
  Serial.begin(115200);
  connectToWifi();

  initProperties();
  setDebugMessageLevel(4);
  ArduinoCloud.begin(ArduinoIoTPreferredConnection, true, "mqtts-up.iot.arduino.cc", 8884);
  ArduinoCloud.printDebugInfo();

  myspi->begin();
  AD.begin();
  AD2.begin();
  AD.reset();
  AD2.reset();

  AD.setFrequency(1000000, 0);
  AD2.setFrequency(1000000, 0);
  AD.setPhase(0);
  AD2.setPhase(90);
  AD.setWave(AD9833_SINE);
  AD2.setWave(AD9833_SINE);
  AD.writeControlRegister(0x2000);
  AD2.writeControlRegister(0x2000);

  Serial.println(AD.getWave());

  ads.setGain(GAIN_TWO);
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1) {
    }
  }

  pinMode(controlPin, OUTPUT);
}

void loop() {
  ArduinoCloud.update();

  digitalWrite(controlPin, HIGH);
  cloudDelay(2500);

  adc0 = ads.readADC_SingleEnded(0);
  adc2 = ads.readADC_SingleEnded(2);
  Vmag1 = ads.computeVolts(adc2);
  Vphase1 = ads.computeVolts(adc0);

  digitalWrite(controlPin, LOW);
  cloudDelay(2500);

  adc0 = ads.readADC_SingleEnded(0);
  adc2 = ads.readADC_SingleEnded(2);
  Vmag2 = ads.computeVolts(adc2);
  Vphase2 = ads.computeVolts(adc0);

  r = 47 * pow(10, (Vmag1 - Vmag2) / 0.6);
  du = (Vphase2 - Vphase1) / 0.01;
  du_rad = (du * PI) / 180.0;
  Rp = r / cos(du_rad);
  Cp = sin(du_rad) / (2 * PI * 1000000 * r);
  Cp_nF = Cp * pow(10, 9);

  rp = Rp;
  vmag1 = Cp_nF;
  ArduinoCloud.update();

  Serial.println("-----------------------------------------------------------");
  Serial.print("Vmag1: "); Serial.print(adc0); Serial.print("  "); Serial.print(Vmag1); Serial.println("V");
  Serial.print("Vphase1: "); Serial.print(adc2); Serial.print("  "); Serial.print(Vphase1); Serial.println("V");
  Serial.print("Vmag2: "); Serial.print(adc0); Serial.print("  "); Serial.print(Vmag2); Serial.println("V");
  Serial.print("Vphase2: "); Serial.print(adc2); Serial.print("  "); Serial.print(Vphase2); Serial.println("V");
  Serial.println("-  -  -  -");
  Serial.print("Rp: "); Serial.print(Rp); Serial.println("Ohms");
  Serial.print("Cp: "); Serial.print(Cp_nF); Serial.println("nF");
  cloudDelay(4000);
}
