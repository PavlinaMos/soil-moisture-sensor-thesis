# ESP32 Measurement Monitor

This project runs on an ESP32 NodeMCU-32S. The sensor currently calculates the inserted values of R and C and calculates the impedance. The user can connect an R, C, or a parallel RC element and the sensor measures and publishes resistance (`RP`) and capacitance (`CP`) to Arduino Cloud. 
It also provides Wi-Fi setup using the WiFi Manager library, which allows the user to connect to the WiFi without hard coding the credentials. stores the credentials of previously connected networks. On startup, the device automatically searches for a known Wi-Fi network and connects to it. If no saved network is found, it switches to Access Point (AP) mode and launches a captive portal, allowing the user to enter the credentials of a new Wi-Fi network, which are then saved for future use.

## Analog Parts Used

1. LTC6268
2. AD8130
3. AD8066
4. ADG733

## Modules Used

1. AD9833 (Waveform Generator)
2. ADS1115 (ADC)
3. AD8302 (Gain-Phase Detector)




## Development environment

 Install PlatformIO IDE extension in VScode 

## Project layout

```text
platformio.ini              PlatformIO board, framework, and library settings
src/main.cpp                Application code
include/thingProperties.h   Arduino Cloud device and variable definitions
include/arduino_secrets.h   Local Wi-Fi and device secrets; never committed

```


## Configure private secrets

Create or edit `include/arduino_secrets.h` locally. It must define the Arduino
Cloud device key. Never commit this file or share its contents.

```cpp
#pragma once

#define SECRET_SSID ""
#define SECRET_PASS ""
#define SECRET_DEVICE_KEY "paste-your-Arduino-Cloud-device-key-here"
```

The Wi-Fi Manager branches do not use `SECRET_SSID` or `SECRET_PASS` for normal
operation; they remain for compatibility. The device key is required for
Arduino Cloud authentication.

## Build the firmware

In VS Code, use the PlatformIO buttons in the bottom status bar:

- **Check mark**: Build the project.
- **Right arrow**: Upload the built firmware to the ESP32.
- **Plug icon**: Open the serial monitor.

The first build downloads and compiles dependencies, so it takes longer. A
successful build ends with `SUCCESS`.

You can also build from a terminal opened in this folder:

```powershell
pio run
```

## Upload to the NodeMCU-32S

1. Connect the ESP32 using a USB data cable.
2. Confirm that Windows assigns a COM port.
4. Click the PlatformIO **Upload** arrow.



## Serial monitor

Open the PlatformIO Serial Monitor after uploading. This project uses
**115200 baud**.  Voltage, phase, resistance, capacitance, Wi-Fi,
and Arduino Cloud connection messages are shown.


## Arduino Cloud

The Arduino Cloud Thing displays the values on dashboards. After the ESP32 joins Wi-Fi, it connects to Arduino Cloud and publishes the measurements periodically. A dashboard's **Live** chart starts at the current viewing session. It also displays these values in a time chart keeping history of up to 15 days.


## Adding a new known network

When no saved network is visible or can connect, the setup portal starts
automatically. Use it to add a new network. The current implementation keeps
up to five profiles. When the list is full, the first stored profile is
replaced.

For testing with a mobile hotspot, enable a 2.4 GHz hotspot. If a known home
network is still available, the ESP32 will correctly reconnect to it instead of
opening the portal. Make the known network unavailable to test portal fallback.
