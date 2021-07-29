#include "PelletMon.h"
#include "../../board.h"
#include "../config/PelletMonConfig.h"
#include "components/CanService.h"
#include "components/EstymaClient.h"
#include "ArduinoOTA.h"

using namespace std::placeholders;

bool PelletMon::init()
{
	/* Create required components (Wifi and Mqtt debug). */
	addComponent<ksf::ksWifiConnector>(PelletMonConfig::pelletMonDeviceName);
	addComponent<ksf::ksMqttDebugResponder>();

	/* Create mqttConnector and statusLed components. */
	addComponent<ksf::ksMqttConnector>();
	auto statusLed_wp = addComponent<ksf::ksLed>(STATUS_LED_PIN);

	/* Add CAN handler. */
	canService_wp = addComponent<comps::CanService>();
	addComponent<comps::EstymaClient>();

	/* Try to initialize superclass. It will initialize our components and tcpip (due to WiFi component). */
	if (!ksApplication::init())
		return false;

	/* ArduinoOTA is allowed to start AFTER app initialization, because tcpip must be initialized. */
	ArduinoOTA.begin();
	ArduinoOTA.setHostname(PelletMonConfig::pelletMonDeviceName);
	ArduinoOTA.setPassword("ota_ksiotframework");

	/* We want to unbind CAN before flash start. */
	ArduinoOTA.onStart([=]() {
		if (auto canService_sp = canService_wp.lock())
			canService_sp->stopService();
	});

	/* Start blinking status led. It will be disabled when Mqtt connection is established (by onMqttConnected callback). */
	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(500);

	/* Application finished initialization, return true as it succedeed. */
	return true;
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