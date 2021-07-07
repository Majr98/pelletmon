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
		CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
		return true;
	}

	void ICACHE_RAM_ATTR CanService::staticInterruptWrapper(void* serviceObject)
	{
		((CanService*)serviceObject)->handleCANBusInterrupt();
	}

	uint8_t ICACHE_RAM_ATTR CanService::readCANReg(uint8_t address) const
	{
		volatile uint32_t* reg = (volatile uint32_t*)(REG_BASE + address * 4);
		return *reg;
	}

	void ICACHE_RAM_ATTR CanService::handleCANBusInterrupt()
	{
		if (readCANReg(REG_IR) & 0x01)
		{
			if (CAN.parsePacket() > 0 && !CAN.packetRtr())
			{
				BaseType_t xHigherPriorityTaskWoken;
				auto ctime = millis();

				for (auto& subMsg : subscribedMessages)
				{
					if (CAN.packetId() == subMsg.frameId)
					{
						if (millis() - subMsg.lastTick > subMsg.period)
						{
							subMsg.lastTick = ctime;

							uint64_t data = 0;
							CAN.readBytes((uint8_t*)&data, CAN.packetDlc());
							CanMessage msg{ CAN.packetId(), (uint8_t)CAN.packetDlc(), data };

							xQueueSendFromISR(rxQueueHandle, &msg, &xHigherPriorityTaskWoken);
						}
					}
				}
			}
		}
	}

	bool CanService::StartService(unsigned int canServiceSpeed, CanServiceSubscribeInfo* subscribeInfo, unsigned int subscribeInfoLen)
	{
		if (canInterruptHandle == nullptr)
		{
			/* Start CAN BUS. */
			if (CAN.begin(canServiceSpeed))
			{
				/* Fill subscription list. */
				for (unsigned int i = 0; i < subscribeInfoLen; ++i)
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

	bool CanService::ReceiveMessage(CanMessage& outMessage)
	{
		/* If we have canInterruptHandle, then simply receive from queue. */
		if (canInterruptHandle)
			return xQueueReceive(rxQueueHandle, (void*)&outMessage, 0) == pdPASS;

		return false;
	}

	void CanService::StopService()
	{
		/* Free interrupt. */
		if (canInterruptHandle)
		{
			esp_intr_free(canInterruptHandle);
			canInterruptHandle = nullptr;
			vQueueDelete(rxQueueHandle);
			rxQueueHandle = nullptr;

			/* Reset CAN device */
			volatile uint32_t* reg = (volatile uint32_t*)(REG_BASE);
			*reg = (*reg & ~0x17) | 0x01;

			/* Close CAN BUS interface. */
			CAN.end();

			/* Clear message list. */
			subscribedMessages.clear();
		}
	}

	CanService::~CanService()
	{
		StopService();
	}
}