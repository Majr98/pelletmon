#include "src/apps/PelletMon/PelletMon.h"
#include "src/apps/config/PelletMOnConfig.h"

// the setup function runs once when you press reset or power the board
void setup()
{
	ksf::initKsfFramework();
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	RUN_APP_BLOCKING_LOOPED(PelletMon)
	RUN_APP_BLOCKING_LOOPED(PelletMonConfig)
}