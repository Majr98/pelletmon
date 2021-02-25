#pragma once
#include <ksIotFrameworkLib.h>
#include "Arduino.h"

namespace comps
{
	class EstymaCANCLient : public ksf::ksComponent
	{
		protected:
			std::weak_ptr<class ksf::ksLed> statusLed_wp;

		public:
			EstymaCANCLient();

			bool init(class ksf::ksComposable* owner) override;
			bool loop() override;

			virtual ~EstymaCANCLient();
	};
}
