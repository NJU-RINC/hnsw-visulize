#include <random>
#include <vector>
#include <fstream>

#include "data_interface.h"
#include "hnsw.h"

int main()
{
    int dim = 2;
    ann::euclidean_distance<float> dist(dim);
    ann::hnsw index(&dist, 10000, 4, 20);
    
    std::mt19937 rand_gen(1995);
    std::uniform_real_distribution<float> distri(0.0, 1.0);

    int n = 50;
    std::vector<float> base(n * dim);
    for (auto & each : base) each = distri(rand_gen);
    std::ofstream ofs("data");
    ofs.write((const char*)&base[0], base.size() * 4);
    ofs.close();
    for (int i = 0; i < n; ++i) {
        index.insert((char*)(&base[dim * i]));
    }

    std::vector<float> query(1 * dim);
    //for (auto& each : query) each = distri(rand_gen);
    for (auto& each : query) each = 0.1;
    index.search((const char*)(&query[0]), 1);

    return 0;
}

