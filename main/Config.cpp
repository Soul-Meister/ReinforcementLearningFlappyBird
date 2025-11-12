#include <vector>
#include "Config.h"

int game_mode;
int window_height_config;
int window_width_config;
int target_fps_config;
int wall_speed_config;
int gap_width_config;
int replay_buffer_size_config;
int replay_buffer_sample_size_config;
double learning_rate_config;
float bias_config;
std::vector<int> network_config;
float min_epsilon_config;
float policy_decay_config;
float gamma_config;
float alpha_config;
bool render_config;


Config::Config() {

    //0 for human player
    //1 for train model on these configurations - HEADED
    //2 for train model on these configurations - HEADLESS
    //3 for loading a model form json, then running
    game_mode = 1;

    //declare game globals
    window_height_config = 1200;
    window_width_config = 1600;
    target_fps_config = 120;
    gap_width_config = 200;
    wall_speed_config = 1;

    //declare agent globals
    replay_buffer_size_config = 100000;
    replay_buffer_sample_size_config = 32; //64-128
    learning_rate_config = 0.0001;
    bias_config = 0.0f;

    //policy
    min_epsilon_config = 0.000;
    policy_decay_config = 0.00001;

    //discount factor
    gamma_config = 0.985f;

    //activation function
    alpha_config = 0.01f;

    //network variables -- {hidden, hidden ... hidden, output} -- layer size
    network_config = {32, 32, 2};

    //render or not
    render_config = true;
}