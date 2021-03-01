#include "src/apps/PelletMon/PelletMon.h"
#include "src/apps/config/PelletMOnConfig.h"

#ifdef ESP32
#include "SPIFFS.h"
#else
#include "FS.h"
#include "user_interface.h"
#endif

// the setup function runs once when you press reset or power the board
void setup()
{
	#ifdef ESP32
		uint32_t chipId = (uint32_t)ESP.getEfuseMac();
	#else
		uint32_t chipId = ESP.getChipId();
	#endif

	auto chipIdBytes = (unsigned char*)&chipId;
	uint8_t mac[6]{ 0xfa, 0xf1, chipIdBytes[0], chipIdBytes[1], chipIdBytes[2], chipIdBytes[3]};

	#ifdef ESP32
		esp_base_mac_addr_set(mac);
	#else
		wifi_set_macaddr(STATION_IF, mac);
	#endif

	SPIFFS.begin(true);
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	RUN_APP_BLOCKING_LOOPED(PelletMon)
	RUN_APP_BLOCKING_LOOPED(PelletMonConfig)
}