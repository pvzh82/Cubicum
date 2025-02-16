#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

class PerlinNoise {
public:
    /**
     * Generate 3D Perlin noise value at given coordinates
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @return Noise value in range [-1, 1]
     */
    static double noise(double x, double y, double z);

    static int p[512];
    static int permutation[256];

private:
    static double fade(double t);
    static double lerp(double t, double a, double b);
    static double grad(int hash, double x, double y, double z);
};

#endif
