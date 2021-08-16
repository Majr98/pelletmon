# Firmware for PelletMon device

This directory contains code for PelletMon device. Written in C++ for ESP32, based on ksIotFrameworkLib and Arduino environment.
Use Visual Studio with Visual Micro plugin to open project properly. ESP32 environment must be installed within Arduino board manager. 
Requires ESP32_CAN library with CAN Common + ksIotFrameworkLib dependencies.

* Temperature readings based on reverse engineered CAN protocol.
* Vide-Net support based on [awesome work done by Kem Klattrup @kematic](https://github.com/kematic).
* No support for Pellet (fuel) level sensor that was planned at beggining (no time for development, Vide-Net protocol might handle that).

## Features
* Based on ksIotFrameworkLib, simple, clear and responsive non-blocking code.
* WiFi settings support (via cap portal with WiFiManager library).
* Reading kettle state (kettle temperature, hot water temperature, power etc.)
* Remote kettle control (heating modes, disable/enable controller etc.).
* Synchronization with MQTT broker.

## Components

PelletMon app is divided into components, that handle specific tasks.
* EstymaClient - handles Estyma CAN protocol (reads message from queue in loop but is also able to send messages), handles VideNet communication also as well.

## Other classes
* VideNet - contains message classes and general protocol implementation for VideNet. Used by EstymaClient to handle VideNet specific messages.
* Board - some low-level code handling CAN bus is placed inside board.cpp.

## References and special thanks
* [Installation manual for Estyma based kettle (PL)](https://www.estyma.pl/wp-content/uploads/2018/11/Przewodnik-instalatora.pdf)
* [Vide-Net documentation (by Kem Klattrup)](https://github.com/kematic/pelle/wiki/Vide.Net-Bia24)
* [CAN-Network documentation (by Kem Klattrup)](https://github.com/kematic/pelle/wiki/CAN-network)
* [Curve Expert creators for awesome tool](https://www.curveexpert.net/)
* [CAN Monitor contributors for awesome tool](https://github.com/tixiv/CAN-Monitor-qt)
* Robert Labuz for his awesome support.
* Damian Nowakowski for being my rubber duck.
* Patryk Pietruszka for dissucssing a lot of problems.
