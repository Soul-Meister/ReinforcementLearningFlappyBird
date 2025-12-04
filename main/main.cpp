#include <algorithm>
#include <atomic>
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
#include <thread>


using namespace std;


std::mutex mtx;

int main() {
    const auto config = Config(); //run config, set globals


    vector<Wall> walls;
    //Bird bird = Bird();
    vector<float> game_state(7);
    //bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y, next_next_wall_y

    Uint32 frameDelay_ms = 1000 / target_fps_config;


    std::atomic<bool> running = true;
    SDL_Event event;
    bool speed_toggle = false;
    bool has_clicked = true;

    int train_steps = 0;
    bool tick_decay = true;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        cout << "SDL_Init Error: " << SDL_GetError() << endl; //the now very not epic window failed to open
    }
    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width_config,
                                          window_height_config, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // create RNG engine (seeded by time)
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());


    switch (game_mode) {
        case 0: {
            //human player
            GameEnv game = GameEnv(1);


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


                if (!game.check_collision(&game.bird, &walls)) {
                    //check collisions, if not
                    game.update(renderer, has_clicked, render_config); //main update function
                }
            }
        }

        break;

        case 1: {
            //very epic multithreaded training
            int max_score = 0;


            //train model headed by default; option to enable/disable graphics -- RENDERING ONLY OCCURS FOR TRAINING THREAD; all multithreaded envs remain unrendered
            cout << "Starting...\n";
            vector<GameEnv> envs = vector<GameEnv>(0, 0); //create vector for game envs

            cout << "Reserving memory...";
            envs.reserve(threads_config - 1); //reserve memory space for each env
            cout << "OK\n";

            cout << "Creating Environments...";
            for (int i = 0; i < threads_config - 1; i++) {
                //for loop to create environments
                envs.emplace_back(GameEnv(i));
            }
            cout << "OK\n";

            cout << "Creating Replay Buffer...";
            ReplayBuffer replay_buffer = ReplayBuffer(replay_buffer_size_config);
            cout << "OK\n";

            size_t train_steps = 0;
            const size_t target_net_update_interval = 50000;

            //Model Reqs init
            cout << "Creating Network...";
            Network network = Network(0);
            cout << "OK\n";

            cout << "Initializing Network...";
            network.init(); //initilize network params
            cout << "OK\n";

            cout << "Creating/Initializing Target Network...";
            Network target_network = network; //deep copy of network to start for target net
            cout << "OK\n";

            cout << "initializing Policy...";
            auto policy = EpsilonGreedyPolicy(policy_decay_config, min_epsilon_config);
            cout << "OK\n";


            if (SDL_Init(SDL_INIT_VIDEO)) {
                cout << "SDL_Init Error: " << SDL_GetError() << endl; //the now very not epic window failed to open
            } else {
                cout << "SDL_Init...OK\n";
            }

            if (!window) {
                cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
                SDL_Quit();
                return 1;
            } else {
                cout << "SDL_CreateWindow...OK\n";
            }


            const Uint32 worker_env_fps = 120;
            const Uint32 worker_delay = 1000 / worker_env_fps;


            vector<std::thread> worker_threads;
            for (int i = 0; i < threads_config - 1; i++) {
                worker_threads.emplace_back([&, i]() {
                    auto &env = envs[i];


                    Uint32 last_step_time = SDL_GetTicks();

                    while (running) {
                        if (speed_toggle) {
                            //env.update(has_clicked, policy, &network, &replay_buffer, (false));

                            Uint32 now = SDL_GetTicks();
                            if (now - last_step_time >= worker_delay / 2) {
                                env.update(has_clicked, policy, &network, &replay_buffer, false);
                                last_step_time = now;
                            } else {
                                // Don't burn CPU; yield thread for a tiny bit
                                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
                        } else {
                            // SLOW MODE — rate-limited
                            Uint32 now = SDL_GetTicks();
                            if (now - last_step_time >= worker_delay) {
                                env.update(has_clicked, policy, &network, &replay_buffer, false);
                                last_step_time = now;
                            } else {
                                // Don't burn CPU; yield thread for a tiny bit
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
                        }


                        if (env.check_collision(&env.bird, &env.walls)) {
                            //reset functionss
                            env.walls.clear();
                            env.bird.x = 200;
                            env.bird.y = window_height_config / 2;
                            env.bird.y_vel = 0;
                            env.last_wall_spawn_frames = env.wall_delay_frames;
                            env.episodes++;
                            env.bird.score = 0;
                        }
                    }
                });
            }


            while (running) {
                //main update loop
                Uint32 startTime = SDL_GetTicks();
                Uint32 frameDelay_ms = 1000 / target_fps_config;


                render(renderer, &envs);
                //render(renderer, &envs[0].bird, &envs[0].walls); single env render

                for (GameEnv &env: envs) {
                    if (env.max_score > max_score) {
                        max_score = env.max_score;
                    }
                }


                if (envs[0].iterations % 20 == 0 && envs[0].iterations > 0) {
                    auto forward = network.forward(envs[0].get_game_state());
                    double flap_rate = (envs[0].iterations > 0)
                                           ? double(envs[0].flaps) / double(envs[0].iterations)
                                           : 0.0;
                    cout << "Epsilon: " << policy.getEpsilon() << "  Max Score: " << max_score << "  Flap Rate: " <<
                            flap_rate << "  Steps: " << envs[0].iterations << "  qValue[0]: " << forward[0] <<
                            " qValue[1]: " << forward[1] << endl;
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
                        //render toggle -- reduced gpu overhead, faster network training if off -- changed with multithreading. I cant tell a difference in training time with/without this, but Ill keep it just in case
                        render_config = !render_config;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_m) {
                        //save model
                        //running = false; //temp for debug
                        network.save_model(&network); //still need to make this lol
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p) {
                        policy.epsilon = policy.epsilon == min_epsilon_config ? 0 : min_epsilon_config;
                        tick_decay = !tick_decay;
                    }
                }

                Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame

                if (!speed_toggle) {
                    // Cap FPS
                    if (frameTime < frameDelay_ms) {
                        SDL_Delay(frameDelay_ms - frameTime);
                    }
                }


                // Training phase (sample batch from replay buffer)

                if (replay_buffer.size() > replay_buffer_sample_size_config) {
                    if (++train_steps % 16 == 0) {
                        auto batch = replay_buffer.sample_prioritized(replay_buffer_sample_size_config, rng);
                        if (batch.empty()) {
                            // something's off, but just skip safely
                        } else {
                            for (const auto &t: batch) {
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
                                target[t.action] = t.reward + bootstrap;

                                network.backward(target);
                                train_steps++;
                            }
                        }

                        if (train_steps % target_net_update_interval == 0) {
                            target_network = network;
                        }
                        if (tick_decay) {
                            policy.decay();
                        }
                    }
                }
            }
        }
        break;
        case 2: {
            //very epic multithreaded training
            int max_score = 0;


            //train model headed by default; option to enable/disable graphics -- RENDERING ONLY OCCURS FOR TRAINING THREAD; all multithreaded envs remain unrendered
            cout << "Starting...\n";
            vector<GameEnv> envs = vector<GameEnv>(0, 0); //create vector for game envs

            cout << "Reserving memory...";
            envs.reserve(threads_config - 1); //reserve memory space for each env
            cout << "OK\n";

            cout << "Creating Environments...";
            for (int i = 0; i < threads_config - 1; i++) {
                //for loop to create environments
                envs.emplace_back(GameEnv(i));
            }
            cout << "OK\n";

            size_t train_steps = 0;
            const size_t target_net_update_interval = 50000;

            //Model Reqs init
            cout << "Creating Network...";
            Network network = Network(0);
            cout << "OK\n";

            cout << "Initializing Network...";
            network = Network::load_model("models/Model_" + to_string(load_model_num) + ".txt");
            //initilize network params
            cout << "OK\n";

            cout << "Creating/Initializing Target Network...";
            Network target_network = network; //deep copy of network to start for target net
            cout << "OK\n";


            if (SDL_Init(SDL_INIT_VIDEO)) {
                cout << "SDL_Init Error: " << SDL_GetError() << endl; //the now very not epic window failed to open
            } else {
                cout << "SDL_Init...OK\n";
            }

            if (!window) {
                cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
                SDL_Quit();
                return 1;
            } else {
                cout << "SDL_CreateWindow...OK\n";
            }

            const Uint32 worker_env_fps = 120;
            const Uint32 worker_delay = 1000 / worker_env_fps;

            vector<std::thread> worker_threads;
            for (int i = 0; i < threads_config - 1; i++) {
                worker_threads.emplace_back([&, i]() {
                    auto &env = envs[i];


                    Uint32 last_step_time = SDL_GetTicks();

                    while (running) {
                        if (speed_toggle) {
                            //env.update(has_clicked, policy, &network, &replay_buffer, (false));

                            Uint32 now = SDL_GetTicks();
                            if (now - last_step_time >= worker_delay / 2) {
                                env.update(has_clicked, &network, false);
                                last_step_time = now;
                            } else {
                                // Don't burn CPU; yield thread for a tiny bit
                                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
                        } else {
                            // SLOW MODE — rate-limited
                            Uint32 now = SDL_GetTicks();
                            if (now - last_step_time >= worker_delay) {
                                env.update(has_clicked, &network, false);
                                last_step_time = now;
                            } else {
                                // Don't burn CPU; yield thread for a tiny bit
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
                        }


                        if (env.check_collision(&env.bird, &env.walls)) {
                            //reset functionss
                            env.walls.clear();
                            env.bird.x = 200;
                            env.bird.y = window_height_config / 2;
                            env.bird.y_vel = 0;
                            env.last_wall_spawn_frames = env.wall_delay_frames;
                            env.episodes++;
                            env.bird.score = 0;
                        }
                    }
                });
            }

            while (running) {
                //main update loop
                Uint32 startTime = SDL_GetTicks();
                Uint32 frameDelay_ms = 1000 / target_fps_config;


                render(renderer, &envs);
                //render(renderer, &envs[0].bird, &envs[0].walls); single env render

                for (GameEnv &env: envs) {
                    if (env.max_score > max_score) {
                        max_score = env.max_score;
                    }
                }


                if (envs[0].iterations % 20 == 0 && envs[0].iterations > 0) {
                    auto forward = network.forward(envs[0].get_game_state());
                    double flap_rate = (envs[0].iterations > 0)
                                           ? double(envs[0].flaps) / double(envs[0].iterations)
                                           : 0.0;
                    cout << "  Max Score: " << max_score << "  Flap Rate: " << flap_rate << "  qValue[0]: " << forward[
                        0] << " qValue[1]: " << forward[1] << endl;
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
                        //render toggle -- reduced gpu overhead, faster network training if off -- changed with multithreading. I cant tell a difference in training time with/without this, but Ill keep it just in case
                        render_config = !render_config;
                    }
                }

                Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame

                if (!speed_toggle) {
                    // Cap FPS
                    if (frameTime < frameDelay_ms) {
                        SDL_Delay(frameDelay_ms - frameTime);
                    }
                }
            }
        }
        break;
        case 3: {
            //yuhh
            int max_score = 0;


            //train model headed by default; option to enable/disable graphics -- RENDERING ONLY OCCURS FOR TRAINING THREAD; all multithreaded envs remain unrendered
            cout << "Starting...\n";
            vector<GameEnv> envs = vector<GameEnv>(0, 0); //create vector for game envs

            cout << "Reserving memory...";
            envs.reserve(threads_config - 1); //reserve memory space for each env
            cout << "OK\n";

            cout << "Creating Environments...";
            for (int i = 0; i < 1; i++) {
                //for loop to create environments
                envs.emplace_back(GameEnv(i));
            }
            cout << "OK\n";

            size_t train_steps = 0;
            const size_t target_net_update_interval = 50000;

            //Model Reqs init
            cout << "Creating Network...";
            Network network = Network(0);
            cout << "OK\n";

            cout << "Initializing Network...";
            network = Network::load_model("models/Model_" + to_string(load_model_num) + ".txt");
            //initilize network params
            cout << "OK\n";

            cout << "Creating/Initializing Target Network...";
            Network target_network = network; //deep copy of network to start for target net
            cout << "OK\n";


            if (SDL_Init(SDL_INIT_VIDEO)) {
                cout << "SDL_Init Error: " << SDL_GetError() << endl; //the now very not epic window failed to open
            } else {
                cout << "SDL_Init...OK\n";
            }

            if (!window) {
                cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
                SDL_Quit();
                return 1;
            } else {
                cout << "SDL_CreateWindow...OK\n";
            }

            const Uint32 worker_env_fps = 120;
            const Uint32 worker_delay = 1000 / worker_env_fps;

            vector<std::thread> worker_threads;
            for (int i = 0; i < 1; i++) {
                worker_threads.emplace_back([&, i]() {
                    auto &env = envs[i];


                    Uint32 last_step_time = SDL_GetTicks();

                    while (running) {
                        if (speed_toggle) {
                            //env.update(has_clicked, policy, &network, &replay_buffer, (false));

                            Uint32 now = SDL_GetTicks();
                            if (now - last_step_time >= worker_delay / 2) {
                                env.update(has_clicked, &network, false);
                                last_step_time = now;
                            } else {
                                // Don't burn CPU; yield thread for a tiny bit
                                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
                        } else {
                            // SLOW MODE — rate-limited
                            Uint32 now = SDL_GetTicks();
                            if (now - last_step_time >= worker_delay) {
                                env.update(has_clicked, &network, false);
                                last_step_time = now;
                            } else {
                                // Don't burn CPU; yield thread for a tiny bit
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
                        }


                        if (env.check_collision(&env.bird, &env.walls)) {
                            //reset functionss
                            env.walls.clear();
                            env.bird.x = 200;
                            env.bird.y = window_height_config / 2;
                            env.bird.y_vel = 0;
                            env.last_wall_spawn_frames = env.wall_delay_frames;
                            env.episodes++;
                            env.bird.score = 0;
                        }
                    }
                });
            }

            while (running) {
                //main update loop
                Uint32 startTime = SDL_GetTicks();
                Uint32 frameDelay_ms = 1000 / target_fps_config;


                render(renderer, &envs);
                //render(renderer, &envs[0].bird, &envs[0].walls); single env render

                for (GameEnv &env: envs) {
                    if (env.max_score > max_score) {
                        max_score = env.max_score;
                    }
                }


                if (envs[0].iterations % 20 == 0 && envs[0].iterations > 0) {
                    auto forward = network.forward(envs[0].get_game_state());
                    double flap_rate = (envs[0].iterations > 0)
                                           ? double(envs[0].flaps) / double(envs[0].iterations)
                                           : 0.0;
                    cout << "  Max Score: " << max_score << "  Flap Rate: " << flap_rate << "  qValue[0]: " << forward[
                        0] << " qValue[1]: " << forward[1] << endl;
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
                        //render toggle -- reduced gpu overhead, faster network training if off -- changed with multithreading. I cant tell a difference in training time with/without this, but Ill keep it just in case
                        render_config = !render_config;
                    }
                }

                Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame

                if (!speed_toggle) {
                    // Cap FPS
                    if (frameTime < frameDelay_ms) {
                        SDL_Delay(frameDelay_ms - frameTime);
                    }
                }
            }
        }
        break;
        case 4: {
            //very epic multithreaded training
            int max_score = 0;


            //train model headed by default; option to enable/disable graphics -- RENDERING ONLY OCCURS FOR TRAINING THREAD; all multithreaded envs remain unrendered
            cout << "Starting...\n";
            vector<GameEnv> envs = vector<GameEnv>(0, 0); //create vector for game envs

            cout << "Reserving memory...";
            envs.reserve(threads_config - 1); //reserve memory space for each env
            cout << "OK\n";

            cout << "Creating Environments...";
            for (int i = 0; i < threads_config - 1; i++) {
                //for loop to create environments
                envs.emplace_back(GameEnv(i));
            }
            cout << "OK\n";

            cout << "Creating Replay Buffer...";
            ReplayBuffer replay_buffer = ReplayBuffer(replay_buffer_size_config);
            cout << "OK\n";

            size_t train_steps = 0;
            const size_t target_net_update_interval = 50000;

            //Model Reqs init
            cout << "Creating Network...";
            Network network = Network(0);
            cout << "OK\n";

            cout << "Initializing Network...";
            network = Network::load_model("models/Model_" + to_string(load_model_num) + ".txt");
            //initilize network params
            cout << "OK\n";

            cout << "Creating/Initializing Target Network...";
            Network target_network = network; //deep copy of network to start for target net
            cout << "OK\n";

            cout << "initializing Policy...";
            auto policy = EpsilonGreedyPolicy(policy_decay_config, min_epsilon_config);
            cout << "OK\n";


            if (SDL_Init(SDL_INIT_VIDEO)) {
                cout << "SDL_Init Error: " << SDL_GetError() << endl; //the now very not epic window failed to open
            } else {
                cout << "SDL_Init...OK\n";
            }

            if (!window) {
                cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
                SDL_Quit();
                return 1;
            } else {
                cout << "SDL_CreateWindow...OK\n";
            }


            const Uint32 worker_env_fps = 120;
            const Uint32 worker_delay = 1000 / worker_env_fps;


            vector<std::thread> worker_threads;
            for (int i = 0; i < threads_config - 1; i++) {
                worker_threads.emplace_back([&, i]() {
                    auto &env = envs[i];


                    Uint32 last_step_time = SDL_GetTicks();

                    while (running) {
                        if (speed_toggle) {
                            //env.update(has_clicked, policy, &network, &replay_buffer, (false));

                            Uint32 now = SDL_GetTicks();
                            if (now - last_step_time >= worker_delay / 2) {
                                env.update(has_clicked, policy, &network, &replay_buffer, false);
                                last_step_time = now;
                            } else {
                                // Don't burn CPU; yield thread for a tiny bit
                                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
                        } else {
                            // SLOW MODE — rate-limited
                            Uint32 now = SDL_GetTicks();
                            if (now - last_step_time >= worker_delay) {
                                env.update(has_clicked, policy, &network, &replay_buffer, false);
                                last_step_time = now;
                            } else {
                                // Don't burn CPU; yield thread for a tiny bit
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
                        }


                        if (env.check_collision(&env.bird, &env.walls)) {
                            //reset functionss
                            env.walls.clear();
                            env.bird.x = 200;
                            env.bird.y = window_height_config / 2;
                            env.bird.y_vel = 0;
                            env.last_wall_spawn_frames = env.wall_delay_frames;
                            env.episodes++;
                            env.bird.score = 0;
                        }
                    }
                });
            }


            while (running) {
                //main update loop
                Uint32 startTime = SDL_GetTicks();
                Uint32 frameDelay_ms = 1000 / target_fps_config;


                render(renderer, &envs);
                //render(renderer, &envs[0].bird, &envs[0].walls); single env render

                for (GameEnv &env: envs) {
                    if (env.max_score > max_score) {
                        max_score = env.max_score;
                    }
                }


                if (envs[0].iterations % 20 == 0 && envs[0].iterations > 0) {
                    auto forward = network.forward(envs[0].get_game_state());
                    double flap_rate = (envs[0].iterations > 0)
                                           ? double(envs[0].flaps) / double(envs[0].iterations)
                                           : 0.0;
                    cout << "Epsilon: " << policy.getEpsilon() << "  Max Score: " << max_score << "  Flap Rate: " <<
                            flap_rate << "  Steps: " << envs[0].iterations << "  qValue[0]: " << forward[0] <<
                            " qValue[1]: " << forward[1] << endl;
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
                        //render toggle -- reduced gpu overhead, faster network training if off -- changed with multithreading. I cant tell a difference in training time with/without this, but Ill keep it just in case
                        render_config = !render_config;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_m) {
                        //save model
                        //running = false; //temp for debug
                        network.save_model(&network); //still need to make this lol
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p) {
                        policy.epsilon = policy.epsilon == min_epsilon_config ? 0 : min_epsilon_config;
                        tick_decay = !tick_decay;
                    }
                }

                Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame

                if (!speed_toggle) {
                    // Cap FPS
                    if (frameTime < frameDelay_ms) {
                        SDL_Delay(frameDelay_ms - frameTime);
                    }
                }


                // Training phase (sample batch from replay buffer)

                if (replay_buffer.size() > replay_buffer_sample_size_config) {
                    if (++train_steps % 16 == 0) {
                        auto batch = replay_buffer.sample_prioritized(replay_buffer_sample_size_config, rng);
                        if (batch.empty()) {
                            // something's off, but just skip safely
                        } else {
                            for (const auto &t: batch) {
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
                                target[t.action] = t.reward + bootstrap;

                                network.backward(target);
                                train_steps++;
                            }
                        }

                        if (train_steps % target_net_update_interval == 0) {
                            target_network = network;
                        }
                        if (tick_decay) {
                            policy.decay();
                        }
                    }
                }
            }
        }
        break;
        default: ;
    }


    //ayo, clean it


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
