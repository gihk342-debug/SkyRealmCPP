#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "types.h"
#include "game.h"

void initRenderer(SDL_Window*& win, SDL_Renderer*& renderer, TTF_Font*& font, int& W, int& H);
void renderGame(SDL_Renderer* renderer, TTF_Font* font, Game& g);