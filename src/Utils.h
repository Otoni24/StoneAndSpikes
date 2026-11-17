#pragma once

#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

class Random {
public:
    Random(unsigned int seed);
    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;

    int Int(int min, int max);
    std::size_t WeightedIndex(const std::vector<int>& weights);

    template <typename Range>
    void Shuffle(Range& range) {
        std::ranges::shuffle(range, randomEngine);
    }

private:
    std::mt19937 randomEngine;
};
