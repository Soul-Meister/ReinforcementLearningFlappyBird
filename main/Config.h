//
// Created by edward on 10/8/25.
//
#pragma once
#ifndef FLAPPYBIRD_CONFIG_H
#define FLAPPYBIRD_CONFIG_H


#include <vector>


class Config {
    public:
    Config();
};

extern int game_mode;//used to determine type of gameplay.
//0 for human player
//1 for train model on these configurations - Single threaded
//2 for train model on these configurations - Multi threaded
//3 for loading a model form json, then running an env making inferences form that model

extern int threads; //if gamemode 2 is selected, this chooses how many threads to run; LIMITED BY CPU CORE COUNT; one thread per core.
//Runs threads-1 environments; 1 is ALWAYS allocated for training loop.
//more threads == faster training


extern int window_height_config;//if rendered, how large the height of the window will be
extern int window_width_config;//how wide it will be
extern int target_fps_config;//I would hope this is obvious
extern int wall_speed_config;//also obvious
extern int gap_width_config;//the size of the gaps in the walls

//Replay buffer parameters -- replay buffer stores a history of past experiences that will be randomly sampled from. Helps get a diverse training background; useful to avoid overfitting if large enough
extern int replay_buffer_size_config;//how large the replay buffer is
extern int replay_buffer_sample_size_config;//batch size for training loop

//Learning rate
extern double learning_rate_config;//how quickly nuerons learn/gain confidence. Low values work good, usually 0.001-0.0001. Higher values lead to exploding gradients and unstable learning
extern float bias_config; //starting bias; this is for giggles. Standard is for neurons to always init bias to 0

//network config
extern std::vector<int> network_config; //determines the size of the network, NOT COUNTING input layer.

//Policy -- NOT ALL WILL BE USED< DEPENDING ON SELECTED POLICY
extern float min_epsilon_config; // Minimum value for exploration in Epsilon policy -- ensures that the model will always be trying enw things, by ensuring the model ALWAYS epxlores, by at least this amount
extern float policy_decay_config; //how quickly the model shifts from exploring to exploiting -- larger values will yeild a 'finished model' sooner, but will shirnk its time to explore, so the model won't perform as well.

//discount rate
extern float gamma_config; //how much rewards are discounted; usually 0.95 - 0.99

//Activation Function Config -- NOT ALL WILL BE USED, USAGE BASED ON SELECTED POLICY
extern float alpha_config;//for Leaky ReLU; determines the value for negative activations in a linear manner -- prevent dead neurons

//global variable for render
extern bool render_config;




#endif //FLAPPYBIRD_CONFIG_H