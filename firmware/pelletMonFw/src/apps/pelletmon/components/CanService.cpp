#include "CanService.h"
#include <CAN.h>
#include "../../../board.h"

/*
	CAN REGs for low level stuff.
*/
#define REG_BASE 0x3ff6b000
#define REG_IR 0x03

namespace comps
{
	bool CanService::init(ksf::ksComposable* owner)
	{
		/* Setup interface pins. */
		CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
		return true;
	}

	void ICACHE_RAM_ATTR CanService::staticInterruptWrapper(void* serviceObject)
	{
		/* Jump to member function. */
		((CanService*)serviceObject)->handleCanIntterupt();
	}

	uint8_t ICACHE_RAM_ATTR CanService::readCanRegistry(uint8_t address) const
	{
		volatile uint32_t* reg = (volatile uint32_t*)(REG_BASE + address * 4);
		return *reg;
	}

	void ICACHE_RAM_ATTR CanService::handleCanIntterupt()
	{
		/* Check if tis is interrupt. */
		if (!(readCanRegistry(REG_IR) & 0x01)) 
			return;

		/* Check if packet is valid and skip RTRs. */
		if (CAN.parsePacket() > 0 && !CAN.packetRtr())
		{
			BaseType_t xHigherPriorityTaskWoken;
			auto ctime = millis();

			/* Iterate over all subscribed messages. */
			for (auto& subMsg : subscribedMessages)
			{
				/* If packet matches subscribed one, then check if it should be queued (based on period). */
				if ((CAN.packetId() == subMsg.frameId) && ((subMsg.period == 0) || (millis() - subMsg.lastTick > subMsg.period)))
				{
					/* Update last tick to current time. */
					subMsg.lastTick = ctime;

					/* Prepare CanMessage for packet. */
					uint64_t data = 0;
					CAN.readBytes((uint8_t*)&data, CAN.packetDlc());
					CanMessage msg{ CAN.packetId(), (uint8_t)CAN.packetDlc(), data };

					/* Queue CanMessage to rxQueue. */
					xQueueSendFromISR(rxQueueHandle, &msg, &xHigherPriorityTaskWoken);
				}
			}
		}
	}

	bool CanService::startService(unsigned int canServiceSpeed, CanServiceSubscribeInfo* subscribeInfo, unsigned int sizeofSubscribeInfo)
	{
		if (!canInterruptHandle)
		{
			/* Calculate item count. */
			unsigned int subscribeInfoCount = sizeofSubscribeInfo / sizeof(CanServiceSubscribeInfo);

			/* Start CAN BUS. */
			if (CAN.begin(canServiceSpeed))
			{
				/* Fill subscription list. */
				for (unsigned int i = 0; i < subscribeInfoCount; ++i)
				{
					auto& csi = subscribeInfo[i];
					subscribedMessages.push_back({csi.frameId, csi.period, 0});
				}

				/* Creqte RX queue. */
				rxQueueHandle = xQueueCreate(10, sizeof(CanMessage));

				/* Observe mode. */
				CAN.observe();

				/* Setup interrupt. */
				esp_intr_alloc(ETS_CAN_INTR_SOURCE, ESP_INTR_FLAG_IRAM, CanService::staticInterruptWrapper, this, &canInterruptHandle);

				return true;
			}
		}

		return false;
	}

	bool CanService::receiveMessage(CanMessage& outMessage)
	{
		/* If we have canInterruptHandle, then simply receive from queue. */
		if (canInterruptHandle)
			return xQueueReceive(rxQueueHandle, (void*)&outMessage, 0) == pdPASS;

		return false;
	}
	
	void CanService::resetCan()
	{
		volatile uint32_t* reg = (volatile uint32_t*)(REG_BASE);
		*reg = (*reg & ~0x17) | 0x01;
	}

	void CanService::stopService()
	{
		/* Free interrupt. */
		if (canInterruptHandle)
		{
			/* Free interrupt and queue. */
			esp_intr_free(canInterruptHandle);
			canInterruptHandle = nullptr;
			vQueueDelete(rxQueueHandle);
			rxQueueHandle = nullptr;

			/* Reset CAN device and close interface. */
			resetCan();
			CAN.end();

			/* Clear message list. */
			subscribedMessages.clear();
		}
	}

	CanService::~CanService()
	{
		stopService();
	}
}
