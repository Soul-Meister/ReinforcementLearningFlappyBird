//
// Created by edward on 10/8/25.
//

#include "Bird.h"
#include <iostream>

#include "../main/config.h"

using namespace std;

void Bird::update() {
    y_vel -= gravity;
    y += y_vel;

    if (y_vel > 10) {
        y_vel = 10;
    }


    //cout << "\rY_Pos: " << y << "   Y_Vel: " << y_vel << "      " << std::flush;

}

Bird::Bird() {
    gravity = -0.13f;
    x = 200.0f;
    y = window_height_config/2.0f;
    width = 50.0f;
    height = 50.0f;
    y_vel = 0.0f;
    score = 0;
    unchecked_score = false;
}
