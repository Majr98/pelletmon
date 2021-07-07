#pragma once
#include <inttypes.h>
namespace misc
{
	namespace EstymaTempSensorType
	{
		enum TYPE
		{
			CT2A,
			CT3A,
			ST_MAX
		};
	}

	class SensorUtils
	{
		private:
			static double sensorData[EstymaTempSensorType::ST_MAX][6];

		public:
			static float convertTemp(uint16_t adcValue, EstymaTempSensorType::TYPE sensorType);
	};
}

