#include <random>
#include <vector>

#include "data_interface.h"
#include "hnsw.h"

int main()
{
    int dim = 2;
    ann::euclidean_distance<float> dist(dim);
    ann::hnsw index(&dist, 10000, 4, 20);
    
    std::mt19937 rand_gen(2020);
    std::uniform_real_distribution<float> distri(0.0, 1.0);

    int n = 1000;
    std::vector<float> base(n * dim);
    for (auto & each : base) each = distri(rand_gen);
    for (int i = 0; i < n; ++i) {
        index.insert((char*)(&base[dim * i]));
    }

    return 0;
}

