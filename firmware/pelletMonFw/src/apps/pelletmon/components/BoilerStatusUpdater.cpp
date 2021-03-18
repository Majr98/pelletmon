#include "BoilerStatusUpdater.h"
#include "../../../board.h"
#include <WString.h>

namespace comps
{
	const char* BoilerStatusUpdater::floatChannelNames[FloatValueType::MAX] = { "boiler_temp", "cwu_temp", "exhaust_temp" };
	const char* BoilerStatusUpdater::uIntChannelNames[UIntValueType::MAX]	= { "rotations" };

	BoilerStatusUpdater::BoilerStatusUpdater()
	{
		/* Set float values to zero. */
		for (unsigned int i = 0; i < FloatValueType::MAX; ++i)
			floatValues[i] = 0;

		/* Set uint values to zero. */
		for (unsigned int i = 0; i < UIntValueType::MAX; ++i)
			uIntValues[i] = 0;
	}

	bool BoilerStatusUpdater::init(ksf::ksComposable* owner)
	{
		/* Grab weak pointer for MQTT Connector. */
		mqtt_wp = owner->findComponent<ksf::ksMqttConnector>();

		/* Grab weak pointer for Status LED */
		led_wp = owner->findComponent<ksf::ksLed>();

		return true;
	}

	void ICACHE_RAM_ATTR BoilerStatusUpdater::updateFloatValue(FloatValueType::TYPE type, float value)
	{
		/* Cache atomic float value. */
		floatValues[type] = value;
	}

	void ICACHE_RAM_ATTR BoilerStatusUpdater::updateUIntValue(UIntValueType::TYPE type, unsigned int value)
	{
		/* Cache atomic unsigned int value. */
		uIntValues[type] = value;
	}

	void BoilerStatusUpdater::sendTelemetryValues(std::shared_ptr<ksf::ksMqttConnector>& mqtt_sp) const
	{
		/* Bling LED three times. */
		if (auto led_sp = led_wp.lock())
			led_sp->setBlinking(100, 3);

		/* Send cached float values to MQTT. */
		for (unsigned int i = 0; i < FloatValueType::MAX; ++i)
		{
			String floatStr(floatValues[i], i == FloatValueType::Temperature_Exhaust ? 0 : 1);
			mqtt_sp->publish(floatChannelNames[i], floatStr, false);
		}

		/* Send cached unsigned int values to MQTT. */
		for (unsigned int i = 0; i < UIntValueType::MAX; ++i)
		{
			String uintStr(uIntValues[i]);
			mqtt_sp->publish(uIntChannelNames[i], uintStr, false);
		}
	}

	bool BoilerStatusUpdater::loop()
	{
		unsigned int ctime = millis();

		/* If MQTT_STATUS_UPDATE_INTERVAL passed, send values to MQTT. */
		if (ctime - lastPublishTime > MQTT_STATUS_UPDATE_INTERVAL)
		{
			if (auto mqtt_sp = mqtt_wp.lock())
			{
				if (mqtt_sp->isConnected())
				{
					sendTelemetryValues(mqtt_sp);
					lastPublishTime = ctime;
				}
			}
		}

		return true;
	}
}