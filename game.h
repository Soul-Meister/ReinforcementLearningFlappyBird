//
// Created by edward on 10/6/25.
//
#pragma once
#include <SDL2/SDL.h>
#include "Bird.h"


#ifndef FLAPPYBIRD_GAME_H
#define FLAPPYBIRD_GAME_H

void update(SDL_Renderer*, Bird, bool);
void render(SDL_Renderer*, Bird*);

#endif //FLAPPYBIRD_GAME_H