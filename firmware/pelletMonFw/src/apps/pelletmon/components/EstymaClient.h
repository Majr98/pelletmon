#pragma once
#include <ksIotFrameworkLib.h>

namespace videnet
{
	class VideNetChangeParamRequest;
}

namespace comps
{
	class CanService;

	class EstymaClient : public ksf::ksComponent
	{
		protected:
			std::weak_ptr<CanService> canService_wp;
			std::weak_ptr<ksf::ksMqttConnector> mqttConn_wp;
			std::weak_ptr<ksf::ksLed> statusLed_wp;
			std::shared_ptr<ksf::ksEventHandle> connEventHandle_sp, disEventHandle_sp, debugMessageEventHandle_sp;
			
			void onMqttConnected();
			void onMqttDisconnected();
			void onDebugMessage(ksf::ksMqttDebugResponder*, const String& message, bool& consumed);

			void sendVideNetRequest(const videnet::VideNetChangeParamRequest& videNetRequest);

		public:
			bool init(ksf::ksComposable* owner) override;
			bool loop() override;
	};
}
