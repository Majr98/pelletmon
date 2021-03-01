#include "BoilerStatusUpdater.h"
#include "../../../board.h"
#include <WString.h>

namespace comps
{
	BoilerStatusUpdater::BoilerStatusUpdater()
	{
		for (unsigned int i = 0; i < TemperatureType::MAX; ++i)
			temperatures[i].first = false;
	}

	bool BoilerStatusUpdater::init(ksf::ksComposable* owner)
	{
		mqtt_wp = owner->findComponent<ksf::ksMqttConnector>();
		led_wp = owner->findComponent<ksf::ksLed>();

		return true;
	}

	const char* BoilerStatusUpdater::tempChNames[TemperatureType::MAX] = 
		{ "boiler_temp", "cwu_temp", "exhaust_temp" };

	void BoilerStatusUpdater::updateTemperature(TemperatureType::TYPE type, double value)
	{
		if (value > -10.0 && value < 500.0)
		{
			double minChange = type == TemperatureType::Exhaust ? 2.0f : 0.25f;

			if (fabs(value - temperatures[type].second) >= minChange)
			{
				temperatures[type].second = value;
				temperatures[type].first = true;
			}
		}
	}

	bool BoilerStatusUpdater::sendTemperatures(std::shared_ptr<class ksf::ksMqttConnector>& mqtt_sp)
	{
		bool bWasDirty = false;
		for (unsigned int i = 0; i < TemperatureType::MAX; ++i)
		{
			if (temperatures[i].first)
			{
				String tempStr(temperatures[i].second, i == TemperatureType::Exhaust ? 0 : 1);
				mqtt_sp->publish(tempChNames[i], tempStr, true);
				temperatures[i].first = false;
				bWasDirty = true;
			}
		}

		return bWasDirty;
	}

	bool BoilerStatusUpdater::loop()
	{
		auto led_sp = led_wp.lock();

		if (auto mqtt_sp = mqtt_wp.lock())
		{
			if (mqtt_sp->isConnected())
			{
				unsigned int ctime = millis();
				if (ctime - lastTempUpdate > 1000)
				{
					if (sendTemperatures(mqtt_sp) && led_sp)
						led_sp->setBlinking(25, 2);

					lastTempUpdate = ctime;
				}

				if (ctime - lastStatusUpdate > 15000)
				{
					mqtt_sp->publish("rotations", String(rotations), true);
					lastStatusUpdate = ctime;

					if (led_sp) led_sp->setBlinking(25, 2);
				}
			}
		}

		return true;
	}
}