#pragma once

#include <ksIotFrameworkLib.h>

namespace comps
{
	class EstymaCANClient;
}

class PelletMon : public ksf::ksApplication
{
	protected:
		std::weak_ptr<comps::EstymaCANClient> canclient_wp;

	public:
		DECLARE_KS_EVENT(otaStartEvent)

		bool init() override;
		bool loop() override;
};