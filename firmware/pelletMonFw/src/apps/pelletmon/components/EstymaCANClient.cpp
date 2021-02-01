#include "EstymaCANCLient.h"
#include "../../../board.h"
#include <CAN.h>

namespace comps
{
	EstymaCANCLient::EstymaCANCLient()
	{
		CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
	}

	bool EstymaCANCLient::init(ksf::ksComposable* owner)
	{
		CAN.begin(125E3);
		return true;
	}

	bool EstymaCANCLient::loop()
	{
		int packetSize = CAN.parsePacket();

		if (packetSize)
		{
			//todo
		}

		return true;
	}

	EstymaCANCLient::~EstymaCANCLient()
	{
		CAN.end();
	}
}