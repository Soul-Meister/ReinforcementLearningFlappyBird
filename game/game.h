//
// Created by edward on 10/6/25.
//
#pragma once

#include <vector>
#include <SDL2/SDL.h>

#include "../Wall/Wall.h"
#include "../Bird/Bird.h"

using namespace std;

#ifndef FLAPPYBIRD_GAME_H
#define FLAPPYBIRD_GAME_H

void update(SDL_Renderer*, Bird*, bool, vector<Wall> *);
void render(SDL_Renderer*, Bird*, vector<Wall> *);
bool check_collision(Bird*, vector<Wall> *);

#endif //FLAPPYBIRD_GAME_H