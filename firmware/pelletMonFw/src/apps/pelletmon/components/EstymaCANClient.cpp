#include "EstymaCANCLient.h"
#include "BoilerStatusUpdater.h"
#include "../../../board.h"
#include <CAN.h>

#define ESTYMA_CAN_ROTATIONS 0x2D7
#define ESTYMA_CAN_TEMEPRATURES 0x2D6
#define ESTYMA_CAN_CWUTEMPS 0x3D6
#define ESTYMA_CAN_EXHAUST_TEMPS 0x4D6

#define ESTYMA_CAN_SPEED 125E3

#define REG_BASE                   0x3ff6b000
#define REG_RXERR                  0x0e
#define REG_TXERR                  0x0f
#define REG_IR                     0x03

namespace comps
{
	EstymaCANClient::EstymaCANClient()
	{
		CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
	}
	
	uint8_t EstymaCANClient::readCANReg(uint8_t address) const
	{
		volatile uint32_t* reg = (volatile uint32_t*)(REG_BASE + address * 4);
		return *reg;
	}

	unsigned int EstymaCANClient::getRxErrorCount() const
	{
		return (unsigned int)readCANReg(REG_RXERR);
	}

	unsigned int EstymaCANClient::getTxErrorCount() const
	{
		return (unsigned int)readCANReg(REG_TXERR);
	}

	void EstymaCANClient::bindCAN()
	{
		if (!isBound)
		{
			isBound = true;

			CAN.begin(ESTYMA_CAN_SPEED);
			
			/*
				[0X2D7, 0X2D6, 0X3D6, 0X4D6]
				Filter: 0x0D6
				Mask : 0x0FE
				ID : 0x0D6 will be received
				ID : 0x0D7 will be received
				ID : 0x1D6 will be received
				ID : 0x1D7 will be received
				ID : 0x2D6 will be received
				ID : 0x2D7 will be received
				ID : 0x3D6 will be received
				ID : 0x3D7 will be received
				ID : 0x4D6 will be received
				ID : 0x4D7 will be received
				ID : 0x5D6 will be received
				ID : 0x5D7 will be received
				ID : 0x6D6 will be received
				ID : 0x6D7 will be received
				ID : 0x7D6 will be received
				ID : 0x7D7 will be received
				16 ID's will be received
			*/
			CAN.filter(0x0D6, 0x0FE);

			/* Observe mode */
			CAN.observe();

			esp_intr_alloc(ETS_CAN_INTR_SOURCE, 0, EstymaCANClient::onInterruptWrapper, this, &eccCANInterruptHandle);
		}
	}

	void EstymaCANClient::onInterruptWrapper(void* eccPtr)
	{
		((EstymaCANClient*)eccPtr)->handleInterrupt();
	}

	void EstymaCANClient::unbindCAN()
	{
		if (isBound)
		{
			isBound = false;

			if (eccCANInterruptHandle)
			{
				esp_intr_free(eccCANInterruptHandle);
				eccCANInterruptHandle = NULL;
			}

			/* Reset CAN device */
			volatile uint32_t* reg = (volatile uint32_t*)(REG_BASE);
			*reg = (*reg & ~0x17) | 0x01;

			CAN.end();
		}
	}

	double EstymaCANClient::calculateTemperature(uint16_t x) const
	{
		double a = 1.445633283634090E+02;
		double b = -1.386947619796149E-01;
		double c = 9.837913934334235E-05;
		double d = -4.486094388070098E-08;
		double e = 1.076415627857750E-11;
		double f = -1.066640689001453E-15;

		return a + b * x + c * pow(x, 2) + d * pow(x, 3) + e * pow(x, 4) + f * pow(x, 5);
	}

	double EstymaCANClient::calculateExhaustTemperature(uint16_t x) const
	{
		double a = 7.371348541128694E+05;
		double b = -1.577562311085309E+03;
		double c = 1.348102462543938E+00;
		double d = -5.750965910950425E-04;
		double e = 1.224810606468546E-07;
		double f = -1.041666667023733E-11;

		return a + b * x + c * pow(x, 2) + d * pow(x, 3) + e * pow(x, 4) + f * pow(x, 5);
	}

	bool EstymaCANClient::init(ksf::ksComposable* owner)
	{
		boilerStatusUpdater_wp = owner->findComponent<BoilerStatusUpdater>();
		return true;
	}

	bool EstymaCANClient::loop()
	{
		return true;
	}

	void EstymaCANClient::handleInterrupt()
	{
		if (readCANReg(REG_IR) & 0x01) 
		{
			if (CAN.parsePacket() > 0 && !CAN.packetRtr())
			{
				if (auto bsu_sp = boilerStatusUpdater_wp.lock())
				{
					switch (CAN.packetId())
					{
						case ESTYMA_CAN_ROTATIONS:
						{
							unsigned short val;
							CAN.readBytes((uint8_t*)&val, 2);
							bsu_sp->rotations = val;
						}
						break;

						case ESTYMA_CAN_TEMEPRATURES:
						{
							uint16_t temps[4];
							CAN.readBytes((uint8_t*)&temps, sizeof(temps));

							bsu_sp->updateTemperature(TemperatureType::Boiler, calculateTemperature(temps[0]));
							bsu_sp->updateTemperature(TemperatureType::Boiler2, calculateTemperature(temps[1]));
						}
						break;

						case ESTYMA_CAN_CWUTEMPS:
						{
							uint16_t temps[4];
							CAN.readBytes((uint8_t*)&temps, sizeof(temps));
							bsu_sp->updateTemperature(TemperatureType::Cwu, calculateTemperature(temps[0]));
						}
						break;

						case ESTYMA_CAN_EXHAUST_TEMPS:
						{
							uint16_t temps[4];
							CAN.readBytes((uint8_t*)&temps, sizeof(temps));
							bsu_sp->updateTemperature(TemperatureType::Exhaust, calculateExhaustTemperature(temps[3]));
						}
						break;
					}
				}
			}
		}
	}

	EstymaCANClient::~EstymaCANClient()
	{
		unbindCAN();
	}
}