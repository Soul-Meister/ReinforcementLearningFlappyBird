#include "GameEnv.h"

#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>

#include "../main/Config.h"
#include <SDL2/SDL.h>



#include "../Agent/Network/Network.h"
#include "../Agent/Policies/EpsilonGreedyPolicy.h"



GameEnv::GameEnv(int ID)
    : rng(std::random_device{}() + ID * 1337u),
      real_dist(0.0f, 1.0f),
      flap_dist(0.5)
{
   // flap_dist(0.15);  // 15% flap

    action = 0;
    flaps = 0;
    episodes = 0;
    iterations = 0;
    max_score = 0;
    wall_delay_frames = 500;
    last_wall_spawn_frames = wall_delay_frames;//will go to 0 then increment up; this is to immediately spawn one


}





float GameEnv::normalize(float value, float min, float max) {
    if (max == min) return 0.0f;
    return 2.0f * ((value - min) / (max - min)) - 1.0f;
}


void GameEnv::update(SDL_Renderer* renderer, bool has_clicked, bool _render) {
    //human player logic lol
    if (has_clicked) {
        bird.update();//update bird position

        //wall update logic
        last_wall_spawn_frames++;
        if (last_wall_spawn_frames > wall_delay_frames) {//create new wall every wall_delay_frames
            last_wall_spawn_frames = 0;
            walls.emplace_back(Wall());
        }

        iterations++;

        for (int i = 0; i < static_cast<int>(walls.size()); i++) {//updater for walls
            walls.at(i).x_pos -= wall_speed_config;//update x position of walls relative to speed set in config
            if (walls.at(i).x_pos+walls.at(i).width < 0) {
                walls.erase(walls.begin() + i);
            }
            if (walls.at(i).x_pos+walls.at(i).width < bird.x && !walls.at(i).is_scored) {
                walls.at(i).is_scored = true;
                bird.score++;
                bird.unchecked_score = true;
                //cout << "Score: " << bird.score << endl;
            }
        }
        //POST UPDATE NETWORK LOGIC
        if (bird.score > max_score) {
            max_score = bird.score;
        }
    }
}

void GameEnv::update(bool has_clicked, Network* network, bool render_) {
    if (has_clicked) {
        bird.update();//update bird position

        //wall update logic
        last_wall_spawn_frames++;
        if (last_wall_spawn_frames > wall_delay_frames) {//create new wall every wall_delay_frames
            last_wall_spawn_frames = 0;
            walls.emplace_back(Wall());
        }

        // action/replay buffer logic
        auto game_state = get_game_state();
        auto initial_game_state = game_state;
        auto q_values = network->forward(game_state); //assign result to memory to avoid multiple calls

        // greedy action
        int greedy = (q_values[0] > q_values[1])
                         ? 0
                         : (q_values[1] > q_values[0])
                               ? 1
                               : std::uniform_int_distribution<int>(0, 1)(rng);

        action = greedy;

        if (action == 1) {
            bird.y_vel = -5;
            flaps++;
        }
        iterations++;

        for (int i = 0; i < static_cast<int>(walls.size()); i++) {//updater for walls
            walls.at(i).x_pos -= wall_speed_config;//update x position of walls relative to speed set in config
            if (walls.at(i).x_pos+walls.at(i).width < 0) {
                walls.erase(walls.begin() + i);
            }
            if (walls.at(i).x_pos+walls.at(i).width < bird.x && !walls.at(i).is_scored) {
                walls.at(i).is_scored = true;
                bird.score++;
                bird.unchecked_score = true;
                //cout << "Score: " << bird.score << endl;
            }
        }
        //POST UPDATE NETWORK LOGIC
        if (bird.score > max_score) {
            max_score = bird.score;
        }
    }
}


