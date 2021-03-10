#include "BoilerStatusUpdater.h"
#include "../../../board.h"
#include <WString.h>

namespace comps
{
	const char* BoilerStatusUpdater::tempChNames[TemperatureType::MAX] = { "boiler_temp", "boiler2_temp", "cwu_temp", "exhaust_temp" };

	BoilerStatusUpdater::BoilerStatusUpdater()
	{
		for (unsigned int i = 0; i < TemperatureType::MAX; ++i)
			temperatures[i] = 0;
	}

	bool BoilerStatusUpdater::init(ksf::ksComposable* owner)
	{
		mqtt_wp = owner->findComponent<ksf::ksMqttConnector>();
		led_wp = owner->findComponent<ksf::ksLed>();

		return true;
	}

	void BoilerStatusUpdater::updateTemperature(TemperatureType::TYPE type, float value)
	{
		temperatures[type] = value;
	}

	void BoilerStatusUpdater::sendTemperatures(std::shared_ptr<ksf::ksMqttConnector>& mqtt_sp) const
	{
		for (unsigned int i = 0; i < TemperatureType::MAX; ++i)
		{
			String tempStr(temperatures[i], i == TemperatureType::Exhaust ? 0 : 1);
			mqtt_sp->publish(tempChNames[i], tempStr, true);
		}
	}

	void BoilerStatusUpdater::sendStatus(std::shared_ptr<ksf::ksMqttConnector>& mqtt_sp) const
	{
		if (auto led_sp = led_wp.lock())
			led_sp->setBlinking(100, 3);

		sendTemperatures(mqtt_sp);
		mqtt_sp->publish("rotations", String(rotations), true);
	}

	bool BoilerStatusUpdater::loop()
	{
		unsigned int ctime = millis();

		if (ctime - lastPublishTime > MQTT_STATUS_UPDATE_INTERVAL)
		{
			if (auto mqtt_sp = mqtt_wp.lock())
			{
				if (mqtt_sp->isConnected())
				{
					sendStatus(mqtt_sp);
					lastPublishTime = ctime;
				}
			}
		}

		return true;
	}
}