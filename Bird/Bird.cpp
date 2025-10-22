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
    //cout << "\rY_Pos: " << y << "   Y_Vel: " << y_vel << "      " << std::flush;

}

Bird::Bird() {
    gravity = -0.13   ;
    x = 200;
    y = window_height_config/2;
    width = 50;
    height = 50;
    y_vel = 0;
    score = 0;
}
