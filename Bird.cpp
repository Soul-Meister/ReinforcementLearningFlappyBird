//
// Created by edward on 10/8/25.
//

#include "Bird.h"
#include <iostream>

using namespace std;

void Bird::update() {
    y_vel -= gravity;
    y += y_vel;
    cout << "Y_Pos: " << y << "   Y_Vel: " << y_vel << endl;
}

Bird::Bird() {
    double gravity = 5;
    double x = 220;
    double y = 390;
    double width = 20;
    double height = 20;
    double y_vel = 0;
}