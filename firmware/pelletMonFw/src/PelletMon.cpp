#include "PelletMon.h"
#include "config/PelletMonConfig.h"
#include "board.h"

#include <ksIotFrameworkLib.h>

#include "ArduinoOTA.h"

using namespace std::placeholders;

bool PelletMon::init()
{
	addComponent<ksf::ksWifiConnector>(PelletMonConfig::pelletMonDeviceName);
	addComponent<ksf::ksMqttDebugResponder>();
	mqtt = addComponent<ksf::ksMqttConnector>();
	statusLed = addComponent<ksf::ksLed>(STATUS_LED_PIN);

	//ArduinoOTA.begin();
	//ArduinoOTA.setPassword("ota_ksiotframework");

	if (!ksApplication::init())
		return false;

	mqtt->onMesssage.registerEvent(msgEventHandle, std::bind(&PelletMon::onMqttMessage, this, _1, _2));
	mqtt->onConnected.registerEvent(connEventHandle, std::bind(&PelletMon::onMqttConnected, this));
	mqtt->onDisconnected.registerEvent(disEventHandle, std::bind(&PelletMon::onMqttDisconnected, this));

	statusLed->setBlinking(500);

	return true;
}

void PelletMon::onMqttDisconnected()
{
	statusLed->setBlinking(500);
}

void PelletMon::onMqttConnected()
{
	statusLed->setBlinking(0);
}

void PelletMon::onMqttMessage(const String& topic, const String& payload)
{
	
}

bool PelletMon::loop()
{
	//ArduinoOTA.handle();
	return ksApplication::loop();
}