#include "PelletMon.h"
#include "../../board.h"
#include "../config/PelletMonConfig.h"
#include "components/EstymaCANClient.h"
#include "ArduinoOTA.h"

using namespace std::placeholders;

bool PelletMon::init()
{
	addComponent<ksf::ksWifiConnector>(PelletMonConfig::pelletMonDeviceName);
	addComponent<ksf::ksMqttDebugResponder>();
	mqtt_wp = addComponent<ksf::ksMqttConnector>();
	statusLed_wp = addComponent<ksf::ksLed>(STATUS_LED_PIN);

	addComponent<comps::EstymaCANCLient>();

	//ArduinoOTA.begin();
	//ArduinoOTA.setPassword("ota_ksiotframework");

	if (!ksApplication::init())
		return false;

	if (auto mqtt_sp = mqtt_wp.lock())
	{
		mqtt_sp->onConnected->registerEvent(connEventHandle_sp, std::bind(&PelletMon::onMqttConnected, this));
		mqtt_sp->onDisconnected->registerEvent(disEventHandle_sp, std::bind(&PelletMon::onMqttDisconnected, this));
	}

	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(500);

	return true;
}

void PelletMon::onMqttDisconnected()
{
	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(500);
}

void PelletMon::onMqttConnected()
{
	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(0);
}

bool PelletMon::loop()
{
	//ArduinoOTA.handle();
	return ksApplication::loop();
}