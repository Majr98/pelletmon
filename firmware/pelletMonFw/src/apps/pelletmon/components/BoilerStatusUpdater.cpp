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
		return true;
	}

	const char* BoilerStatusUpdater::tempChNames[TemperatureType::MAX] = 
		{ "boiler_temp", "cwu_temp", "exhaust_temp" };

	void BoilerStatusUpdater::updateTemperature(TemperatureType::TYPE type, float value)
	{
		if (fabsf(value - temperatures[type].second) >= 0.1f)
		{
			temperatures[type].second = value;
			temperatures[type].first = true;
		}
	}

	void BoilerStatusUpdater::sendTemperatures(std::shared_ptr<class ksf::ksMqttConnector>& mqtt_sp)
	{
		for (unsigned int i = 0; i < TemperatureType::MAX; ++i)
		{
			if (temperatures[i].first)
			{
				mqtt_sp->publish(tempChNames[i], String(temperatures[i].second, 1), true);
				temperatures[i].first = false;
			}
		}
	}

	bool BoilerStatusUpdater::loop()
	{
		if (auto mqtt_sp = mqtt_wp.lock())
		{
			unsigned int ctime = millis();
			if (ctime - lastTempUpdate > 1000)
			{
				sendTemperatures(mqtt_sp);
				lastTempUpdate = ctime;
			}

			if (ctime - lastStatusUpdate > 15000)
			{
				mqtt_sp->publish("rotations", String(rotations), true);
				lastStatusUpdate = ctime;
			}
		}

		return true;
	}
}