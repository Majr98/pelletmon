#include "PelletMonConfig.h"
#include "../board.h"

const char PelletMonConfig::pelletMonDeviceName[] = "PelletMon";

bool PelletMonConfig::init()
{
	addComponent<ksf::ksLed>(STATUS_LED_PIN);

	addComponent<ksf::ksWiFiConfigurator>(pelletMonDeviceName);
	addComponent<ksf::ksMqttConfigProvider>();

	return ksApplication::init();
}

bool PelletMonConfig::loop()
{
	return ksApplication::loop();
}