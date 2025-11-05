#include <algorithm>
#include <iostream>
#include <SDL2/SDL.h>
#include <vector>


#include "../Agent/Network/Network.h"
#include "../game/game.h"
#include "../Bird/Bird.h"
#include "../Wall/Wall.h"
#include "../Agent/Policies/EpsilonGreedyPolicy.h"
#include "config.h"

#include <random>
#include <chrono>


using namespace std;

int game_mode;

//game globals
int window_height_config;
int window_width_config;
int target_fps_config;
int gap_width_config;
int wall_speed_config;

//agent globals
int replay_buffer_size_config;
int replay_buffer_sample_size_config;
double learning_rate_config;
float bias_config;

//policy
float min_epsilon_config;
float policy_decay_config;

//discount factor
float gamma_config;

//network
vector<int> network_config;

//activation function
float alpha_config;

struct Transition {
    std::vector<float> state;
    int action;
    double reward;
    std::vector<float> next_state;
    bool done;
};





int main() {

    //0 for human player
    //1 for train model on these configurations - HEADED
    //2 for train model on these configurations - HEADLESS
    //3 for loading a model form json, then running
    game_mode = 1;

    //declare game globals
    window_height_config = 1200;
    window_width_config = 1600;
    target_fps_config = 120;
    gap_width_config = 110;
    wall_speed_config = 1;

    //declare agent globals
    replay_buffer_size_config = 100000;
    replay_buffer_sample_size_config = 128; //64-128
    learning_rate_config = 0.001;
    bias_config = 0.01f;

    //policy
    min_epsilon_config = 0.001;
    policy_decay_config = 0.001;

    //discount factor
    gamma_config = 0.99f;

    //activation function
    alpha_config = 0.01f;

    //network variables -- {inputs, hidden, hidden ... hidden, output}
    network_config = {20, 20, 20, 2};

    //wall creation
    int wall_delay_frames = 360;
    int last_wall_spawn_frames = wall_delay_frames;//will go to 0 then increment up; this is to immediately spawn one


    vector<Wall> walls;
    Bird bird = Bird();
    vector<float> game_state(7); //bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y, next_next_wall_y
    vector<Transition> replay_buffer;

    Uint32 frameDelay_ms = 1000/target_fps_config;


    bool running = true;
    SDL_Event event;
    bool speed_toggle  = false;
    bool has_clicked = true;
    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width_config, window_height_config, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // create RNG engine (seeded by time)
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> real_dist(0.0f, 1.0f);
    std::uniform_int_distribution<int> action_dist(0, 1);






    switch (game_mode) {
        case 0: {//human player
            if (SDL_Init(SDL_INIT_VIDEO)) {
                cout << "SDL_Init Error: " << SDL_GetError() << endl;//the now very not epic window failed to open
            }

            if (!window) {
                cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
                SDL_Quit();
                return 1;
            }




                    while (running) {//main update loop
            Uint32 startTime = SDL_GetTicks();

            if (has_clicked) {
                last_wall_spawn_frames++;
            }


            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {//quit
                    running = false;
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {//check if jumps
                    has_clicked = true;
                    bird.y_vel = -5;//set bird velocity
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {//reset function
                    has_clicked = false;
                    walls.clear();
                    bird.x =200;
                    bird.y = window_height_config/2;
                    bird.y_vel = 0;
                    last_wall_spawn_frames = wall_delay_frames;
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s) {//speed toggle
                    speed_toggle = !speed_toggle;
                }
            }


            Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame
            if (last_wall_spawn_frames >= wall_delay_frames) {
                last_wall_spawn_frames = 0;
                walls.emplace_back(Wall());
            }

            if (!speed_toggle) {
                // Cap FPS
                if (frameTime < frameDelay_ms) {
                    SDL_Delay(frameDelay_ms - frameTime);
                }
            }


           if (!check_collision(&bird, &walls)) {//check collisions, if not
               update(renderer, &bird, has_clicked, &walls);//main update function
            }

        }





        }

            break;
        case 1: {
            //train model headed

            //Model Reqs init
            Network network = Network();
            auto policy = EpsilonGreedyPolicy(policy_decay_config, min_epsilon_config);
            network.init();


            if (SDL_Init(SDL_INIT_VIDEO)) {
                cout << "SDL_Init Error: " << SDL_GetError() << endl; //the now very not epic window failed to open
            }

            if (!window) {
                cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
                SDL_Quit();
                return 1;
            }
            while (running) {
                //main update loop
                Uint32 startTime = SDL_GetTicks();

                if (has_clicked) {
                    last_wall_spawn_frames++;
                }


                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        running = false;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                        //check if space is pressed
                        has_clicked = true;
                        bird.y_vel = -5; //set bird velocity
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                        //reset function
                        walls.clear();
                        bird.x = 200;
                        bird.y = window_height_config / 2;
                        bird.y_vel = 0;
                        last_wall_spawn_frames = wall_delay_frames;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s) {
                        //speed toggle
                        speed_toggle = !speed_toggle;
                    }
                }


                Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame
                if (last_wall_spawn_frames >= wall_delay_frames) {
                    last_wall_spawn_frames = 0;
                    walls.emplace_back(Wall());
                }

                if (!speed_toggle) {
                    // Cap FPS
                    if (frameTime < frameDelay_ms) {
                        SDL_Delay(frameDelay_ms - frameTime);
                    }
                }



                //initial pass -- prepare data
                auto initial_game_state = game_state;
                auto q_values = network.forward(game_state); //assign result to memory to avoid multiple calls

                // epsilon-greedy action selection
                int action = (real_dist(rng) < policy.getEpsilon())
                               ? action_dist(rng)                             // explore
                               : (q_values[0] > q_values[1] ? 0 : 1);            //exploit

                if (action == 1) {
                    bird.y_vel = -5;
                }

                //update the game with model choice
                update(renderer, &bird, has_clicked, &walls);

                //get post model choice data
                game_state = get_game_state(&bird, &walls); //retrieve game state
                double reward = get_reward(game_state, &bird, check_collision(&bird, &walls));
                cout << "Reward: " << reward  <<  "  Epsilon: " << policy.getEpsilon() << endl;


                //save data to a replay buffer
                replay_buffer.emplace_back(Transition{
                    initial_game_state,
                    action,
                    reward,
                    game_state,
                    check_collision(&bird, &walls) //enter the "end of episode" function
                });

                // Training phase (sample batch from replay buffer)
                if (replay_buffer.size() > replay_buffer_sample_size_config) {
                    // sample random batch of transitions
                    for (int n = 0; n < replay_buffer_sample_size_config; ++n) {

                        std::uniform_int_distribution<size_t> idx_dist(0, replay_buffer.size() - 1);
                        const Transition& t = replay_buffer[idx_dist(rng)];

                        // forward pass: Q(s)
                        std::vector<float> q_current = network.forward(t.state);

                        // forward pass: Q(s')
                        std::vector<float> q_next = network.forward(t.next_state);
                        float max_next = *max_element(q_next.begin(), q_next.end());

                        // build target vector (same size as output)
                        std::vector<float> target = q_current;
                        target[t.action] = t.reward + (t.done ? 0.0f : gamma_config * max_next);

                        // backpropagate loss (Q-learning)
                        network.backward(target);
                    }

                    // Decay epsilon after each training step
                    policy.decay();
                }

                //make sure replay buffer remains within buffer size limit
                if ((int) replay_buffer.size() > replay_buffer_size_config) {
                    //if buffer exceeds max size, delete oldest entry
                    replay_buffer.erase(replay_buffer.begin());
                }

                //reset the bird if it has crashed
                if (check_collision(&bird, &walls)) {
                    //reset functionss
                    walls.clear();
                    bird.x = 200;
                    bird.y = window_height_config / 2;
                    bird.y_vel = 0;
                    last_wall_spawn_frames = wall_delay_frames;
                }



            }
        }

            break;
        case 2:
            //train model headless
            break;
        case 3:
            //load model
            break;

        default:
            break;
    }


    //ayo, clean it
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
