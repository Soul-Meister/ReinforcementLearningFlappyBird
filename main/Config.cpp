#include <vector>
#include "Config.h"


int game_mode;
int load_model_num;
int threads_config;
int border_thickness_config;
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
    //1 for train model on these configurations
    //2 for load model given the defined number; one env
    //3 for load model given the defined number; multiple env
    //4 for load model given the defined number; continue training that model using these configs;
    game_mode = 4;
    load_model_num = 3;


    //amount of threads to run; will run threads-1 envs, one reserved for training thread
    //threads_config = 8;
    threads_config = std::thread::hardware_concurrency(); //uses as many threads as your computer has cores; can be set to a chosen int
    border_thickness_config = 5;

    //declare game globals
    //-------------SET TO YOUR MONITOR RESOLUTION--------------------------------------------------
    window_width_config = 1920;
    window_height_config = 1080;
    //---------------------------------------------------------------------------------------------
    target_fps_config = 240;
    gap_width_config = 200;
    wall_speed_config = 1;

    //declare agent globals
    replay_buffer_size_config = 100000;
    replay_buffer_sample_size_config = 64; //64-128
    learning_rate_config = 0.00004;
    bias_config = 0.0f;

    //policy
    min_epsilon_config = 0.005;
    policy_decay_config = 0.00001;
    //discount factor
    gamma_config = 0.9994f;

    //activation function
    alpha_config = 0.01f;

    //network variables -- {hidden, hidden ... hidden, output} -- layer size
    network_config = {32, 64, 64, 32, 2};

    //render or not
    render_config = true;
}