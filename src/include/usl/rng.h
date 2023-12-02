#ifndef NOCTIS_RNG_H
#define NOCTIS_RNG_H

#include <iostream>
#include <random>
#include <ctime>

class RNG {
public:
    static RNG& getInstance();
    double random(double from, double to);

private:
    RNG();
    std::mt19937 generator; // Mersenne Twister engine
};

RNG& RNG::getInstance() {
    static RNG instance;
    return instance;
}

RNG::RNG() {
    std::seed_seq seed{ static_cast<unsigned>(std::time(0)) };
    generator.seed(seed);
}

double RNG::random(double from, double to) {
    std::uniform_real_distribution<double> distribution(from, to);
    return distribution(generator);
}

#endif