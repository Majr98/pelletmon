#include "EstymaClient.h"
#include "CanService.h"
#include "../misc/SensorUtils.h"

using namespace misc;
using namespace std::placeholders;

/* CAN packet ids */
#define ESTYMA_CAN_ROTATIONS 0x2D7
#define ESTYMA_CAN_TEMEPRATURES 0x2D6
#define ESTYMA_CAN_CWUTEMPS 0x3D6
#define ESTYMA_CAN_EXHAUST_TEMPS 0x4D6

/* CAN speed */
#define ESTYMA_CAN_SPEED 125E3

namespace comps
{
	bool EstymaClient::init(ksf::ksComposable* owner)
	{
		/* Grab weak ptr for Can Service*/
		canService_wp = owner->findComponent<CanService>();

		/* Grab weak pointer for MQTT Connector. */
		mqttConn_wp = owner->findComponent<ksf::ksMqttConnector>();

		/* Grab weak pointer for LED. */
		statusLed_wp = owner->findComponent<ksf::ksLed>();

		/* Bind to debug responder message. */
		if (auto mqttdc_sp = owner->findComponent<ksf::ksMqttDebugResponder>().lock())
			mqttdc_sp->customDebugHandler->registerEvent(debugMessageEventHandle_sp, std::bind(&EstymaClient::onDebugMessage, this, _1, _2, _3));

		/* Bind to MQTT events. */
		if (auto mqtt_sp = mqttConn_wp.lock())
		{
			mqtt_sp->onConnected->registerEvent(connEventHandle_sp, std::bind(&EstymaClient::onMqttConnected, this));
			mqtt_sp->onDisconnected->registerEvent(disEventHandle_sp, std::bind(&EstymaClient::onMqttDisconnected, this));
		}

		return true;
	}

	void EstymaClient::onMqttConnected()
	{
		/* Create structure to subscribe messages. */
		if (auto canService = canService_wp.lock())
		{
			CanServiceSubscribeInfo subMsgs[] =
			{
				{ESTYMA_CAN_ROTATIONS,		10000},
				{ESTYMA_CAN_TEMEPRATURES,	10000},
				{ESTYMA_CAN_CWUTEMPS,		10000},
				{ESTYMA_CAN_EXHAUST_TEMPS,	10000}
			};

			/* Start CAN service. */
			canService->startService(ESTYMA_CAN_SPEED, subMsgs, sizeof(subMsgs));
		}

		/* Stop blinking status led on MQTT connect. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(0);
	}

	void EstymaClient::onMqttDisconnected()
	{
		/* Start blinking status led on MQTT disconnect. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(500);

		/* Stop CAN service. */
		if (auto canService_sp = canService_wp.lock())
			canService_sp->stopService();
	}

	void EstymaClient::onDebugMessage(ksf::ksMqttDebugResponder* responder, const String& message, bool& consumed)
	{
		if (message.equals("test_pellet"))
		{
			responder->respond("test pellet ok!");
			consumed = true;
		}
		else if (message.equals("cdis"))
		{
			responder->respond("disable controller not implemented yet!");
			consumed = true;
		}
		else if (message.equals("cen"))
		{
			responder->respond("enable controller not implemented yet!");
			consumed = true;
		}
	}

	bool EstymaClient::loop()
	{
		if (auto canService_sp = canService_wp.lock())
		{
			CanMessage incommingMessage;
			if (canService_sp->receiveMessage(incommingMessage))
			{
				if (auto mqttConnection = mqttConn_wp.lock())
				{
					switch (incommingMessage.frameId)
					{
						/* Handle kettle temp packet. */
						case ESTYMA_CAN_TEMEPRATURES:
							mqttConnection->publish("boiler_temp", String(SensorUtils::convertTemp(incommingMessage.u16[0], EstymaTempSensorType::CT2A), 1));
						break;

						/* Handle rotations packet. */
						case ESTYMA_CAN_ROTATIONS:
							mqttConnection->publish("rotations", String(incommingMessage.u16[0]));
						break;

						/* Handle boiler packet. */
						case ESTYMA_CAN_CWUTEMPS:
							mqttConnection->publish("cwu_temp", String(SensorUtils::convertTemp(incommingMessage.u16[0], EstymaTempSensorType::CT2A), 1));
						break;

						/* Handle exhaust packet. */
						case ESTYMA_CAN_EXHAUST_TEMPS:
							mqttConnection->publish("exhaust_temp", String(SensorUtils::convertTemp(incommingMessage.u16[3], EstymaTempSensorType::CT3A), 0));
						break;
					}
				}

				/* Blink LED on each recevied packet. */
				if (auto statusLed_sp = statusLed_wp.lock())
					statusLed_sp->setBlinking(75, 2);
			}
		}

		return true;
	}
}