#include "EstymaClient.h"
#include "CanService.h"
#include "../videnet/VideNet.h"

using namespace videnet;
using namespace std::placeholders;

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
				{VIDE_NET_RESPONSE,	0}
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
		videNetRequests.clear();

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

	void EstymaClient::eraseVideNetRequestIf(std::function<bool(std::shared_ptr<videnet::VideNetRequest> req)> fn)
	{
		for (auto it = videNetRequests.begin(); it != videNetRequests.end();)
		{
			it = fn(*it) ? videNetRequests.erase(it) : it + 1;
		}
	}

	void EstymaClient::tryPublishToMqtt(const char* topic, const String& value) const
	{
		if (auto mqttConnection = mqttConn_wp.lock())
			mqttConnection->publish(topic, value);
	}

	void EstymaClient::handleMessageQueue()
	{
		if (auto canService_sp = canService_wp.lock())
		{
			CanMessage incommingMessage;
			if (canService_sp->receiveMessage(incommingMessage) && incommingMessage.frameId == VIDE_NET_RESPONSE)
			{
				/* Handle vide net packet. Remove request if response handled. */
				eraseVideNetRequestIf([&](std::shared_ptr<videnet::VideNetRequest> req)->bool {
					return req->onResponse(incommingMessage);
				});
			}
		}
	}

	void EstymaClient::handleVideNetPeriodicOps()
	{
		if (auto canService_sp = canService_wp.lock())
		{
			if (millis() - lastVideNetPing > VIDE_NET_PING_DELAY)
			{
				/* Send ping packet to kettle. It brings up our module in CAN network node list. */
				canService_sp->sendMessage({VIDE_NET_PING, 1, 0});

				/* Request current kettle temperature. */
				sendVideNetRequest<VideNetGetKettleTemp>([&](uint16_t kettleTemp) {
					tryPublishToMqtt("boiler_temp", String(kettleTemp * 0.1f, 1));
				});

				/* Request current hot water temperature. */
				sendVideNetRequest<VideNetGetHotWaterTemp>([&](uint16_t hotWaterTemp) {
					tryPublishToMqtt("cwu_temp", String(hotWaterTemp * 0.1f, 1));
				});

				/* Request current heat mode. */
				sendVideNetRequest<VideNetGetHeatMode>([&](uint8_t heatMode) {
					tryPublishToMqtt("heatmode_current", String(heatMode + 1));
				});

				/* Request current hot water mode. */
				sendVideNetRequest<VideNetGetHotWaterMode>([&](uint8_t heatMode) {
					tryPublishToMqtt("hotwatermode_current", String(heatMode + 1));
				});

				/* Request total burner usage (kg * 10). */
				sendVideNetRequest<VideNetGetBurnerUsageTotal>([&](uint32_t totalUsage) {
					tryPublishToMqtt("burnerusage_total", String(totalUsage * 0.1f, 1));
				});

				/* Request current burner power. */
				sendVideNetRequest<VideNetGetBurnerPower>([&](uint8_t powerPercentage) {
					tryPublishToMqtt("burnerpower_current", String(powerPercentage));
				});

				/* Blink LED on each recevied packet. */
				if (auto statusLed_sp = statusLed_wp.lock())
					statusLed_sp->setBlinking(75, 2);

				/* Set current time as last ping time. */
				lastVideNetPing = millis();
			}
		}

		/* Call cleanup method to kick out timed out requests. */
		eraseVideNetRequestIf([](std::shared_ptr<videnet::VideNetRequest> req)->bool {
			return millis() - req->getSendingTime() > KSF_ONE_SECOND_MS;
		});
	}

	bool EstymaClient::loop()
	{
		/* Handles pending messages in queue. */
		handleMessageQueue();

		/* Handles periodic VideNet operations (clean up requests, send info to MQTT etc...). */
		handleVideNetPeriodicOps();

		return true;
	}
}