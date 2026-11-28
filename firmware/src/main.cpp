#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>

#include "AD9833.h"
#include <Adafruit_ADS1X15.h>
#include "thingProperties.h"
#include <cmath>

const int controlPin = 13;
int16_t adc0, adc2;
float Vmag1, Vphase1, Vmag2, Vphase2;
float Z, r, du;
float du_rad;
float Rp, Cp;
float Cp_nF;
float frequency=1000000;
const float pi = 3.14159f;

const float kg = 0.19f;
const float c_parasitic = 57e-12;
const float c_parasitic_coax = 16e-12;
float e_prime;
float e_double_prime;
float eapp;

const float a = -3.7e-9; //first equation (theta)
const float b = 2.7e-6;
const float c = 7.98e-5;
const float d = 0.08359; //first equation (theta)

const float a_wout = -3.71e-9;          //first equation after removing 57pF (theta_wout_parasitics)
const float b_wout = 2.53e-6;
const float c_wout = 15.633e-5;
const float d_wout = 0.0887286;  
float theta_wout_parasitics;     //first equation after removing 57pF  (theta_wout_parasitics)

const float a2 = -1.75e-10; // second equation (theta2)
const float b2 = -6.29e-7;
const float c2 = 9.33e-4;
const float d2 = 0.026416;
float theta2;              // second equation (theta2)

const float a2_wout = -5.75e-10;     // second equation after removing 57pF (theta2_wout_parasitics)
const float b2_wout = -2.23e-7;
const float c2_wout = 79.1183e-5;
const float d2_wout = 0.0514256;
float theta2_wout_parasitics;   // second equation after removing 57pF (theta2_wout_parasitics)


const float a_l = -5.2e-10;     // last equation written on thesis
const float b_l = 8.56e-8;
const float c_l = 6.02e-4;
const float d_l = 0.061913;
float theta_last;


// VSPI uses default SCLK=18, MISO=19, MOSI=23, SELECT=4 & 5.
SPIClass *myspi = new SPIClass(VSPI);
AD9833 AD(5, myspi);
AD9833 AD2(4, myspi);
Adafruit_ADS1115 ads;
WiFiManager wifiManager;
WiFiConnectionHandler *cloudConnection = nullptr;
Preferences knownNetworks;

constexpr char NETWORKS_NAMESPACE[] = "known-wifi";
constexpr uint8_t MAX_KNOWN_NETWORKS = 5;

String ssidKey(uint8_t index) { return "ssid" + String(index); }
String passKey(uint8_t index) { return "pass" + String(index); }

bool isVisibleNetwork(const String &ssid, int networkCount) {
  for (int i = 0; i < networkCount; ++i) {
    if (WiFi.SSID(i) == ssid) {
      return true;
    }
  }
  return false;
}

bool connectToKnownNetwork() {
  const int networkCount = WiFi.scanNetworks();
  if (networkCount <= 0) {
    Serial.println("No known Wi-Fi networks are visible.");
    return false;
  }

  knownNetworks.begin(NETWORKS_NAMESPACE, true);
  for (uint8_t i = 0; i < MAX_KNOWN_NETWORKS; ++i) {
    String ssid = knownNetworks.getString(ssidKey(i).c_str(), "");
    String pass = knownNetworks.getString(passKey(i).c_str(), "");
    if (ssid.isEmpty() || !isVisibleNetwork(ssid, networkCount)) {
      continue;
    }

    Serial.print("Trying known Wi-Fi: ");
    Serial.println(ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());
    const unsigned long deadline = millis() + 15000;
    while (WiFi.status() != WL_CONNECTED && millis() < deadline) {
      delay(250);
    }
    if (WiFi.status() == WL_CONNECTED) {
      knownNetworks.end();
      return true;
    }
    WiFi.disconnect();
  }
  knownNetworks.end();
  return false;
}

void rememberNetwork(const String &ssid, const String &pass) {
  knownNetworks.begin(NETWORKS_NAMESPACE, false);
  uint8_t targetSlot = MAX_KNOWN_NETWORKS;
  for (uint8_t i = 0; i < MAX_KNOWN_NETWORKS; ++i) {
    String storedSsid = knownNetworks.getString(ssidKey(i).c_str(), "");
    if (storedSsid == ssid) {
      targetSlot = i;
      break;
    }
    if (storedSsid.isEmpty() && targetSlot == MAX_KNOWN_NETWORKS) {
      targetSlot = i;                                          //  slot is empty AND havent found slot yet
    }
  }
  if (targetSlot == MAX_KNOWN_NETWORKS) {                      //  no known Wifi AND no empty slot
    targetSlot = 0;
  }
  knownNetworks.putString(ssidKey(targetSlot).c_str(), ssid);
  knownNetworks.putString(passKey(targetSlot).c_str(), pass);
  knownNetworks.end();
}

