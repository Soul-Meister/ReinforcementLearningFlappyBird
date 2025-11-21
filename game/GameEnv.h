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
class GameEnv {
public:
    GameEnv();
    int flaps;
    int episodes;
    int iterations;
    int max_score;
    int wall_delay_frames;
    int last_wall_spawn_frames;
    Bird bird;
    vector<Wall> walls;
    float normalize(float value, float min, float max);
    void update(SDL_Renderer*, bool, bool render_);
    void render(SDL_Renderer*, Bird*, vector<Wall> *);
    bool check_collision(Bird*, vector<Wall> *);
    void reset();
    vector<float> get_game_state(Bird* pbird, vector<Wall>* pwalls);
    double get_reward(const vector<float>& state, Bird* bird, bool done);
};



#endif //FLAPPYBIRD_GAME_H