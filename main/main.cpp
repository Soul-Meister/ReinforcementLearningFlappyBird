#include <iostream>
#include <SDL2/SDL.h>
#include <vector>

#include "../game/game.h"
#include "../Bird/Bird.h"
#include "../Wall/Wall.h"
#include "config.h"


using namespace std;

int window_height;
int window_width;
int target_fps;
int gap_width_config;
int wall_speed;

int main() {
    //declare globals
    window_height = 1200;
    window_width = 1600;
    target_fps = 120;
    gap_width_config = 80;
    wall_speed = 1;


    //Declare variables

    int wall_delay_frames = 360;
    int last_wall_spawn_frames = 0;

    vector<Wall> walls;

    Uint32 frameDelay_ms = 1000/target_fps;




    if (SDL_Init(SDL_INIT_VIDEO)) {
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
    }

    SDL_Window *window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN);

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

            if (has_clicked) {
                last_wall_spawn_frames++;
            }


            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
                    has_clicked = true;
                    bird.y_vel = -5;
                }
            }


            Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame
            if (last_wall_spawn_frames >= wall_delay_frames) {
                last_wall_spawn_frames = 0;
                walls.insert(walls.begin(), Wall());
                cout << "WALL CREATED YO" << endl;
            }

            // Cap FPS
            if (frameTime < frameDelay_ms) {
                SDL_Delay(frameDelay_ms - frameTime);
            }


            if (!check_collision(&bird, &walls)) {//check collisions
                update(renderer, &bird, has_clicked, &walls);
            }

        }//test
    //ayo, clean it
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}