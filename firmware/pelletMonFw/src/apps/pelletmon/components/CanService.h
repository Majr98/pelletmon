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
			uint32_t u32[2];
			uint16_t u16[4];
			uint8_t u8[8];
		};

		CanMessage() {}

		CanMessage(long _frameId, uint8_t _dataLen, const uint8_t* buf, uint8_t bufLen)
			: frameId(_frameId), dataLen(_dataLen)
		{
			if (dataLen > sizeof(u8))
				dataLen = sizeof(u8);

			if (bufLen > dataLen)
				bufLen = dataLen;

			memcpy(u8, buf, bufLen);
		}

		CanMessage(long _frameId, uint8_t _dataLen, uint64_t _u64)
			: frameId(_frameId), dataLen(_dataLen), u64(_u64) {}

		CanMessage(long _frameId, uint8_t _dataLen, uint32_t _u32[2]) 
			: frameId(_frameId), dataLen(_dataLen)
		{
			memcpy(u32, _u32, sizeof(u32));
		}

		CanMessage(long _frameId, uint8_t _dataLen, uint16_t _u16[4]) 
			: frameId(_frameId), dataLen(_dataLen)
		{
			memcpy(u16, _u16, sizeof(u16));
		}

		CanMessage(long _frameId, uint8_t _dataLen, uint8_t _u8[8])
			: frameId(_frameId), dataLen(_dataLen)
		{
			memcpy(u8, _u8, sizeof(u8));
		}
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

			bool startService(unsigned int canServiceSpeed, CanServiceSubscribeInfo* subscribeInfo, unsigned int subscribeInfoLen);
			bool sendMessage(const CanMessage& inMessage) const;
			bool receiveMessage(CanMessage& outMessage);
			void stopService();

			virtual ~CanService();
	};
}
