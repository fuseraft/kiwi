#ifndef NOCTIS_RNG_H
#define NOCTIS_RNG_H

class RNG
{
public:
    static void seed();
    static double random(double from, double to);
};

void RNG::seed()
{
    srand((unsigned int)time(NULL));
}

double RNG::random(double from, double to)
{
    double r = (double)rand() / (double)RAND_MAX;
    double min = std::min(from, to);
    double max = std::max(from, to);
    return min + (r * (max - min));
}

#endif