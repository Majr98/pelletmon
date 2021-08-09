#pragma once
#include <ksIotFrameworkLib.h>

namespace comps
{
	class EstymaClient;
}

class PelletMon : public ksf::ksApplication
{
	protected:
		std::weak_ptr<comps::EstymaClient> estymaClient_wp;

	public:
		bool init() override;
		bool loop() override;
};