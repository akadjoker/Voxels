#pragma once

#include <cstdlib>
#include <ctime>
#include <cmath>

class Noise
{
	public:
		Noise();
		Noise(unsigned int seed);
		~Noise();
		unsigned int	seed;
		float	noise(float x, float y, float z);
		float	OctavePerlin(float x, float y, float z, int octaves, float persistence);
	private:
		int				*p;
};