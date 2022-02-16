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
			std::weak_ptr<ksf::ksMqttConnector> mqttConn_wp;													//< Weak pointer for mqtt connection.
			std::weak_ptr<ksf::ksLed> statusLed_wp;																//< Weak pointer for status LED.
			std::shared_ptr<ksf::ksEventHandle> connEventHandle_sp, disEventHandle_sp, msgEventHandle_sp;		//< Shared event pointers.
			
			ksf::ksSafeList<std::shared_ptr<videnet::VideNetRequest>> videNetRequests;		//< Safe list of pending Vide Net requests.
			unsigned long lastVideNetPing = 0;												//< Last VideNet ping packet send time (millis).

			/* 
				Event handler method called when MQTT service connected to server. 
			*/
			void onMqttConnected();

			/* 
				Event handler method called when MQTT service receives a message.
				@param topic - reference of topic string.
				@param message - reference of message string.
			*/
			void onMqttMessage(const String& topic, const String& message);

			/*
				Event handler method called when MQTT service disconnected.
			*/
			void onMqttDisconnected();

			/*
				Method used to queue VideNet request (pushes request to videNetRequest list).
				@param request_sp - shared pointer of VideNetRequest.
			*/
			void queueVideNetRequest(std::shared_ptr<videnet::VideNetRequest> request_sp);

			/*
				Template method used to create and queue new VideNetRequest with specified type.
				@params ... - params passed to VideNetRequest constructor.
			*/
			template <class Type, class... Params>
			std::weak_ptr<Type> sendVideNetRequest(Params...rest)
			{
				std::shared_ptr<Type> ptr = std::make_shared<Type>(rest...);
				queueVideNetRequest(ptr);
				return std::weak_ptr<Type>(ptr);
			}

			/*
				Template method used to create and queue new VideNetRequest with specified type.
			*/
			template <class Type>
			std::weak_ptr<Type> sendVideNetRequest()
			{
				std::shared_ptr<Type> ptr = std::make_shared<Type>();
				queueVideNetRequest(ptr);
				return std::weak_ptr<Type>(ptr);
			}

			/*
				Method that tries to publish passed message (value) to requested MQTT topic.
				@param topic - reference to topic string.
				@param value - reference to value/message string.
			*/
			void tryPublishToMqtt(const String& topic, const String& value) const;

			/*
				Method that processes CAN message queue, calls callbacks etc. 
			*/
			void handleMessageQueue();
			
			/*
				Method that handles periodic VideNet operations, like ping, status update etc.
			*/
			void handleVideNetPeriodicOps();

			/*
				Method that triggers value update to be sent to MQTT server (All values + controller state).
			*/
			void uploadValuesToMqtt();

			/*
				Method that triggers value update to be sent to MQTT server, but only for controller state.
			*/
			void uploadControllerSetupStateToMqtt();

		public:

			/*
				VideNetClient constructor method.
				@param canRxPin - CAN BUS RX (to SN65HVD230 chip) pin number.
				@param canTxPin - CAN BUS TX (to SN65HVD230 chip) pin number.
			*/
			VideNetClient(uint8_t canRxPin, uint8_t canTxPin);

			/*
				Method that initializes component.
				@param owner - ksComposable / application owner pointer. Valid only during this call.
				@return - true on init success, otherwise false. False will break whole app init.
			*/
			bool init(ksf::ksComposable* owner) override;

			/*
				Method called every application loop.
				@return - true if you want to continue execution, false to break app execution.
			*/
			bool loop() override;

			/*
				Method used to force CAN circuit to stop. Used before reset (for example in OTA process).
			*/
			void forceCanStop();
	};
}
