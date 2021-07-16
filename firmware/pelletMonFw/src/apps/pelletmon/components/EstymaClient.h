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

			void queueVideNetRequest(std::weak_ptr<videnet::VideNetChangeParamRequest> request_wp);

			template <class Type, class... Params>
			std::weak_ptr<Type> sendVideNetRequest(Params...rest)
			{
				std::shared_ptr<Type> ptr = std::make_shared<Type>(rest...);
				queueVideNetRequest(ptr);
				return std::weak_ptr<Type>(ptr);
			}

			template <class Type>
			std::weak_ptr<Type> sendVideNetRequest()
			{
				std::shared_ptr<Type> ptr = std::make_shared<Type>();
				queueVideNetRequest(ptr);
				return std::weak_ptr<Type>(ptr);
			}

		public:
			bool init(ksf::ksComposable* owner) override;
			bool loop() override;
	};
}
