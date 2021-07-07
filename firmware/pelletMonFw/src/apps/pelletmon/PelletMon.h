#pragma once

#include <ksIotFrameworkLib.h>

namespace comps
{
	class CanService;
}

class PelletMon : public ksf::ksApplication
{
	protected:
		std::weak_ptr<comps::CanService> canService_wp;

	public:
		bool init() override;
		bool loop() override;
};