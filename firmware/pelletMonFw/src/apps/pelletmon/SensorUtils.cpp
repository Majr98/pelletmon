#include "SensorUtils.h"

namespace misc
{
	SensorUtils::sensorData = {
		{1.445633283634090E+02, -1.386947619796149E-01, 9.837913934334235E-05, -4.486094388070098E-08, 1.076415627857750E-11, -1.066640689001453E-15},
		{1.445633283634090E+02, -1.386947619796149E-01, 9.837913934334235E-05, -4.486094388070098E-08, 1.076415627857750E-11, -1.066640689001453E-15}
	}

	float SensorUtils::convertTemp(uint16_t x, )
	{
		double arr[] = { 1.445633283634090E+02, -1.386947619796149E-01, 9.837913934334235E-05, -4.486094388070098E-08, 1.076415627857750E-11, -1.066640689001453E-15 }


		return float(a + b * x + c * pow(x, 2) + d * pow(x, 3) + e * pow(x, 4) + f * pow(x, 5));
	}

	float SensorUtils::calculateExhaustTemperature(uint16_t x)
	{
		double a = 7.371348541128694E+05;
		double b = -1.577562311085309E+03;
		double c = 1.348102462543938E+00;
		double d = -5.750965910950425E-04;
		double e = 1.224810606468546E-07;
		double f = -1.041666667023733E-11;

		return float(a + b * x + c * pow(x, 2) + d * pow(x, 3) + e * pow(x, 4) + f * pow(x, 5));
	}
}