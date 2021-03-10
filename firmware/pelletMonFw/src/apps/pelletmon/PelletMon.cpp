#include "PelletMon.h"
#include "../../board.h"
#include "../config/PelletMonConfig.h"
#include "components/EstymaCANClient.h"
#include "components/BoilerStatusUpdater.h"
#include "ArduinoOTA.h"

using namespace std::placeholders;

bool PelletMon::init()
{
	/* Create required components (Wifi and Mqtt debug). */
	addComponent<ksf::ksWifiConnector>(PelletMonConfig::pelletMonDeviceName);
	addComponent<ksf::ksMqttDebugResponder>();

	/* Cache weak pointers to mqttConnector and statusLed components. */
	mqtt_wp = addComponent<ksf::ksMqttConnector>();
	statusLed_wp = addComponent<ksf::ksLed>(STATUS_LED_PIN);

	/* Add CAN handler. */
	addComponent<comps::BoilerStatusUpdater>();
	canclient_wp = addComponent<comps::EstymaCANClient>();

	/* Try to initialize superclass. It will initialize our components and tcpip (due to WiFi component). */
	if (!ksApplication::init())
		return false;

	/* ArduinoOTA is allowed to start AFTER app initialization, because tcpip must be initialized. */
	ArduinoOTA.begin();
	ArduinoOTA.setPassword("ota_ksiotframework");

	/* We want to unbind CAN before flash start. */
	ArduinoOTA.onStart([=]() {
		if (auto canclient_sp = canclient_wp.lock())
			canclient_sp->unbindCAN();
	});

	/* Bind to MQTT events. */
	if (auto mqtt_sp = mqtt_wp.lock())
	{
		mqtt_sp->onConnected->registerEvent(connEventHandle_sp, std::bind(&PelletMon::onMqttConnected, this));
		mqtt_sp->onDisconnected->registerEvent(disEventHandle_sp, std::bind(&PelletMon::onMqttDisconnected, this));
	}

	/* Start blinking status led. It will be disabled when Mqtt connection is established (by onMqttConnected callback). */
	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(500);

	/* Application finished initialization, return true as it succedeed. */
	return true;
}

void PelletMon::onMqttDisconnected()
{
	/* Start blinking status led on MQTT disconnect. */
	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(500);

	/* Unbind CAN on disconnect. */
	if (auto canclient_sp = canclient_wp.lock())
		canclient_sp->unbindCAN();
}

void PelletMon::onMqttConnected()
{
	/* Stop blinking status led on MQTT connect. */
	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(0);

	/* Bind CAN on connected. */
	if (auto canclient_sp = canclient_wp.lock())
		canclient_sp->bindCAN();
}

bool PelletMon::loop()
{
	/* Handle OTA stuff. */
	ArduinoOTA.handle();

	/*	
		Return to superclass application loop. 
		It handles all our components and whole app logic. 
	*/
	return ksApplication::loop();
}