bool connectToWifi() {
  WiFi.mode(WIFI_STA);
  // Store our own list of profiles, rather than the single ESP Wi-Fi profile.
  wifiManager.resetSettings();
  wifiManager.setHostname("ESP32-Meter");
  wifiManager.setConnectTimeout(20);
  wifiManager.setConfigPortalTimeout(300);

  if (!connectToKnownNetwork()) {
    Serial.println("Opening Wi-Fi setup portal.");
    if (!wifiManager.startConfigPortal("ESP32-Meter-Setup", "configure-me")) {
      Serial.println("Wi-Fi setup timed out; restarting.");
      ESP.restart();
      return false;
    }

    String ssid = wifiManager.getWiFiSSID(false);
    String pass = wifiManager.getWiFiPass(false);
    rememberNetwork(ssid, pass);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi setup timed out; restarting.");
    ESP.restart();
    return false;
  }

  String ssid = WiFi.SSID();
  knownNetworks.begin(NETWORKS_NAMESPACE, true);
  String pass;
  for (uint8_t i = 0; i < MAX_KNOWN_NETWORKS; ++i) {
    if (knownNetworks.getString(ssidKey(i).c_str(), "") == ssid) {
      pass = knownNetworks.getString(passKey(i).c_str(), "");
      break;
    }
  }
  knownNetworks.end();
  cloudConnection = new WiFiConnectionHandler(ssid.c_str(), pass.c_str());

  Serial.print("Wi-Fi connected: ");
  Serial.println(WiFi.localIP());
  return true;
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
  ArduinoCloud.begin(*cloudConnection, true, "mqtts-up.iot.arduino.cc", 8884);
  ArduinoCloud.printDebugInfo();

  myspi->begin();
  AD.begin();
  AD2.begin();
  AD.reset();
  AD2.reset();

  AD.setFrequency(frequency, 0);
  AD2.setFrequency(frequency, 0);
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
 // ArduinoCloud.update();

  digitalWrite(controlPin, HIGH);
 // cloudDelay(2500);
delay(3000);

  adc0 = ads.readADC_SingleEnded(0);
  adc2 = ads.readADC_SingleEnded(2);
  Vmag1 = ads.computeVolts(adc2);
  Vphase1 = ads.computeVolts(adc0);

  digitalWrite(controlPin, LOW);
 // cloudDelay(2500);
 delay(4000);

  adc0 = ads.readADC_SingleEnded(0);
  adc2 = ads.readADC_SingleEnded(2);
  Vmag2 = ads.computeVolts(adc2);
  Vphase2 = ads.computeVolts(adc0);

  r = 47 * pow(10, (Vmag1 - Vmag2) / 0.6);
  du = (Vphase1 - Vphase2) / 0.01;
  du_rad = (du * PI) / 180.0;
  Rp = r / cos(du_rad);
  Cp = - sin(du_rad) / (2 * PI * 1000000 * r);
  Cp_nF = Cp * 1e9 - c_parasitic * 1e9  - c_parasitic_coax * 1e9;

  rp = Rp;
  cp = Cp_nF;
  //ArduinoCloud.update();

  e_prime = ( Cp_nF * 1e-9f ) / ( kg * (8.854 * 1e-12f ));
  e_double_prime = 1.0f / (rp * kg * 2.0f * pi * frequency * (8.854 * 1e-12f ) );
  eapp = (e_prime / 2.0f) *(1.0f + sqrt(1.0f + pow(e_double_prime / e_prime, 2.0f)));
  
  theta = a * pow(eapp, 3.0f) + b * pow(eapp, 2.0f) + c * eapp + d;
  theta2 = a2 * pow(eapp, 3.0f) + b2 * pow(eapp, 2.0f) + c2 * eapp + d2; //second equation


  theta_wout_parasitics  = a_wout * pow(eapp, 3.0f) + b_wout * pow(eapp, 2.0f) + c_wout * eapp + d_wout;
  theta2_wout_parasitics = a2_wout * pow(eapp, 3.0f) + b2_wout * pow(eapp, 2.0f) + c2_wout * eapp + d2_wout;
  
  theta_last = a_l * pow(eapp, 3.0f) + b_l * pow(eapp, 2.0f) + c_l * eapp + d_l;



  Serial.println("-----------------------------------------------------------");
  Serial.print("Vmag1: "); Serial.print(adc0); Serial.print("  "); Serial.print(Vmag1); Serial.println("V");
  Serial.print("Vphase1: "); Serial.print(adc2); Serial.print("  "); Serial.print(Vphase1); Serial.println("V");
  Serial.print("Vmag2: "); Serial.print(adc0); Serial.print("  "); Serial.print(Vmag2); Serial.println("V");
  Serial.print("Vphase2: "); Serial.print(adc2); Serial.print("  "); Serial.print(Vphase2); Serial.println("V");
  Serial.println("-  -  -  -");
  Serial.print("Rp: "); Serial.print(Rp); Serial.println("Ohms");
  Serial.print("Cp: "); Serial.print(Cp_nF,3); Serial.println("nF");

  Serial.print("theta: "); Serial.println(theta); 
  Serial.print("theta2: "); Serial.println(theta2); //second equation 
  
  Serial.print("theta_wout_cpar: "); Serial.println(theta_wout_parasitics); 
  Serial.print("theta2_wout_c_par: "); Serial.println(theta2_wout_parasitics); //second equation 
  Serial.print("theta_last: "); Serial.println(theta_last);
  Serial.print("---e_prime: "); Serial.print(e_prime); Serial.print("--- e_double_prime"); Serial.print(e_double_prime); Serial.print("--- e_apparent:"); Serial.print(eapp);
 
 
 
  // cloudDelay(4000);
  delay(5000);
}
