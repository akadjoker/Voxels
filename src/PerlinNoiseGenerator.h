#pragma once
#include <cmath>
#include <cstdlib>
#include <ctime>

class PerlinNoiseGenerator
{
public:
    static float AMPLITUDE;
    static int OCTAVES;
    static float ROUGHNESS;

    PerlinNoiseGenerator()
    {
        seed = 0;
        xOffset = 0;
        zOffset = 0;
    }

    PerlinNoiseGenerator(int gridX, int gridZ, int vertexCount, int seed)
    {
        this->seed = seed;
        xOffset = gridX * (vertexCount - 1);
        zOffset = gridZ * (vertexCount - 1);
    }

    float generateHeight(int x, int z)
    {
        x = x < 0 ? -x : x;
        z = z < 0 ? -z : z;

        float total = 0;
        float d = pow(2, OCTAVES - 1);
        for (int i = 0; i < OCTAVES; i++)
        {
            float freq = pow(2, i) / d;
            float amp = pow(ROUGHNESS, i) * AMPLITUDE;
            total += getInterpolatedNoise((x + xOffset) * freq, (z + zOffset) * freq) * amp;
        }

        return static_cast<float>(static_cast<int>(total));
    }

private:
    int seed;
    int xOffset;
    int zOffset;

    float getInterpolatedNoise(float x, float z)
    {
        int intX = static_cast<int>(x);
        int intZ = static_cast<int>(z);
        float fracX = x - intX;
        float fracZ = z - intZ;

        float v1 = getSmoothNoise(intX, intZ);
        float v2 = getSmoothNoise(intX + 1, intZ);
        float v3 = getSmoothNoise(intX, intZ + 1);
        float v4 = getSmoothNoise(intX + 1, intZ + 1);
        float i1 = interpolate(v1, v2, fracX);
        float i2 = interpolate(v3, v4, fracX);
        return interpolate(i1, i2, fracZ);
    }

    float interpolate(float a, float b, float blend)
    {
        double theta = blend * M_PI;
        float f = static_cast<float>((1.0 - cos(theta)) * 0.5);
        return a * (1.0 - f) + b * f;
    }

    float getSmoothNoise(int x, int z)
    {
        float corners = (getNoise(x - 1, z - 1) + getNoise(x + 1, z - 1) + getNoise(x - 1, z + 1) + getNoise(x + 1, z + 1)) / 16.0;
        float sides = (getNoise(x - 1, z) + getNoise(x + 1, z) + getNoise(x, z - 1) + getNoise(x, z + 1)) / 8.0;
        float center = getNoise(x, z) / 4.0;
        return corners + sides + center;
    }

    float getNoise(int x, int z)
    {
        srand(x * 49632 + z * 325176 + seed);
        return static_cast<float>(rand()) / RAND_MAX * 2.0 - 1.0;
    }
};


float PerlinNoiseGenerator::AMPLITUDE = 80.0;
int PerlinNoiseGenerator::OCTAVES = 7;
float PerlinNoiseGenerator::ROUGHNESS = 0.3;
