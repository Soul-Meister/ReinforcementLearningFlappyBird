//
// Created by edward on 10/6/25.
//
#pragma once

#include <vector>
#include <SDL2/SDL.h>

#include "../Wall/Wall.h"
#include "../Bird/Bird.h"
#include "../Agent/Policies/EpsilonGreedyPolicy.h"
#include "../Agent/Network/Network.h"
#include "../Agent/ReplayBuffer/ReplayBuffer.h"
using namespace std;

#ifndef FLAPPYBIRD_GAME_H
#define FLAPPYBIRD_GAME_H
class GameEnv {
public:
    GameEnv(int);
    int action;
    int flaps;
    int episodes;
    int iterations;
    int max_score;
    int wall_delay_frames;
    int last_wall_spawn_frames;
    Bird bird;
    vector<Wall> walls;

    std::mt19937 rng;
    std::uniform_real_distribution<float> real_dist;
    std::bernoulli_distribution flap_dist;

    float normalize(float value, float min, float max);


    void update(bool, EpsilonGreedyPolicy, Network*, ReplayBuffer*, bool);//network update version
    void update(SDL_Renderer*, bool, bool);//human player version
    void update(bool clicked, Network *network, bool render_);


    bool check_collision(Bird*, vector<Wall> *);
    //void reset();
    vector<float> get_game_state();
    double get_reward(const vector<float>& state, Bird* bird, bool done);
};

void render(SDL_Renderer*, Bird*, vector<Wall> *);
void render(SDL_Renderer*, vector<GameEnv>*);
void render_mini_window(SDL_Renderer*, GameEnv*, int, int, int, int);



#endif //FLAPPYBIRD_GAME_H