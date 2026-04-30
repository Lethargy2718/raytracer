#include "random.hpp"
#include <random>

static std::mt19937 generator(std::random_device{}());
static std::uniform_real_distribution<double> distribution(0.0, 1.0);

double random_double() {
    return distribution(generator);
}

double random_double(double min, double max) {
    return min + (max - min) * random_double();
}