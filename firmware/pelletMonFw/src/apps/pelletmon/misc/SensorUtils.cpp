#include "SensorUtils.h"
#include <math.h>

namespace misc
{
	double SensorUtils::sensorData[EstymaTempSensorType::ST_MAX][6] =
	{
		/* CT2A polynominal formula values. */
		{1.445633283634090E+02, -1.386947619796149E-01, 9.837913934334235E-05, -4.486094388070098E-08, 1.076415627857750E-11, -1.066640689001453E-15},
		
		/* CT3A polynominal formula values. */
		{7.371348541128694E+05, -1.577562311085309E+03, 1.348102462543938E+00, -5.750965910950425E-04, 1.224810606468546E-07, -1.041666667023733E-11}
	};

	float SensorUtils::convertTemp(uint16_t x, EstymaTempSensorType::TYPE t)
	{
		/* Check bounds. */
		if ((t < 0) || (t >= EstymaTempSensorType::ST_MAX))
			return 0;

		/* Return calculated polynominal formula. */
		return float(
			sensorData[t][0] + sensorData[t][1] * x + sensorData[t][2] * pow(x, 2) + 
			sensorData[t][3] * pow(x, 3) + sensorData[t][4] * pow(x, 4) + sensorData[t][5] * pow(x, 5)
		);
	}
}