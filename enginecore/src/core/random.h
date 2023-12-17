#pragma once

#include <random>

namespace ec {

    template<typename T>
    T randomInt() {

        std::random_device r;

        std::default_random_engine e(r());
        std::uniform_int_distribution<T> uniform_dist;

        return uniform_dist(e);

    }

}