void GameEnv::update(bool has_clicked, EpsilonGreedyPolicy policy, Network* network, ReplayBuffer* replay_buffer, bool render_) {
    if (has_clicked) {//update after clicked; only used in gamemode 0 for human player, otherwise its always true
        //bird update logic

       // std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count() * replay_buffer->size());

        bird.update();//update bird position

        //wall update logic
        last_wall_spawn_frames++;
        if (last_wall_spawn_frames > wall_delay_frames) {//create new wall every wall_delay_frames
            last_wall_spawn_frames = 0;
            walls.emplace_back(Wall());
        }

        // action/replay buffer logic
        auto game_state = get_game_state();
        auto initial_game_state = game_state;
        auto q_values = network->forward(game_state); //assign result to memory to avoid multiple calls

        // greedy action
        int greedy = (q_values[0] > q_values[1])
                         ? 0
                         : (q_values[1] > q_values[0])
                               ? 1
                               : std::uniform_int_distribution<int>(0, 1)(rng);

        // ε-greedy

        if (real_dist(rng) < policy.getEpsilon()) {
            // true random: 50/50 flap vs no flap
            action = flap_dist(rng) ? 1 : 0;
        } else {
            action = greedy;
        }


        if (action == 1) {
            bird.y_vel = -5;
            flaps++;
        }
        iterations++;


        for (int i = 0; i < static_cast<int>(walls.size()); i++) {//updater for walls
            walls.at(i).x_pos -= wall_speed_config;//update x position of walls relative to speed set in config
            if (walls.at(i).x_pos+walls.at(i).width < 0) {
                  walls.erase(walls.begin() + i);
            }
            if (walls.at(i).x_pos+walls.at(i).width < bird.x && !walls.at(i).is_scored) {
                walls.at(i).is_scored = true;
                bird.score++;
                bird.unchecked_score = true;
                //cout << "Score: " << bird.score << endl;
            }
        }
        //POST UPDATE NETWORK LOGIC
        game_state = get_game_state();

        double reward = get_reward(game_state, &bird, check_collision(&bird, &walls));

        if (bird.score > max_score) {
            max_score = bird.score;
        }

        //save to replay buffer
        ReplayBuffer::Transition tr{
            initial_game_state,
            action,
            reward,
            game_state,
            check_collision(&bird, &walls),
            1.0f
        };

        if (bird.unchecked_score) {
            tr.priority = 5.0f;         // more likely to be sampled
        } else {
            tr.priority = 1.0f;         // baseline
        }

        replay_buffer->push(std::move(tr));


//no need for update logic here; one thread handles it in main


    }
}

void render(SDL_Renderer *renderer, Bird* pbird, vector<Wall>* pwalls) {//one game renderer

    //Clear the window bro
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    //make a yellow box (bird) bro (draw the bird)
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect bird_rect = {static_cast<int>(pbird->x), static_cast<int>(pbird->y), static_cast<int>(pbird->width), static_cast<int>(pbird->height)};
    SDL_RenderFillRect(renderer, &bird_rect);

    //draw walls
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);//avoid re-iteration by setting color once for walls
    for (Wall wall : *pwalls) {

        //draw top wall
        SDL_Rect wall_top_rect = {static_cast<int>(wall.x_pos), static_cast<int>(wall.y_pos - wall.gap_size - wall.height), wall.width, wall.height};
        SDL_RenderFillRect(renderer, &wall_top_rect);

        //draw bottom wall
        SDL_Rect wall_bottom_rect = {static_cast<int>(wall.x_pos), static_cast<int>(wall.y_pos + wall.gap_size), wall.width, wall.height};
        SDL_RenderFillRect(renderer, &wall_bottom_rect);
    }

    //show it bro
    SDL_RenderPresent(renderer);
}

