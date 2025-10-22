//
// Created by edward on 10/8/25.
//
#pragma once
#ifndef FLAPPYBIRD_CONFIG_H
#define FLAPPYBIRD_CONFIG_H


#include <vector>


extern int window_height_config;
extern int window_width_config;
extern int target_fps_config;
extern int wall_speed_config;
extern int gap_width_config;

//Replay buffer parameters
extern int replay_buffer_size_config;
extern int replay_buffer_sample_size_config;4\

//Learning rate
extern double learning_rate_config;

//network config
extern std::vector<int> network_config;

//selected config

#endif //FLAPPYBIRD_CONFIG_H