# ABOUT

This is firmware code for PelletMon device. Written in C++ with Arduino framework, using Visual Studio with Visual Micro plugin.
Uses ksIotFrameworkLib. Connects to MQTT broker and sends fresh boiler status data.

# COMPONENTS

App PelletMon is constructed from two components.

* BoilerStatusUpdater - caches and sends values periodically to MQTT broker.
* EstymaCANClient - handles Estyma CAN protocol (via interrupt) and updates values inside BoilerStatusUpdater.