void render_mini_window(SDL_Renderer* renderer, GameEnv* env, int x_min, int y_min, int tile_width, int tile_height)
{
    // Set viewport so everything is clipped to this tile
    SDL_Rect viewport { x_min, y_min, tile_width, tile_height };
    SDL_RenderSetViewport(renderer, &viewport);

    // Clear tile background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, nullptr);

    // Compute scale factors from world -> tile
    double scale_x = double(tile_width)  / window_width_config;
    double scale_y = double(tile_height) / window_height_config;

    // --- draw bird ---
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect bird_rect {
        int(env->bird.x      * scale_x),
        int(env->bird.y      * scale_y),
        int(env->bird.width  * scale_x),
        int(env->bird.height * scale_y)
    };
    SDL_RenderFillRect(renderer, &bird_rect);

    // --- draw walls ---
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (const Wall& wall : env->walls) {
        // top wall
        SDL_Rect wall_top_rect {
            int(wall.x_pos * scale_x),
            int((wall.y_pos - wall.gap_size - wall.height) * scale_y),
            int(wall.width  * scale_x),
            int(wall.height * scale_y)
        };
        SDL_RenderFillRect(renderer, &wall_top_rect);

        // bottom wall
        SDL_Rect wall_bottom_rect {
            int(wall.x_pos * scale_x),
            int((wall.y_pos + wall.gap_size) * scale_y),
            int(wall.width  * scale_x),
            int(wall.height * scale_y)
        };
        SDL_RenderFillRect(renderer, &wall_bottom_rect);
    }

    // --- draw borders once per tile, outside wall loop ---
    SDL_SetRenderDrawColor(renderer, 65, 121, 163, 255);

    // top border
    SDL_Rect top_border_rect {
        0,
        0,
        tile_width,
        border_thickness_config
    };
    SDL_RenderFillRect(renderer, &top_border_rect);

    // bottom border
    SDL_Rect bottom_border_rect {
        0,
        tile_height - border_thickness_config,
        tile_width,
        border_thickness_config
    };
    SDL_RenderFillRect(renderer, &bottom_border_rect);

    // left border
    SDL_Rect left_border_rect {
        0,
        0,
        border_thickness_config,
        tile_height
    };
    SDL_RenderFillRect(renderer, &left_border_rect);

    // right border
    SDL_Rect right_border_rect {
        tile_width - border_thickness_config,
        0,
        border_thickness_config,
        tile_height
    };
    SDL_RenderFillRect(renderer, &right_border_rect);

    // reset viewport for the rest of the frame
    SDL_RenderSetViewport(renderer, nullptr);
}


//multiple env renderer
void render(SDL_Renderer* renderer, vector<GameEnv>* envs) {

    //RENDER EACH ENV
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int columns = floor(sqrt(envs->size()));
    int rows = ceil((float)envs->size() / columns);

    int tile_w = window_width_config / columns;
    int tile_h = window_height_config / rows;

    double scale_x = double(tile_w) / window_width_config;
    double scale_y = double(tile_h) / window_height_config;
    double scale = (double)(scale_x + scale_y) / 2.0;

    for (int i = 0; i < envs->size(); i++) {
        int x = (i % columns) * tile_w;
        int y = (i / columns) * tile_h;

        render_mini_window(renderer, &envs->at(i), x, y, tile_w, tile_h);
    }
    SDL_RenderPresent(renderer);
}




bool GameEnv::check_collision(Bird* pbird, vector<Wall>* pwalls) {
    // top/bottom
    if (pbird->y < 0) {
        return true;
    }

    if (pbird->y + pbird->height > window_height_config) return true;

    for (const Wall& wall : *pwalls) {
        if (wall.x_pos < pbird->x + pbird->width && wall.x_pos + wall.width > pbird->x) {
            // top rect
            if (pbird->y < wall.y_pos - wall.gap_size) return true;
            // bottom rect
            if (pbird->y + pbird->height > wall.y_pos + wall.gap_size) return true;
        }
    }
    return false;
}


vector<float> GameEnv::get_game_state() {
    //bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y,



    for (int i = 0; i < static_cast<int>(walls.size()); i++) {
        if (!walls.at(i).is_scored) {
            if (walls.size() > 1) {//If we have at least two walls, return the next two walls of data
                // In get_game_state (two-wall branch)
                float dx0 = walls.at(i).x_pos + bird.width - bird.x;
                float dx1 = walls.at(i+1).x_pos - bird.x;

                return vector<float>{
                    normalize(bird.x, 0, window_width_config),
                    normalize(bird.y + bird.height * 0.5f, 0, window_height_config),
                    normalize(bird.y_vel, -5, 5),
                    normalize(dx0, 0, window_width_config),
                    0,//normalize(dx1, 0, window_width_config),
                    normalize(walls.at(i).y_pos,  gap_width_config/2, window_height_config-gap_width_config/2),
                    0,//normalize(walls.at(i+1).y_pos,  gap_width_config/1.9, window_height_config-gap_width_config/1.9),
                };
            }
            else {//else, return just the next wall. THERE WILL ALWAYS BE AT LEAST ONE WALL -- i hope
                // In get_game_state (two-wall branch)
                float dx0 = walls.at(i).x_pos - + bird.width + bird.x;
                return vector<float>{
                    normalize(bird.x + bird.width * 0.5f, 0, window_width_config),
                    normalize(bird.y + bird.height * 0.5f, 0, window_height_config),
                    normalize(bird.y_vel, -10, 10),
                    normalize(dx0,  0, window_width_config),
                    0,
                    normalize(walls.at(i).y_pos, gap_width_config/2, window_height_config-gap_width_config/2),
                    0
                };

            }
        }
    }
    return vector<float>{0, 0, 0, 0, 0, 0, 0};
}

//bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y, next_next_wall_y

double GameEnv::get_reward(const std::vector<float> &state, Bird *pBird, bool done) {
    constexpr double ALIVE_BONUS = 0.01;
    constexpr double ALIGN_WEIGHT = 0.1;
    constexpr double VEL_ABS_WEIGHT = 0.0;
    constexpr double VEL_UP_WEIGHT = 0.0;
    constexpr double APPROACH_WALL_WEIGHT = 0.0;
    constexpr double EDGE_PENALTY_WEIGHT = 0.1;
    constexpr double PIPE_REWARD = 1.0;
    constexpr double DEATH_PENALTY = 1.0;
    constexpr double EDGE_MARGIN_PX = 200;
    constexpr double LOCAL_FLAP_PENALTY_WEIGHT = 0.45;


    double reward = 0.0;

    double vel_up_ = std::max(0.0, -double(state[2])); // state[2] is normalized vel

    if (state[1] < state[5] && vel_up_ > 0.0) { // above gap & moving up
        reward -= LOCAL_FLAP_PENALTY_WEIGHT * vel_up_;
    }
    double vel_down = std::max(0.0, double(state[2])); // positive = going down

    if (state[1] > state[5] && vel_down > 0.0) { // below gap & moving down
        reward -= LOCAL_FLAP_PENALTY_WEIGHT * vel_down;
    }


    // 1) survival bonus
    reward += ALIVE_BONUS;

    // 2) vertical alignment with next gap (both in [-1,1])

    double d = std::fabs(double(state[1]) - double(state[5])) * 0.5; // d ∈ [0,1]
    d = std::clamp(d, 0.0, 1.0);

    // 1 when perfectly aligned, -1 when very far
    double align = 1.0 - 2.0 * d;     // align ∈ [-1,1]

    reward += ALIGN_WEIGHT * align;

    // 3) velocity penalties (state[2] ∈ [-1,1])
    double vel_abs = std::fabs(double(state[2]));
    double vel_up  = std::max(0.0, -double(state[2])); // up is negative if you subtract to go up
    reward -= VEL_ABS_WEIGHT * vel_abs;
    reward -= VEL_UP_WEIGHT  * vel_up;

    // 4) approach the wall: state[3] is normalized dx ∈ [-1,1]
    // map to [0,1]: 0=near, 1=far → reward (1 - dx01)
    double dx01 = (double(state[3]) + 1.0) * 0.5; // ∈ [0,1]
    reward += APPROACH_WALL_WEIGHT * (1.0 - dx01);

    // 5) edge proximity penalty (in pixels)
    double top_pen = 0.0, bot_pen = 0.0;
    if (pBird->y < EDGE_MARGIN_PX)
        top_pen = (EDGE_MARGIN_PX - pBird->y) / EDGE_MARGIN_PX;
    double bottom_line = double(window_height_config) - EDGE_MARGIN_PX;
    double bird_bottom = pBird->y + pBird->height;
    if (bird_bottom > bottom_line)
        bot_pen = (bird_bottom - bottom_line) / EDGE_MARGIN_PX;
    double edge_pen = std::min(1.0, top_pen + bot_pen);
    reward -= EDGE_PENALTY_WEIGHT * edge_pen;



    // 6) big event terms
    if (pBird->unchecked_score) {
        pBird->unchecked_score = false;
        reward += PIPE_REWARD;
    }
    if (done) {
        reward -= DEATH_PENALTY;
    }
    if (abs(reward) > 100) {
        cout << "REWARD EXPLODED" << endl;
        terminate();
    }
    reward = clamp(reward, -1.0, 1.0);

    return reward;
}




