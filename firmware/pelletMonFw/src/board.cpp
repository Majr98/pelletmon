#include "board.h"
#include <ESP32CAN.h>

/* Static CAN_cfg struct. */
CAN_device_t CAN_cfg;

/* Method that configures ESP32 SJA1000 module using params defined in board.h. */
void init_can_config()
{
	CAN_cfg.tx_pin_id = (gpio_num_t)CAN_TX_PIN;
	CAN_cfg.rx_pin_id = (gpio_num_t)CAN_RX_PIN;
	CAN_cfg.rx_queue = nullptr;

	CAN_cfg.speed = CAN_SPEED;
}

/* Method that configures ESP32 SJA1000 filter based on ID and mask. */
void init_can_filter(int id, int mask)
{
	id &= 0x7ff;
	mask = ~(mask & 0x7ff);

	CAN_filter_t p_filter;

	p_filter.FM = Single_Mode;
	p_filter.ACR0 = id >> 3;
	p_filter.ACR1 = id << 5;
	p_filter.ACR2 = 0;
	p_filter.ACR3 = 0;

	p_filter.AMR0 = mask >> 3;
	p_filter.AMR1 = (mask << 5) | 0x1f;
	p_filter.AMR2 = 0xff;
	p_filter.AMR3 = 0xff;

	ESP32Can.CANConfigFilter(&p_filter);
}

/* Method that actually runs ESP32 SJA1000 module. */
bool start_can_module(unsigned int msgQueueSize)
{
	if (!is_can_module_running())
	{
		CAN_cfg.rx_queue = xQueueCreate(msgQueueSize, sizeof(CAN_frame_t));

		if (CAN_cfg.rx_queue != nullptr)
		{
			ESP32Can.CANInit();
			return true;
		}
	}

	return false;
}

/* Method that stops ESP32 SJA1000 module. */
bool stop_can_module()
{
	if (is_can_module_running())
	{
		ESP32Can.CANStop();
		vQueueDelete(CAN_cfg.rx_queue);

		CAN_cfg.rx_queue = nullptr;
		return true;
	}

	return false;
}

/* Method that checks if ESP32 SJA1000 module is operating. */
bool is_can_module_running()
{
	return CAN_cfg.rx_queue != nullptr;
}

/* Method that writes CAN frame to ESP32 SJA1000 module. */
bool can_write_frame(const CAN_frame_t& frame)
{
	return ESP32Can.CANWriteFrame(&frame) != -1;
}