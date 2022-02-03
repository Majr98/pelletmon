#pragma once
#include <ksIotFrameworkLib.h>

namespace comps
{
	class VideNetClient;
}

class PelletMon : public ksf::ksApplication
{
	protected:
		std::weak_ptr<comps::VideNetClient> videNetClient_wp;		//< Weak pointer to VideNetClient component.

	public:
		/*
			Method called on application initialize.
			Call base class after setting up all the components!.
			@return - true on init success, otherwise false.
		*/
		bool init() override;

		/*
			Method calle every application loop.
			@return - true to continue application execution, false to break application execution.
		*/
		bool loop() override;
};