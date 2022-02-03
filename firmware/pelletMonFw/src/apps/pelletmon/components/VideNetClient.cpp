#include "../../../board.h"
#include "../videnet/VideNet.h"
#include "VideNetClient.h"
#include <esp32_can.h>

using namespace videnet;
using namespace std::placeholders;

namespace comps
{
	bool VideNetClient::init(ksf::ksComposable* owner)
	{
		/* Grab weak pointer for MQTT Connector. */
		mqttConn_wp = owner->findComponent<ksf::ksMqttConnector>();

		/* Grab weak pointer for LED. */
		statusLed_wp = owner->findComponent<ksf::ksLed>();

		/* Bind to MQTT events. */
		if (auto mqtt_sp = mqttConn_wp.lock())
		{
			mqtt_sp->onConnected->registerEvent(connEventHandle_sp, std::bind(&VideNetClient::onMqttConnected, this));
			mqtt_sp->onMesssage->registerEvent(msgEventHandle_sp, std::bind(&VideNetClient::onMqttMessage, this, _1, _2));
			mqtt_sp->onDisconnected->registerEvent(disEventHandle_sp, std::bind(&VideNetClient::onMqttDisconnected, this));
		}

		/* Pre-initialize CAN bus. */
		CAN0.setCANPins((gpio_num_t)CAN_RX_PIN, (gpio_num_t)CAN_TX_PIN);
		CAN0.init(CAN_PROTO_SPEED);

		/* Pre-initialize filter - accept only VIDE NET responses. */
		CAN0.watchFor(VIDE_NET_RESPONSE);

		return true;
	}

	void VideNetClient::onMqttConnected()
	{
		/* Stop blinking status led on MQTT connect. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(0);

		/* Subscribe to 'set' messages. */
		if (auto mqtt_sp = mqttConn_wp.lock())
			mqtt_sp->subscribe("set/#");

		/* Run CAN service. */
		CAN0.enable();
	}
	
	void VideNetClient::onMqttDisconnected()
	{
		/* Clear pending requests. */
		videNetRequests.unsafeEraseAllQueues();

		/* Start blinking status led on MQTT disconnect. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(500);

		/* Stop CAN service. */
		CAN0.disable();
	}

	void VideNetClient::onMqttMessage(const String& topic, const String& message)
	{
		if (topic.equals("set/controller_enabled"))
		{
			sendVideNetRequest<VideNetSetController>(message.toInt() == 1, [&]() {
				uploadControllerSetupStateToMqtt();
			});
		}
		else if (topic.equals("set/heatmode"))
		{
			uint8_t heatMode = message.toInt();
			sendVideNetRequest<VideNetSetHeatMode>(heatMode < HeatMode::MAX ? heatMode : HeatMode::Off, [&](){
				uploadControllerSetupStateToMqtt();
			});
		}
		else if (topic.equals("set/hotwatermode"))
		{
			uint8_t hotWaterMode = message.toInt();
			sendVideNetRequest<VideNetSetHotWaterMode>(hotWaterMode < HeatMode::MAX ? hotWaterMode : HeatMode::Off, [&]() {
				uploadControllerSetupStateToMqtt();
			});
		}
		else return;

		/* Blink LED. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(50, 8);

		/* Save settings to kettle EEPROM. */
		sendVideNetRequest<VideNetSaveSettings>();
	}

	void VideNetClient::handleMessageQueue()
	{
		/* Queue remove of timed out requests. */
		for (auto& req : videNetRequests.getList())
		{
			if ((millis() - req->getSendingTime() > KSF_ONE_SECOND_MS))
				videNetRequests.queueRemove(req);
		}

		/* Synchronize requests. */
		videNetRequests.synchronizeQueues();

		for (CAN_FRAME rx_frame; CAN0.read(rx_frame);)
		{
			if (rx_frame.id == VIDE_NET_RESPONSE)
			{
				/* Handle vide net packet. Handle plus remove handled requests. */
				for (auto& req : videNetRequests.getList())
				{
					if (req->onResponse(rx_frame))
						videNetRequests.queueRemove(req);
				}

				/* Synchronize requests. */
				videNetRequests.synchronizeQueues();
			}
		}
	}

