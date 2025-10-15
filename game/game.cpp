#include "game.h"
#include "../main/config.h"
#include <SDL2/SDL.h>

#include <iostream>


void update(SDL_Renderer *renderer, Bird* pbird, bool has_clicked, vector<Wall>* pwalls) {
    if (has_clicked) {//update per click
        pbird->update();//update bird

        for (int i = 0; i < static_cast<int>(pwalls->size()); i++) {//updater for walls
            pwalls->at(i).x_pos -= wall_speed;//update x position of walls relative to speed
            if (pwalls->at(i).x_pos+pwalls->at(i).width < 0) {
                  pwalls->erase(pwalls->begin() + i);
            }
            if (pwalls->at(i).x_pos+pwalls->at(i).width < pbird->x && !pwalls->at(i).is_scored) {
                pwalls->at(i).is_scored = true;
                pbird->score++;
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
    for (Wall wall : *pwalls) {
        if (wall.x_pos < pbird->x + pbird->width) {
            if (wall.x_pos + wall.width > pbird->x) {//initial x checks
                if (wall.y_pos - wall.gap_size > pbird->y) {//upper
                    return true;
                }
                if (wall.y_pos + wall.gap_size < pbird->y + pbird->height) {//lower
                    return true;
                }
            }
        }
    }

    return false;
}

vector<double> get_game_state(Bird* pBird, vector<Wall>* pwalls) {
    //bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y, next_next_wall_y
    for (int i = 0; i < static_cast<int>(pwalls->size()); i++) {
        if (!pwalls->at(i).is_scored) {
            if (pwalls->size() > 1) {//If we have at least two walls, return the next two walls of data
                return vector<double>{pBird->x, pBird->y, pBird->y_vel, pwalls->at(i).x_pos, pwalls->at(i+1).x_pos, pwalls->at(i).y_pos, pwalls->at(i+1).y_pos};
            }
            else {//else, return just the next wall. THERE WILL ALWAYS BE AT LEAST ONE WALL
                return vector<double>{pBird->x, pBird->y, pBird->y_vel, pwalls->at(i).x_pos, 0, pwalls->at(i).y_pos, 0};

            }
        }
    }
    return vector<double>{0, 0, 0, 0, 0, 0, 0};
}


