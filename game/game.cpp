#include "game.h"
#include "../main/config.h"
#include <SDL2/SDL.h>


void update(SDL_Renderer *renderer, Bird* pbird, bool has_clicked, vector<Wall>* pwalls) {
    if (has_clicked) {
        pbird->update();//update bird

        for (Wall &wall : *pwalls) {
            wall.x_pos -= wall_speed;//update x position of walls relative to speed
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