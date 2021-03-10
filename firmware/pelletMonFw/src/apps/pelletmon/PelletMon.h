#pragma once

#include <ksIotFrameworkLib.h>

namespace ksf
{
	class ksMqttConnector;
	class ksEventHandle;
	class ksLed;
}

namespace comps
{
	class EstymaCANClient;
}

class PelletMon : public ksf::ksApplication
{
	protected:
		std::weak_ptr<ksf::ksMqttConnector> mqtt_wp;
		std::weak_ptr<ksf::ksLed> statusLed_wp;
		std::weak_ptr<comps::EstymaCANClient> canclient_wp;

		std::shared_ptr<ksf::ksEventHandle> connEventHandle_sp, disEventHandle_sp;

		void onMqttConnected();
		void onMqttDisconnected();

	public:
		bool init() override;
		bool loop() override;
};