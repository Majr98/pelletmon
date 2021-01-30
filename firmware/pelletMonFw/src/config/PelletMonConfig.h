#pragma once
#include <ksIotFrameworkLib.h>

class PelletMonConfig : public ksf::ksApplication
{
	public:
		static const char pelletMonDeviceName[];

		bool init() override;
		bool loop() override;
};