#pragma once
#include <random> //need for random number generation

extern std::mt19937 rng; //keep one global instance of your random device (per thread)

class DieRoller
{
public:
	void Seed();

	float RollDN(float sides);

	DieRoller()
	{
		Seed();
	}
};