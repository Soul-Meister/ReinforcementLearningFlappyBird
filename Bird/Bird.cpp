//
// Created by edward on 10/8/25.
//

#include "Bird.h"
#include <iostream>

using namespace std;

void Bird::update() {
    y_vel -= gravity;
    y += y_vel;
    cout << "\rY_Pos: " << y << "   Y_Vel: " << y_vel << "      " << std::flush;

}

Bird::Bird() {
    gravity = -0.13   ;
    x = 200;
    y = 220;
    width = 50;
    height = 50;
    y_vel = 0;
}
