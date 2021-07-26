#include "EstymaClient.h"
#include "CanService.h"
#include "../misc/SensorUtils.h"
#include "../videnet/VideNet.h"

using namespace misc;
using namespace videnet;
using namespace std::placeholders;

/* CAN packet ids */
#define ESTYMA_CAN_ROTATIONS 0x2D7
//#define ESTYMA_CAN_TEMEPRATURES 0x2D6
//#define ESTYMA_CAN_CWUTEMPS 0x3D6
#define ESTYMA_CAN_EXHAUST_TEMPS 0x4D6

/* CAN speed */
#define ESTYMA_CAN_SPEED 125E3

namespace comps
{
	bool EstymaClient::init(ksf::ksComposable* owner)
	{
		/* Grab weak ptr for Can Service. */
		canService_wp = owner->findComponent<CanService>();

		/* Grab weak pointer for MQTT Connector. */
		mqttConn_wp = owner->findComponent<ksf::ksMqttConnector>();

		/* Grab weak pointer for LED. */
		statusLed_wp = owner->findComponent<ksf::ksLed>();

		/* Bind to MQTT events. */
		if (auto mqtt_sp = mqttConn_wp.lock())
		{
			mqtt_sp->onConnected->registerEvent(connEventHandle_sp, std::bind(&EstymaClient::onMqttConnected, this));
			mqtt_sp->onMesssage->registerEvent(msgEventHandle_sp, std::bind(&EstymaClient::onMqttMessage, this, _1, _2));
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
				{ESTYMA_CAN_EXHAUST_TEMPS,	10000},
				{VIDE_NET_RESPONSE,				0}
			};

			/* Start CAN service. */
			canService->startService(ESTYMA_CAN_SPEED, subMsgs, sizeof(subMsgs));
		}

		/* Stop blinking status led on MQTT connect. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(0);

		/* Subscribe to 'set' messages. */
		if (auto mqtt_sp = mqttConn_wp.lock())
			mqtt_sp->subscribe("set/#");
	}

	void EstymaClient::onMqttDisconnected()
	{
		/* Clear pending requests. */
		cleanupVideNetRequests(true);

		/* Start blinking status led on MQTT disconnect. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(500);

		/* Stop CAN service. */
		if (auto canService_sp = canService_wp.lock())
			canService_sp->stopService();
	}

	void EstymaClient::onMqttMessage(const String& topic, const String& message)
	{
		if (topic.equals("set/controller"))
		{
			sendVideNetRequest<VideNetSetController>(message.toInt() == 1);
		}
		else if (topic.equals("set/heatmode"))
		{
			uint8_t heatMode = message.toInt() - 1;
			sendVideNetRequest<VideNetSetHeatMode>(heatMode < 4 ? heatMode : HeatMode::Off);
		}
		else if (topic.equals("set/hotwatermode"))
		{
			uint8_t hotWaterMode = message.toInt() - 1;
			sendVideNetRequest<VideNetSetHotWaterMode>(hotWaterMode < 4 ? hotWaterMode : HeatMode::Off);
		}
		else return;

		/* Blink LED - handled command. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(100, 4);

		sendVideNetRequest<VideNetSaveSettings>();
	}

	void EstymaClient::queueVideNetRequest(std::shared_ptr<VideNetRequest> request_sp)
	{
		if (auto canService_sp = canService_wp.lock())
			if (canService_sp->sendMessage(request_sp->prepareMessage()) && request_sp->needWaitForReply())
				videNetRequests.push_back(request_sp);
	}

	void EstymaClient::handleVideNetResponse(const CanMessage& incommingMessage)
	{
		for (auto it = videNetRequests.begin(); it != videNetRequests.end();)
			if (it->get()->onResponse(incommingMessage))
				it = videNetRequests.erase(it);
			else
				++it;
	}

	void EstymaClient::cleanupVideNetRequests(bool forceClearAll)
	{
		for (auto it = videNetRequests.begin(); it != videNetRequests.end();)
			if (forceClearAll || (millis() - it->get()->getSendingTime() > 15000))
				it = videNetRequests.erase(it);
			else
				++it;
	}

	void EstymaClient::handleIncommingQueue()
	{
		if (auto canService_sp = canService_wp.lock())
		{
			CanMessage incommingMessage;
			
			if (canService_sp->receiveMessage(incommingMessage))
			{
				if (incommingMessage.frameId == VIDE_NET_RESPONSE)
				{
					handleVideNetResponse(incommingMessage);
				}
				else if (auto mqttConnection = mqttConn_wp.lock())
				{
					switch (incommingMessage.frameId)
					{
						/* Handle Vide Net response. */
						case VIDE_NET_RESPONSE:
							handleVideNetResponse(incommingMessage);
						break;

						/* Handle rotations packet. */
						case ESTYMA_CAN_ROTATIONS:
							mqttConnection->publish("rotations", String(incommingMessage.u16[0]));
						break;

						/* Handle exhaust packet. */
						case ESTYMA_CAN_EXHAUST_TEMPS:
							mqttConnection->publish("exhaust_temp", String(SensorUtils::convertTemp(incommingMessage.u16[3], EstymaTempSensorType::CT3A), 0));
						break;
					}

					/* Blink LED on each recevied packet. */
					if (auto statusLed_sp = statusLed_wp.lock())
						statusLed_sp->setBlinking(50, 3);
				}
			}
		}
	}

	void EstymaClient::tickVideNet()
	{
		if (auto canService_sp = canService_wp.lock())
		{
			if (millis() - lastVideNetPing > 15000)
			{
				canService_sp->sendMessage({ VIDE_NET_PING, 1, 0 });

				if (auto mqttConnection = mqttConn_wp.lock())
				{
					sendVideNetRequest<VideNetGetKettleTemp>([=](uint16_t kettleTemp) {
						mqttConnection->publish("boiler_temp", String(kettleTemp / 10.0f, 1));
					});

					sendVideNetRequest<VideNetGetHotWaterTemp>([=](uint16_t hotWaterTemp) {
						mqttConnection->publish("cwu_temp", String(hotWaterTemp / 10.0f, 1));
					});

					sendVideNetRequest<VideNetGetHeatMode>([=](uint8_t heatMode) {
						mqttConnection->publish("heatmode_current", String(heatMode + 1));
					});

					sendVideNetRequest<VideNetGetHotWaterMode>([=](uint8_t heatMode) {
						mqttConnection->publish("hotwatermode_current", String(heatMode + 1));
					});
				}

				/* Blink LED on each recevied packet. */
				if (auto statusLed_sp = statusLed_wp.lock())
					statusLed_sp->setBlinking(75, 2);

				lastVideNetPing = millis();
			}
		}

		cleanupVideNetRequests();
	}

	bool EstymaClient::loop()
	{
		tickVideNet();
		return true;
	}
}