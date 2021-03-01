#pragma once
#include <ksIotFrameworkLib.h>

namespace comps
{
	namespace TemperatureType
	{
		enum TYPE
		{
			Boiler,
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
			std::pair<bool, double> temperatures[TemperatureType::MAX];

			unsigned int rotations = 0;
			unsigned int lastTempUpdate = 0;
			unsigned int lastStatusUpdate = 0;

			std::weak_ptr<ksf::ksMqttConnector> mqtt_wp;
			std::weak_ptr<ksf::ksLed> led_wp;

		public:
			BoilerStatusUpdater();

			bool sendTemperatures(std::shared_ptr<class ksf::ksMqttConnector>& mqtt_sp);
			void updateTemperature(TemperatureType::TYPE type, double value);

			bool init(class ksf::ksComposable* owner) override;
			bool loop() override;
	};
}
