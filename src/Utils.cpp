#include "Utils.h"

Random::Random(const unsigned int seed)
    : randomEngine(seed) {
}

int Random::Int(const int min, const int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(randomEngine);
}

std::size_t Random::WeightedIndex(const std::vector<int>& weights) {
    std::discrete_distribution<std::size_t> distribution(
        weights.begin(), weights.end()
    );
    return distribution(randomEngine);
}