#pragma once
#include <ksIotFrameworkLib.h>

namespace comps
{
	class BoilerStatusUpdater;
	class EstymaCANClient : public ksf::ksComponent
	{
		protected:
			std::weak_ptr<BoilerStatusUpdater> boilerStatusUpdater_wp;

			bool isBound = false;
			intr_handle_t eccCANInterruptHandle = NULL;
		
			float calculateTemperature(uint16_t x) const;
			float calculateExhaustTemperature(uint16_t x) const;

			uint8_t readCANReg(uint8_t address) const;

			static void staticInterruptWrapper(void* eccPtr);

		public:
			EstymaCANClient();

			unsigned int getRxErrorCount() const;
			unsigned int getTxErrorCount() const;

			void bindCAN();
			void unbindCAN();

			bool init(class ksf::ksComposable* owner) override;
			void handleCANBusInterrupt();

			virtual ~EstymaCANClient();
	};
}
