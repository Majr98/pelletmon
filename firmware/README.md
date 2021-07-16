# Firmware for PelletMon device

This directory contains code for PelletMon device. Written in C++ for ESP32, based on ksIotFrameworkLib and Arduino environment.
Use Visual Studio with Visual Micro plugin to open project properly. ESP32 environment must be installed within Arduino board manager. 
Requires ArduinoCAN + ksIotFrameworkLib dependencies.

* Temperature readings based on reverse engineered CAN protocol.
* Vide-Net support based on [awesome work done by Kem Klattrup @kematic](https://github.com/kematic).
* No support for Pellet (fuel) level sensor that was planned at beggining (no time for development, Vide-Net protocol might handle that).

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

## References and special thanks
* [Installation manual for Estyma based kettle (PL)](https://www.estyma.pl/wp-content/uploads/2018/11/Przewodnik-instalatora.pdf)
* [Vide-Net documentation (by Kem Klattrup)](https://github.com/kematic/pelle/wiki/Vide.Net-Bia24)
* [CAN-Network documentation (by Kem Klattrup)](https://github.com/kematic/pelle/wiki/CAN-network)
* [Curve Expert creators for awesome tool](https://www.curveexpert.net/)
* [CAN Monitor contributors for awesome tool](https://github.com/tixiv/CAN-Monitor-qt)
* Robert £abuz for his awesome support.
* Damian Nowakowski for being my rubber duck.
* Patryk Pietruszka for dissucssing a lot of problems.