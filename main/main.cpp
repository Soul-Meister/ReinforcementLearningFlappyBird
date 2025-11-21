#include <algorithm>
#include <iostream>
#include <SDL2/SDL.h>
#include <vector>


#include "../Agent/Network/Network.h"
#include "../Agent/ReplayBuffer/ReplayBuffer.h"
#include "../game/GameEnv.h"
#include "../Bird/Bird.h"
#include "../Wall/Wall.h"
#include "../Agent/Policies/EpsilonGreedyPolicy.h"
#include "Config.h"

#include <random>
#include <chrono>


using namespace std;


struct Transition {
    std::vector<float> state;
    int action;
    double reward;
    std::vector<float> next_state;
    bool done;
};


int main() {
    const auto config = Config(); //run config, set globals


    vector<Wall> walls;
    //Bird bird = Bird();
    vector<float> game_state(7);
    //bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y, next_next_wall_y

    Uint32 frameDelay_ms = 1000 / target_fps_config;


    bool running = true;
    SDL_Event event;
    bool speed_toggle = false;
    bool has_clicked = true;
    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width_config,
                                          window_height_config, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // create RNG engine (seeded by time)
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> real_dist(0.0f, 1.0f);
    std::uniform_int_distribution<int> action_dist(0, 75);
    //1 in 75 chance for the epsilon choice to actually be jump; makes it better for learning rates


    switch (game_mode) {
        case 0: {
            //human player
            GameEnv game = GameEnv();
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
                    game.last_wall_spawn_frames++;
                }


                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        //quit
                        running = false;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                        //check if jumps
                        has_clicked = true;
                        game.bird.y_vel = -5; //set bird velocity
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                        //reset function
                        has_clicked = false;
                        walls.clear();
                        game.bird.x = 200;
                        game.bird.y = window_height_config / 2;
                        game.bird.y_vel = 0;
                        game.last_wall_spawn_frames = game.wall_delay_frames;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s) {
                        //speed toggle
                        speed_toggle = !speed_toggle;
                    }
                }


                Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame
                if (game.last_wall_spawn_frames >= game.wall_delay_frames) {
                    game.last_wall_spawn_frames = 0;
                    walls.emplace_back(Wall());
                }

                if (!speed_toggle) {
                    // Cap FPS
                    if (frameTime < frameDelay_ms) {
                        SDL_Delay(frameDelay_ms - frameTime);
                    }
                }


                if (!game.check_collision(&game.bird, &walls)) {
                    //check collisions, if not
                    game.update(renderer, &game.bird, has_clicked, &walls, render_config); //main update function
                }
            }
        }

        break;
        case 1: {
            //train model headed by default; option to enable/disable graphics -- RENDERING ONLY OCCURS FOR TRAINING THREAD; all multithreaded envs remain unrendered

            vector<GameEnv> envs = vector<GameEnv>(0);//create vector for game envs
            envs.reserve(threads-1);//reserve memory space for each env
            cout <<"initializing..." << endl;//very epic debug
            for (int i = 0; i < threads-1; i++) {//for loop to create environments
                envs.emplace_back(GameEnv());
            }

            ReplayBuffer replayBuffer = ReplayBuffer(replay_buffer_size_config);


            size_t train_steps = 0;
            const size_t target_net_update_interval = 1000;

            //Model Reqs init
            Network network = Network();
            network.init();//initilize network params
            Network target_network = network;//deep copy of network to start for target net

            auto policy = EpsilonGreedyPolicy(policy_decay_config, min_epsilon_config);


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
                for (size_t i = 0; i < envs.size(); i++) {
                    GameEnv &env = envs[i];
                    env.update(renderer, has_clicked, (render_config && i==1));
                }


                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        running = false;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s) {
                        //speed toggle
                        // -- removes FPS cap
                        speed_toggle = !speed_toggle;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_h) {
                        //render toggle -- reduced gpu overhead, faster network training if off
                        render_config = !render_config;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_m) {
                        //save model
                        network.save_model();
                    }
                }


                Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame
                if (env.last_wall_spawn_frames >= env.wall_delay_frames) {
                    env.last_wall_spawn_frames = 0;
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

                // ε-greedy with unbiased greedy tie
                int greedy =
                        (q_values[0] > q_values[1]) ? 0 : (q_values[1] > q_values[0]) ? 1 : action_dist(rng);
                // random on tie

                int action = (real_dist(rng) < policy.getEpsilon()) ? action_dist(rng) == 1 ? 1 : 0 : greedy;

                if (action == 1) {
                    env.bird.y_vel = -5;
                    env.flaps++;
                }
                env.iterations++;

                //update the game with model choice
                env.update(renderer, &env.bird, has_clicked, &walls, render_config);

                //get post model choice data
                game_state = env.get_game_state(&env.bird, &walls); //retrieve game state

                double reward = env.get_reward(game_state, &env.bird, env.check_collision(&env.bird, &walls));
                if (action == 1) {
                    reward = -0.01;
                }
                //cout << flush;
                if (env.bird.score > env.max_score) {
                    env.max_score = env.bird.score;
                }
                if (static_cast<int>(env.iterations) % 100 == 0) {
                    cout << "Reward: " << reward << "  Epsilon: " << policy.getEpsilon() << " Episodes: " << env.
                            episodes << " Flap Rate: " << env.flaps / env.iterations << " Max Score: " << env.max_score <<
                            "\n";
                    cout << q_values[0] << " " << q_values[1] << " \n";
                }


                //save data to a replay buffer
                replay_buffer.emplace_back(Transition{
                    initial_game_state,
                    action,
                    reward,
                    game_state,
                    env.check_collision(&env.bird, &walls) //enter the "end of episode" function
                });

                // Training phase (sample batch from replay buffer)

                if (replay_buffer.size() > replay_buffer_sample_size_config) {
                    std::uniform_int_distribution<size_t> idx_dist(0, replay_buffer.size() - 1);

                    for (int n = 0; n < replay_buffer_sample_size_config; ++n) {
                        const Transition &t = replay_buffer[idx_dist(rng)];

                        // Double DQN target
                        std::vector<float> q_next_online = network.forward(t.next_state);
                        int a_star = int(std::distance(
                            q_next_online.begin(),
                            std::max_element(q_next_online.begin(), q_next_online.end())
                        ));

                        std::vector<float> q_next_target = target_network.forward(t.next_state);
                        float next_val = q_next_target[a_star];

                        float bootstrap = t.done ? 0.0f : gamma_config * next_val;

                        std::vector<float> q_cur = network.forward(t.state);
                        std::vector<float> target = q_cur;
                        target[t.action] = static_cast<float>(t.reward) + bootstrap;


                        {
                            network.backward(target);
                        }
                        train_steps++;
                    }

                    // Periodic hard update of target net
                    if (train_steps % target_net_update_interval == 0) {
                        target_network = network;
                    }

                    policy.decay();
                }


                //make sure replay buffer remains within buffer size limit
                if ((int) replay_buffer.size() > replay_buffer_size_config) {
                    //if buffer exceeds max size, delete oldest entry
                    replay_buffer.erase(replay_buffer.begin());
                }

                //reset the bird if it has crashed
                if (env.check_collision(&env.bird, &walls)) {
                    //reset functionss
                    walls.clear();
                    env.bird.x = 200;
                    env.bird.y = window_height_config / 2;
                    env.bird.y_vel = 0;
                    env.last_wall_spawn_frames = env.wall_delay_frames;
                    env.episodes++;
                    env.bird.score = 0;
                }
            }
        }

        break;
        case 2:
            //train model, multithreaded

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
