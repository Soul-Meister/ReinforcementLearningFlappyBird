#include <iostream>
#include <SDL2/SDL.h>

using namespace std;

int main() {
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

        while (running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                    cout << "yo they pressed space bruh" << endl;
                }
            }

            //Clear the window bro
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            //make a yellow box (bird) bro
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_Rect bird = {390, 220, 20, 20, };
            SDL_RenderFillRect(renderer, &bird);

            //show it bro
            SDL_RenderPresent(renderer);
            cout << "test";
        }
    //ayo, clean it
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}