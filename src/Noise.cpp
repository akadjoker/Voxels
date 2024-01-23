#include "Noise.hpp"

Noise::Noise()
{
	srand(time(NULL));

	p = new int[512];

	for (int i = 0; i < 256; ++i)
		p[i] = i;
	int		j = 0;
	int		tmp;
	for (int i = 0; i < 256; i++)
	{
		j = rand() & 255;

		tmp = p[i];
		p[i] = p[j];
		p[j] = tmp;
	}
	for (int i = 256; i < 512; i++)
	{
		p[i] = p[i - 256];
	}
}

Noise::Noise(unsigned int seed)
{
	this->seed = seed;
	srand(seed);

	p = new int[512];

	for (int i = 0; i < 256; ++i)
		p[i] = i;
	int		j = 0;
	int		tmp;
	for (int i = 0; i < 256; i++)
	{
		j = rand() & 255;

		tmp = p[i];
		p[i] = p[j];
		p[j] = tmp;
	}
	for (int i = 256; i < 512; i++)
	{
		p[i] = p[i - 256];
	}
}

Noise::~Noise()
{
	delete p;
}

static float	lerp(float t, float a, float b)
{
	return a + t * (b - a);
}

static float	grad(int hash, float x, float y, float z)
{
	int		h = hash & 15;
	float	u = (h < 8) ? x : y;
	float	v;

	if (h < 4)
		v = y;
	else if (h == 12 || h == 14)
		v = x;
	else
		v = z;
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

static float	fade(float t)
{
	return t*t*t*(t*(t*6-15)+10);
}

float 	Noise::noise(float x, float y, float z)
{
	int	unit_x = (int)floor(x) & 255;
	int	unit_y = (int)floor(y) & 255;
	int	unit_z = (int)floor(z) & 255;
	float	sub_x = x - floor(x);
	float	sub_y = y - floor(y);
	float	sub_z = z - floor(z);
	float	u = fade(sub_x);
	float	v = fade(sub_y);
	float	w = fade(sub_z);
	int		a = p[unit_x] + unit_y;
	int		aa = p[a] + unit_z;
	int		ab = p[a + 1] + unit_z;
	int		b = p[unit_x + 1] + unit_y;
	int		ba = p[b] + unit_z;
	int		bb = p[b + 1] + unit_z;

	return	lerp(w, lerp(v, lerp(u,
                    grad(p[aa], sub_x, sub_y, sub_z),
                    grad(p[ba], sub_x - 1, sub_y, sub_z)),
					lerp(u,
					grad(p[ab], sub_x, sub_y - 1, sub_z),
                    grad(p[bb], sub_x - 1, sub_y - 1, sub_z))),
					lerp(v,
					lerp(u,
					grad(p[aa + 1], sub_x, sub_y, sub_z - 1),
					grad(p[ba + 1], sub_x - 1, sub_y, sub_z - 1)),
					lerp(u,
					grad(p[ab + 1], sub_x, sub_y - 1, sub_z - 1),
					grad(p[bb + 1], sub_x - 1, sub_y - 1, sub_z - 1))));
}

float	Noise::OctavePerlin(float x, float y, float z, int octaves, float persistence)
{
    float total = 0;
    float frequency = 1;
    float amplitude = 1;
    float maxValue = 0;
    for(int i=0;i<octaves;i++) {
        total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
        
        maxValue += amplitude;
        
        amplitude *= persistence;
        frequency *= 2;
    }
    
    return total/maxValue;
}