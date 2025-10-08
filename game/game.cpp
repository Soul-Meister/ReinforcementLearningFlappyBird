#include "./game.h"
#include "../Bird/Bird.h"
#include <SDL2/SDL.h>


void update(SDL_Renderer *renderer, Bird* pbird, bool has_clicked) {
    if (has_clicked) {
        pbird->update();
    }
    render(renderer, pbird);
}

void render(SDL_Renderer *renderer, Bird* pbird) {

    //Clear the window bro
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    //make a yellow box (bird) bro
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_Rect bird_rect = {static_cast<int>(pbird->x), static_cast<int>(pbird->y), static_cast<int>(pbird->width), static_cast<int>(pbird->height), };
    SDL_RenderFillRect(renderer, &bird_rect);

    //show it bro
    SDL_RenderPresent(renderer);
}