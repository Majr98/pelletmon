#pragma once
#include <CAN_config.h>
#include <CAN.h>
///////////////////////////////////////////////////////////////////////////////////////////

#define STATUS_LED_PIN 22

#define CAN_RX_PIN 4
#define CAN_TX_PIN 5
#define CAN_SPEED CAN_SPEED_125KBPS

#define HCSR05_ECHO_PIN 32
#define HCSR05_TRIG_PIN 33

#define MQTT_STATUS_UPDATE_INTERVAL 15000

///////////////////////////////////////////////////////////////////////////////////////////

/* Method that configures ESP32 SJA1000 module using params defined in board.h. */
extern void init_can_config();

/* Method that configures ESP32 SJA1000 filter based on ID and mask. */
extern void init_can_filter(int id, int mask = 0x7ff);

/* Method that actually runs ESP32 SJA1000 module. */
extern bool start_can_module(unsigned int msgQueueSize = 10);

/* Method that stops ESP32 SJA1000 module. */
extern bool stop_can_module();

/* Method that checks if ESP32 SJA1000 module is operating. */
extern bool is_can_module_running();

/* Method that writes CAN frame to ESP32 SJA1000 module. */
extern bool can_write_frame(const CAN_frame_t& frame);

/* Method that reads CAN frame from ESP32 SJA1000 module. */
extern bool can_get_next_frame(const CAN_frame_t& frame);

///////////////////////////////////////////////////////////////////////////////////////////