	void VideNetClient::queueVideNetRequest(std::shared_ptr<VideNetRequest> request_sp)
	{
		/* If frame has been successfully pushed into CAN bus and expects reply, push item on pending request list. */
		if (CAN0.sendFrame(request_sp->prepareMessage()) && request_sp->needWaitForReply())
			videNetRequests.queueAdd(request_sp);
	}

	void VideNetClient::tryPublishToMqtt(const String& topic, const String& value) const
	{
		if (auto mqttConnection = mqttConn_wp.lock())
			mqttConnection->publish(topic, value);
	}

	void VideNetClient::uploadControllerSetupStateToMqtt()
	{
		/* Request current controller enabled status. */
		sendVideNetRequest<VideNetGetController>([&](bool isEnabled) {
			tryPublishToMqtt("controller_enabled", isEnabled ? "1" : "0");
		});

		/* Request current heat mode. */
		sendVideNetRequest<VideNetGetHeatMode>([&](uint8_t heatMode) {
			tryPublishToMqtt("heatmode_current", String(heatMode));
		});

		/* Request current hot water mode. */
		sendVideNetRequest<VideNetGetHotWaterMode>([&](uint8_t heatMode) {
			tryPublishToMqtt("hotwatermode_current", String(heatMode));
		});
	}

	void VideNetClient::uploadValuesToMqtt()
	{
		/* Request upload whole controller state (enabled, heat mode, hot water mode). */
		uploadControllerSetupStateToMqtt();

		/* Request current kettle temperature. */
		sendVideNetRequest<VideNetGetKettleTemp>([&](uint16_t kettleTemp) {
			tryPublishToMqtt("boiler_temp", String(kettleTemp * 0.1f, 1));
		});

		/* Request current hot water temperature. */
		sendVideNetRequest<VideNetGetHotWaterTemp>([&](uint16_t hotWaterTemp) {
			tryPublishToMqtt("cwu_temp", String(hotWaterTemp * 0.1f, 1));
		});

		/* Request total burner usage (kg * 10). */
		sendVideNetRequest<VideNetGetBurnerUsageTotal>([&](uint32_t totalUsage) {
			tryPublishToMqtt("burnerusage_total", String(totalUsage * 0.1f, 1));
		});

		/* Request current burner power. */
		sendVideNetRequest<VideNetGetBurnerPower>([&](uint8_t powerPercentage) {
			tryPublishToMqtt("burnerpower_current", String(powerPercentage));
		});

		/* Request current burner status. */
		sendVideNetRequest<VideNetGetBurnerStatus>([&](uint8_t currentBurnerStatus) {
			tryPublishToMqtt("burnerstatus_current", String(currentBurnerStatus));
		});

		/* Request current alarm. */
		sendVideNetRequest<VideNetGetAlarmPointer>([&](uint8_t alarmPointer) {
			sendVideNetRequest<VideNetGetAlarmAckTime>(alarmPointer, [=](uint32_t alarmAckTime) {
				uint8_t alarmActive = alarmAckTime == 0 ? 1 : 0;
				tryPublishToMqtt("burner_alarm_active", String(alarmActive));
			});
		});
				
		/* Blink LED. */
		if (auto statusLed_sp = statusLed_wp.lock())
			statusLed_sp->setBlinking(75, 4);
	}

	void VideNetClient::handleVideNetPeriodicOps()
	{
		if (millis() - lastVideNetPing > VIDE_NET_PING_DELAY)
		{
			/* Send ping packet to kettle. It brings up our module in CAN network node list. */
			sendVideNetRequest<VideNetPing>();

			/* Read params and post them. */
			if (auto mqttConnection = mqttConn_wp.lock())
				if (mqttConnection->isConnected())
					uploadValuesToMqtt();
			
			/* Set current time as last ping time. */
			lastVideNetPing = millis();
		}
	}

	bool VideNetClient::loop()
	{
		if (!CAN0.isFaulted())
		{
			/* Handles periodic VideNet operations (clean up requests, send info to MQTT etc...). */
			handleVideNetPeriodicOps();

			/* Handles pending messages in queue. */
			handleMessageQueue();
		}

		return true;
	}

	void VideNetClient::forceCanStop()
	{
		CAN0.disable();
	}
}