#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>

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
      targetSlot = i;
    }
  }
  if (targetSlot == MAX_KNOWN_NETWORKS) {
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
