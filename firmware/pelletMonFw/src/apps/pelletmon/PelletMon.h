#pragma once

#include <ksIotFrameworkLib.h>

namespace ksf
{
	class ksMqttConnector;
	class ksEventHandle;
	class ksLed;
}

class PelletMon : public ksf::ksApplication
{
	protected:
		std::shared_ptr<ksf::ksMqttConnector> mqtt;

		std::shared_ptr<ksf::ksLed> statusLed;
		std::shared_ptr<ksf::ksEventHandle> connEventHandle, msgEventHandle, disEventHandle;

		void onMqttMessage(const String& topic, const String& payload);
		void onMqttConnected();
		void onMqttDisconnected();

	public:
		bool init() override;
		bool loop() override;
};