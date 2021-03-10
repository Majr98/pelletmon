#pragma once
#include <ksIotFrameworkLib.h>
#include <atomic>

namespace comps
{
	namespace FloatValueType
	{
		enum TYPE
		{
			Temperature_Boiler,
			Temperature_Boiler2,
			Temperature_PotableWater,
			Temperature_Exhaust,
			MAX
		};
	}

	namespace UIntValueType
	{
		enum TYPE
		{
			Rotations,
			MAX
		};
	}

	class BoilerStatusUpdater : public ksf::ksComponent
	{
		friend class EstymaCANClient;

		protected:
			static const char* floatChannelNames[FloatValueType::MAX];
			std::atomic<float> floatValues[FloatValueType::MAX];

			static const char* uIntChannelNames[UIntValueType::MAX];
			std::atomic<unsigned int> uIntValues[UIntValueType::MAX];

			unsigned int lastPublishTime = 0;

			std::weak_ptr<ksf::ksMqttConnector> mqtt_wp;
			std::weak_ptr<ksf::ksLed> led_wp;

			void sendValues(std::shared_ptr<class ksf::ksMqttConnector>& mqtt_sp) const;

		public:
			BoilerStatusUpdater();

			void updateFloatValue(FloatValueType::TYPE type, float value);
			void updateUIntValue(UIntValueType::TYPE type, unsigned int value);

			bool init(class ksf::ksComposable* owner) override;
			bool loop() override;
	};
}
