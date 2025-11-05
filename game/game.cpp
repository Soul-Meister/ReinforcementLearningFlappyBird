#include "game.h"
#include "../main/config.h"
#include <SDL2/SDL.h>

#include <iostream>


float normalize(float value, float min, float max) {
    if (max == min) return 0.0f;
    return 2.0f * ((value - min) / (max - min)) - 1.0f;
}



void update(SDL_Renderer *renderer, Bird* pbird, bool has_clicked, vector<Wall>* pwalls) {
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
    render(renderer, pbird, pwalls);
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
    //bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y, next_next_wall_y
    for (int i = 0; i < static_cast<int>(pwalls->size()); i++) {
        if (!pwalls->at(i).is_scored) {
            if (pwalls->size() > 1) {//If we have at least two walls, return the next two walls of data
                return vector<float>{
                    normalize(pBird->x, 0, window_width_config),
                    normalize(pBird->y, 0, window_height_config),
                    normalize(pBird->y_vel, -10, 10),
                    normalize(pwalls->at(i).x_pos, 0, window_width_config),
                    normalize(pwalls->at(i+1).x_pos, 0, window_width_config),
                    normalize(pwalls->at(i).y_pos,  gap_width_config/1.9, window_height_config-gap_width_config/1.9),
                    normalize(pwalls->at(i+1).y_pos,  gap_width_config/1.9, window_height_config-gap_width_config/1.9),
                };
            }
            else {//else, return just the next wall. THERE WILL ALWAYS BE AT LEAST ONE WALL -- i hope
                return vector<float>{
                    normalize(pBird->x, 0, window_width_config),
                    normalize(pBird->y, 0, window_height_config),
                    normalize(pBird->y_vel, -10, 10),
                    normalize(pwalls->at(i).x_pos,  0, window_width_config),
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

double get_reward(const vector<float>& state, Bird* bird, bool done) {
    double reward = 0.0;

    // ─────────────────────────────────────────────
    // 1. Reward for being alive (encourages surviving)
    reward += 0.10;  // small positive reward every frame

    // ─────────────────────────────────────────────
    // 2. Shaping: encourage alignment with the gap center
    // state[1] = bird_y (normalized)
    // state[5] = next_wall_y (normalized)
    double vertical_distance = fabs(state[1] - state[5]);  // 0 is perfect alignment
    reward += (1.0 - vertical_distance);  // max +1, min negative if far

    // ─────────────────────────────────────────────
    // 3. Punish vertical speed (flapping too aggressively)
    // state[2] = bird_y_vel (normalized -1 to 1 ideally)
    double vel_penalty = fabs(state[2]);
    reward -= 0.05 * vel_penalty;  // gentle penalty

    // ─────────────────────────────────────────────
    // 4. Encourage moving forward toward pipes
    // state[3] = next_wall_x (normalized 0 to 1, smaller = closer)
    // So we reward (1 - x), bigger when close to pipe
    double dist_to_wall = state[3];
    reward += 0.05 * (1.0 - dist_to_wall);

    // ─────────────────────────────────────────────
    // 5. Big reward for passing a pipe
    if (bird->unchecked_score) {
        bird->unchecked_score = false;
        reward += 30.0;   // Not too huge, keeps training stable
    }

    // ─────────────────────────────────────────────
    // 6. Big punishment for dying
    if (done) {
        reward -= 50.0;
    }

    return reward;
}





