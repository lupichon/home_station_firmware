#include "driver_BH1750.hpp"

BH1750Sensor::BH1750Sensor() : Sensor("BH1750", "lux") 
{

}

bool BH1750Sensor::begin()
{
	return sensor.begin();
}

void BH1750Sensor::read(Measurement& m)
{
	m.luminosity = sensor.readLightLevel();
}

float BH1750Sensor::measuredValue(const Measurement& m) const
{
    return m.luminosity;
}