#pragma once
#include <ksIotFrameworkLib.h>
#include <atomic>
namespace comps
{
	class BoilerStatusUpdater;
	class EstymaCANClient : public ksf::ksComponent
	{
		protected:
			std::weak_ptr<ksf::ksMqttConnector> mqtt_wp;
			std::weak_ptr<ksf::ksLed> statusLed_wp;
			std::weak_ptr<BoilerStatusUpdater> boilerStatusUpdater_wp;

			std::shared_ptr<ksf::ksEventHandle> connEventHandle_sp, disEventHandle_sp, otaStartEventHandle_sp;

			bool isBound = false;
			intr_handle_t eccCANInterruptHandle = NULL;
		
			float calculateTemperature(uint16_t x) const;
			float calculateExhaustTemperature(uint16_t x) const;

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
