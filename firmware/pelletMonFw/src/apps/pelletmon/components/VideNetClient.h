#pragma once
#include <ksIotFrameworkLib.h>

namespace videnet
{
	class VideNetRequest;
}

namespace comps
{
	class VideNetClient : public ksf::ksComponent
	{
		protected:
			std::weak_ptr<ksf::ksMqttConnector> mqttConn_wp;
			std::weak_ptr<ksf::ksLed> statusLed_wp;
			std::shared_ptr<ksf::ksEventHandle> connEventHandle_sp, disEventHandle_sp, msgEventHandle_sp;
			
			std::vector<std::shared_ptr<videnet::VideNetRequest>> videNetRequests, stillValidRequests;
			unsigned long lastVideNetPing = 0;

			void onMqttConnected();
			void onMqttMessage(const String& topic, const String& message);
			void onMqttDisconnected();

			void queueVideNetRequest(std::shared_ptr<videnet::VideNetRequest> request_sp);

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

			void tryPublishToMqtt(const char* topic, const String& value) const;

			void handleMessageQueue();
			void handleVideNetPeriodicOps();

			void uploadValuesToMqtt();

		public:
			bool init(ksf::ksComposable* owner) override;
			bool loop() override;

			void forceCanStop();
	};
}
