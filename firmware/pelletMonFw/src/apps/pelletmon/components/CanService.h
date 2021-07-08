#pragma once
#include <ksIotFrameworkLib.h>

namespace comps
{
	struct CanServiceSubscribeInfo
	{
		long frameId;
		unsigned long period;
	};

	struct CanServiceSubscribeEntry
	{
		long frameId;
		unsigned long period;
		unsigned long lastTick;
	};

	struct CanMessage
	{
		long frameId;
		uint8_t dataLen;

		union
		{
			uint64_t u64;
			uint16_t u16[4];
			uint8_t u8[8];
		};
	};

	class CanService : public ksf::ksComponent
	{
		private:
			static void ICACHE_RAM_ATTR staticInterruptWrapper(void* serviceObject);
			void ICACHE_RAM_ATTR handleCanIntterupt();
			
			uint8_t ICACHE_RAM_ATTR readCanRegistry(uint8_t address) const;
			void resetCan();

			intr_handle_t canInterruptHandle = nullptr;
			QueueHandle_t rxQueueHandle = nullptr;

			std::vector<CanServiceSubscribeEntry> subscribedMessages;

		public:
			bool init(ksf::ksComposable* owner) override;

			bool StartService(unsigned int canServiceSpeed, CanServiceSubscribeInfo* subscribeInfo, unsigned int subscribeInfoLen);
			bool ReceiveMessage(CanMessage& outMessage);
			void StopService();

			virtual ~CanService();
	};
}
