#include "game.h"

#include <algorithm>

#include "../main/config.h"
#include <SDL2/SDL.h>

#include <iostream>


float normalize(float value, float min, float max) {
    if (max == min) return 0.0f;
    return 2.0f * ((value - min) / (max - min)) - 1.0f;
}



void update(SDL_Renderer *renderer, Bird* pbird, bool has_clicked, vector<Wall>* pwalls, bool render_) {
    if (has_clicked) {//update per click
        pbird->update();//update bird

        for (int i = 0; i < static_cast<int>(pwalls->size()); i++) {//updater for walls
            pwalls->at(i).x_pos -= wall_speed_config;//update x position of walls relative to speed
            if (pwalls->at(i).x_pos+pwalls->at(i).width < 0) {
                  pwalls->erase(pwalls->begin() + i);
            }
            if (pwalls->at(i).x_pos+pwalls->at(i).width < pbird->x && !pwalls->at(i).is_scored) {
                pwalls->at(i).is_scored = true;
                pbird->score++;
                pbird->unchecked_score = true;
                cout << "Score: " << pbird->score << endl;
            }
        }
    }
    if (render_) {
        render(renderer, pbird, pwalls);
    }
}

void render(SDL_Renderer *renderer, Bird* pbird, vector<Wall>* pwalls) {

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

        //draw top wall
        SDL_Rect wall_bottom_rect = {static_cast<int>(wall.x_pos), static_cast<int>(wall.y_pos + wall.gap_size), wall.width, wall.height};
        SDL_RenderFillRect(renderer, &wall_bottom_rect);
    }

    //show it bro
    SDL_RenderPresent(renderer);
}

bool check_collision(Bird* pbird, vector<Wall>* pwalls) {
    // top/bottom
    if (pbird->y < 0) return true;
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


vector<float> get_game_state(Bird* pBird, vector<Wall>* pwalls) {
    //bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y,



    for (int i = 0; i < static_cast<int>(pwalls->size()); i++) {
        if (!pwalls->at(i).is_scored) {
            if (pwalls->size() > 1) {//If we have at least two walls, return the next two walls of data
                // In get_game_state (two-wall branch)
                float dx0 = pwalls->at(i).x_pos   - pBird->x;
                float dx1 = pwalls->at(i+1).x_pos - pBird->x;

                return vector<float>{
                    normalize(pBird->x, 0, window_width_config),
                    normalize(pBird->y, 0, window_height_config),
                    normalize(pBird->y_vel, -10, 10),
                    normalize(dx0, 0, window_width_config),
                    normalize(dx1, 0, window_width_config),
                    normalize(pwalls->at(i).y_pos,  gap_width_config/1.9, window_height_config-gap_width_config/1.9),
                    normalize(pwalls->at(i+1).y_pos,  gap_width_config/1.9, window_height_config-gap_width_config/1.9),
                };
            }
            else {//else, return just the next wall. THERE WILL ALWAYS BE AT LEAST ONE WALL -- i hope
                // In get_game_state (two-wall branch)
                float dx0 = pwalls->at(i).x_pos   - pBird->x;
                return vector<float>{
                    normalize(pBird->x, 0, window_width_config),
                    normalize(pBird->y, 0, window_height_config),
                    normalize(pBird->y_vel, -10, 10),
                    normalize(dx0,  0, window_width_config),
                    0,
                    normalize(pwalls->at(i).y_pos, gap_width_config/1.9, window_height_config-gap_width_config/1.9),
                    0
                };

            }
        }
    }
    return vector<float>{0, 0, 0, 0, 0, 0, 0};
}

//bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y, next_next_wall_y

double get_reward(const std::vector<float>& state, Bird* bird, bool done) {
    // ---- tunable weights (feel free to tweak) ----
    constexpr double ALIVE_BONUS         = 0.05;  // dense survival reward
    constexpr double ALIGN_WEIGHT        = 3;  // vertical alignment with gap
    constexpr double VEL_ABS_WEIGHT      = 0.15;  // penalty for big |velocity|
    constexpr double VEL_UP_WEIGHT       = 0.075;  // extra penalty for upward motion
    constexpr double APPROACH_WALL_WEIGHT= 0.00;  // reward for being close to next wall
    constexpr double EDGE_PENALTY_WEIGHT = 0.6;  // penalty near top/bottom edges
    constexpr double PIPE_REWARD         = 30.0;  // reward for passing a pipe
    constexpr double DEATH_PENALTY       = 50.0;  // penalty on crash
    constexpr double EDGE_MARGIN_PX      = 120.0; // proximity width for edge penalty

    double reward = 0.0;

if (bird->y_vel < -4.85) {
    reward -= 1;
}
    // 1) survival bonus
    reward += ALIVE_BONUS;

    // 2) vertical alignment with next gap (both in [-1,1])
    double vertical_distance = std::fabs(double(state[1]) - double(state[5])); // ∈ [0, 2]
    double align_reward = 1.0 - 0.5 * vertical_distance;                       // map → [0,1]
    align_reward = std::clamp(align_reward, 0.0, 1.0);
    reward += ALIGN_WEIGHT * align_reward;

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
    if (bird->y < EDGE_MARGIN_PX)
        top_pen = (EDGE_MARGIN_PX - bird->y) / EDGE_MARGIN_PX;
    double bottom_line = double(window_height_config) - EDGE_MARGIN_PX;
    double bird_bottom = bird->y + bird->height;
    if (bird_bottom > bottom_line)
        bot_pen = (bird_bottom - bottom_line) / EDGE_MARGIN_PX;
    double edge_pen = std::min(1.0, top_pen + bot_pen);
    reward -= EDGE_PENALTY_WEIGHT * edge_pen;

    // 6) big event terms
    if (bird->unchecked_score) {
        bird->unchecked_score = false;
        reward += PIPE_REWARD;
    }
    if (done) {
        reward -= DEATH_PENALTY;
    }

    return reward;
}




