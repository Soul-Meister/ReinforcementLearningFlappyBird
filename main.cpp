#include <iostream>
#include <SDL2/SDL.h>

#include "game.h"
#include "Bird.h"

using namespace std;

int main() {

    int target_fps = 10;
    Uint32 frameDelay_ms = 1000/target_fps;


    if (SDL_Init(SDL_INIT_VIDEO)) {
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
    }

    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);

    if (!window) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    bool running = true;
    SDL_Event event;

    bool has_clicked = false;
    Bird bird = Bird();




        while (running) {//main update loop
            Uint32 startTime = SDL_GetTicks();

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                    cout << "SPACE" << endl;
                    has_clicked = true;
                }
            }

            update(renderer, bird, has_clicked);

            Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame

            // Cap FPS
            if (frameTime < frameDelay_ms) {
                SDL_Delay(frameDelay_ms - frameTime);
            }
        }
    //ayo, clean it
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}