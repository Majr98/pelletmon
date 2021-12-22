#pragma once
#include <ksIotFrameworkLib.h>

namespace comps
{
	class VideNetClient;
}

class PelletMon : public ksf::ksApplication
{
	protected:
		std::weak_ptr<comps::VideNetClient> videNetClient_wp;

	public:
		bool init() override;
		bool loop() override;
};