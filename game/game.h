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

void update(SDL_Renderer*, Bird*, bool, vector<Wall> *, bool render_);
void render(SDL_Renderer*, Bird*, vector<Wall> *);
bool check_collision(Bird*, vector<Wall> *);
void reset();
vector<float> get_game_state(Bird* pbird, vector<Wall>* pwalls);
double get_reward(const vector<float>& state, Bird* bird, bool done);

#endif //FLAPPYBIRD_GAME_H