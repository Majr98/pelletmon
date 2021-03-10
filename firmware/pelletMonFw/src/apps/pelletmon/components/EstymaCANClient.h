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
		
			double calculateTemperature(uint16_t x) const;
			double calculateExhaustTemperature(uint16_t x) const;

			static void onInterruptWrapper(void* eccPtr);
			uint8_t readCANReg(uint8_t address) const;

		public:
			EstymaCANClient();

			unsigned int getRxErrorCount() const;
			unsigned int getTxErrorCount() const;

			void bindCAN();
			void unbindCAN();

			bool init(class ksf::ksComposable* owner) override;
			bool loop() override;
			void handleInterrupt();

			virtual ~EstymaCANClient();
	};
}
