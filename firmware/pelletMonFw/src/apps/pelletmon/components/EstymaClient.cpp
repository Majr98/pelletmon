#include "../../../board.h"
#include "../videnet/VideNet.h"
#include "EstymaClient.h"

using namespace videnet;
using namespace std::placeholders;

namespace comps
{
	bool EstymaClient::init(ksf::ksComposable* owner)
	{
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

		/* Pre-initialize CAN bus. */
		init_can_config();

		/* Pre-initialize filter - accept only VIDE NET responses. */
		init_can_filter(VIDE_NET_RESPONSE);

		return true;
	}

	void EstymaClient::onMqttConnected()
	{
		/* Stop blinking status led on MQTT connect. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(0);

		/* Subscribe to 'set' messages. */
		if (auto mqtt_sp = mqttConn_wp.lock())
			mqtt_sp->subscribe("set/#");

		/* Run CAN service. */
		start_can_module();
	}
	
	void EstymaClient::onMqttDisconnected()
	{
		/* Clear pending requests. */
		videNetRequests.clear();

		/* Start blinking status led on MQTT disconnect. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(500);

		/* Stop CAN service. */
		stop_can_module();
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

	void EstymaClient::handleMessageQueue()
	{
		for (CAN_frame_t rx_frame; can_get_next_frame(rx_frame);)
		{
			if (rx_frame.MsgID == VIDE_NET_RESPONSE)
			{
				/* Handle vide net packet. Remove request if response handled. */
				eraseVideNetRequestIf([&](std::shared_ptr<videnet::VideNetRequest> req)->bool {
					return req->onResponse(rx_frame);
				});
			}
		}
	}

	void EstymaClient::queueVideNetRequest(std::shared_ptr<VideNetRequest> request_sp)
	{
		if (can_write_frame(request_sp->prepareMessage()) && request_sp->needWaitForReply())
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

	void EstymaClient::uploadValuesToMqtt()
	{
		if (auto mqttConnection = mqttConn_wp.lock())
		{
			if (mqttConnection->isConnected())
			{
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
					statusLed_sp->setBlinking(75, 4);
			}
		}
	}

	void EstymaClient::handleVideNetPeriodicOps()
	{
		if (millis() - lastVideNetPing > VIDE_NET_PING_DELAY)
		{
			/* Send ping packet to kettle. It brings up our module in CAN network node list. */
			sendVideNetRequest<VideNetPing>();

			/* Read params and post them. */
			uploadValuesToMqtt();

			/* Set current time as last ping time. */
			lastVideNetPing = millis();
		}

		/* Call cleanup method to kick out timed out requests. */
		eraseVideNetRequestIf([](std::shared_ptr<videnet::VideNetRequest> req)->bool {
			return millis() - req->getSendingTime() > KSF_ONE_SECOND_MS;
		});
	}

	bool EstymaClient::loop()
	{
		if (is_can_module_running())
		{
			/* Handles pending messages in queue. */
			handleMessageQueue();

			/* Handles periodic VideNet operations (clean up requests, send info to MQTT etc...). */
			handleVideNetPeriodicOps();
		}

		return true;
	}
}