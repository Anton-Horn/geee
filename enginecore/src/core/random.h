#pragma once

#include <chrono>
#include <random>

namespace ec {

    template<typename T>
    T randomInt() {

        std::random_device r;

        std::default_random_engine e(r());
        e.seed((uint32_t)std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<T> uniform_dist;

        return uniform_dist(e);

    }
    

}