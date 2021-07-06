#pragma once
#include <ksIotFrameworkLib.h>
#include <atomic>
namespace comps
{
	class BoilerStatusUpdater;

	namespace EstymaTempSensorType
	{
		enum TYPE
		{
			CTZ_01,
			CT_2A,
			CTP_02,
			CT_3B
		};
	}

	class EstymaCANClient : public ksf::ksComponent
	{
		protected:
			std::weak_ptr<ksf::ksMqttConnector> mqtt_wp;
			std::weak_ptr<ksf::ksLed> statusLed_wp;
			std::weak_ptr<BoilerStatusUpdater> boilerStatusUpdater_wp;

			std::shared_ptr<ksf::ksEventHandle> connEventHandle_sp, disEventHandle_sp, otaStartEventHandle_sp;

			bool isBound = false;
			intr_handle_t eccCANInterruptHandle = NULL;

			double calcSteinhart(double a, double b, double c, double r1, double x) const;
			float calculateTemperature(uint16_t adcValue, EstymaTempSensorType::TYPE sensorType) const;

			uint8_t readCANReg(uint8_t address) const;

			static void ICACHE_RAM_ATTR staticInterruptWrapper(void* eccPtr);
			void ICACHE_RAM_ATTR handleCANBusInterrupt();

			void onMqttConnected();
			void onMqttDisconnected();

			void bindCAN();
			void unbindCAN();

		public:
			EstymaCANClient();

			unsigned int getRxErrorCount() const;
			unsigned int getTxErrorCount() const;

			bool init(ksf::ksComposable* owner) override;
			
			virtual ~EstymaCANClient();
	};
}
