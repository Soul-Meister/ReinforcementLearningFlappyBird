#include <iostream>
#include <SDL2/SDL.h>
#include <vector>

#include "../game/game.h"
#include "../Bird/Bird.h"
#include "../Wall/Wall.h"
#include "config.h"


using namespace std;

//game globals
int window_height;
int window_width;
int target_fps;
int gap_width_config;
int wall_speed_config;

//agent globals
int replay_buffer_size_config;
int replay_buffer_sample_size_config;



int main() {


    //declare game globals
    window_height = 1200;
    window_width = 1600;
    target_fps = 120;
    gap_width_config = 80;
    wall_speed_config = 1;

    //declare agent globals
    replay_buffer_size_config = 100000;
    replay_buffer_sample_size_config = 128; //64-128
    learning_rate_config = 0.001;

    //network variables -- {inputs, hidden, hidden ... hidden, output}
    network_config = {7, 100, 100, 100, 1};


    //Declare variables

    int wall_delay_frames = 360;
    int last_wall_spawn_frames = wall_delay_frames;//will go to 0 then increment up; this is to immediately spawn onw

    vector<Wall> walls;
    vector<double> game_state(7); //bird_x, bird_y, bird_y_vel, next_wall_x, next_next_wall_x, next_wall_y, next_next_wall_y
    vector<vector<double>> replay_buffer;

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
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {//check if space is pressed
                    has_clicked = true;
                    bird.y_vel = -5;//set bird velocity
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {//reset function
                    has_clicked = false;
                    walls.clear();
                    bird.x =200;
                    bird.y = window_height/2;
                    bird.y_vel = 0;
                    last_wall_spawn_frames = wall_delay_frames;
                }
            }


            Uint32 frameTime = SDL_GetTicks() - startTime; // Calculate time taken for frame
            if (last_wall_spawn_frames >= wall_delay_frames) {
                last_wall_spawn_frames = 0;
                walls.push_back(Wall());
            }

            // Cap FPS
            if (frameTime < frameDelay_ms) {
                SDL_Delay(frameDelay_ms - frameTime);
            }


           if (!check_collision(&bird, &walls)) {//check collisions
                update(renderer, &bird, has_clicked, &walls);//main update function
               game_state = get_game_state(&bird, &walls);//update game state
                replay_buffer.push_back(game_state);//add current gamestate to replay buffer
               if ((int)replay_buffer.size() > (int)replay_buffer_size_config) {//if buffer exceeds max size, delete oldest entry
                   replay_buffer.erase(replay_buffer.begin());
               }
            }

        }
    //ayo, clean it
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}