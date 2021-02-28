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
			std::pair<bool, float> temperatures[TemperatureType::MAX];

			unsigned int rotations = 0;
			unsigned int lastTempUpdate = 0;
			unsigned int lastStatusUpdate = 0;

			std::weak_ptr<ksf::ksMqttConnector> mqtt_wp;

		public:
			BoilerStatusUpdater();

			void sendTemperatures(std::shared_ptr<class ksf::ksMqttConnector>& mqtt_sp);
			void updateTemperature(TemperatureType::TYPE type, float value);

			bool init(class ksf::ksComposable* owner) override;
			bool loop() override;
	};
}
