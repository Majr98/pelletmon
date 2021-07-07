#pragma once
namespace misc
{
	namespace EstymaTempSensorType
	{
		enum TYPE
		{
			CT2A,
			CT3A
		};
	}

	class SensorUtils
	{
		private:
			static double sensorData[2][6];

		public:
			static float convertTemp(uint16_t adcValue, EstymaTempSensorType::TYPE sensorType);
	};
}

