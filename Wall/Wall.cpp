//
// Created by edward on 10/8/25.
//

#include "Wall.h"
#include "../main/Config.h"

#include <chrono>
#include <random>

Wall::Wall() {


    const unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();//epic number gen

    // Mersenne Twister
    std::mt19937 rng(seed);

    std::uniform_real_distribution<float> distInt(gap_width_config +20, window_height_config-gap_width_config + 20);    // random int [1,<height of window>]

    width = 50;
    height = window_height_config;

    x_pos = static_cast<float>(window_width_config+width);//just outside the window
    y_pos = distInt(rng);

    gap_size = gap_width_config;

    is_scored = false;

}