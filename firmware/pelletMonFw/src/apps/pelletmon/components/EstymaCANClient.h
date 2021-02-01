#pragma once
#include <ksIotFrameworkLib.h>
#include "Arduino.h"

namespace comps
{
	class EstymaCANCLient : public ksf::ksComponent
	{
		protected:
			
		public:
			EstymaCANCLient();

			bool init(class ksf::ksComposable* owner) override;
			bool loop() override;

			virtual ~EstymaCANCLient();
	};
}
