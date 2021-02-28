#pragma once
#include <ksIotFrameworkLib.h>

namespace comps
{
	class BoilerStatusUpdater;
	class EstymaCANClient : public ksf::ksComponent
	{
		protected:
			std::weak_ptr<class ksf::ksLed> statusLed_wp;
			std::weak_ptr<BoilerStatusUpdater> boilerStatusUpdater_wp;

			double calculateTemperature(short x) const;
			double calculateExhaustTemperature(short x) const;
			
		public:
			EstymaCANClient();

			bool init(class ksf::ksComposable* owner) override;
			bool loop() override;

			virtual ~EstymaCANClient();
	};
}
