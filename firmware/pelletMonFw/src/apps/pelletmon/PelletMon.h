#pragma once
#include <ksIotFrameworkLib.h>

class PelletMon : public ksf::ksApplication
{
	public:
		bool init() override;
		bool loop() override;
};