# Firmware for PelletMon device

This directory contains code for PelletMon device. Written in C++ for ESP32, based on ksIotFrameworkLib and Arduino environment.
Use Visual Studio with Visual Micro plugin to open project properly. ESP32 environment must be installed within Arduino board manager. 
Requires ArduinoCAN + ksIotFrameworkLib dependencies.

* Temperature readings based on reverse engineered CAN protocol.
* Vide-Net support based on awesome work done by Kem Klattrup @kematic (https://github.com/kematic)

## Features
* CanBus communication support.
* Based on ksIotFrameworkLib, simple, clear and responsive non-blocking code.
* Settings support (via WiFi portal - WiFiManager library).
* Reading of temperatures (kettle, water, exhaust) + blower RPM.
* Remote kettle control (heating modes, disable/enable controller).
* Automatic publishing fresh readings to MQTT broker.
* Basic VideNet support, might be extended to full kettle remote control.

## Components

PelletMon app is divided into components, that handle specific tasks.

* CanService - handles low-level CAN communication by using Arduino-CAN library. Handles message-filtering, receives messages into a queue. 
* EstymaClient - handles Estyma CAN protocol (reads message from queue in loop but is also able to send messages), handles VideNet communication also as well.

## Other classes

* VideNet - contains message classes and general protocol implementation for VideNet. Used by EstymaClient to handle VideNet specific messages.
* SensorUtils - contains sensor specific calculation algorithms, to be able to convert raw ADC readings done by Estyma-driver into human-readable temperature values.
