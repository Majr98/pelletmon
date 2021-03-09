#pragma once
#include <ksIotFrameworkLib.h>

namespace comps
{
	namespace TemperatureType
	{
		enum TYPE
		{
			Boiler,
			Boiler2,
			Cwu,
			Exhaust,
			MAX
		};
	}

	class BoilerStatusUpdater : public ksf::ksComponent
	{
		friend class EstymaCANClient;

		protected:
			static const char* tempChNames[TemperatureType::MAX];
			double temperatures[TemperatureType::MAX];
			unsigned int rotations = 0;
			unsigned int lastStatusUpdate = 0;

			std::weak_ptr<ksf::ksMqttConnector> mqtt_wp;
			std::weak_ptr<ksf::ksLed> led_wp;

			void sendStatus(std::shared_ptr<class ksf::ksMqttConnector>& mqtt_sp) const;
			void sendTemperatures(std::shared_ptr<class ksf::ksMqttConnector>& mqtt_sp) const;

		public:
			BoilerStatusUpdater();

			void updateTemperature(TemperatureType::TYPE type, double value);

			bool init(class ksf::ksComposable* owner) override;
			bool loop() override;
	};
}
