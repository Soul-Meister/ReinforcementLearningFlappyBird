//
// Created by edward on 10/8/25.
//

#include "Wall.h"
#include "../main/config.h"

#include <chrono>
#include <random>

Wall::Wall() {


    const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    // Mersenne Twister PRNG
    std::mt19937 rng(seed);

    // Example distributions
    std::uniform_int_distribution<int> distInt(1, window_height);    // random int [1,<height of window>]


    width = 50;
    height = window_height;

    x_pos = window_width+width;//just outside the window
    y_pos = distInt(rng);

    gap_size = gap_width_config;

}