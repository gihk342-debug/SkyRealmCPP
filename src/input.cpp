#include "input.h"

void processInput(InputState& input, bool& quit) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            quit = true;
            break;

        case SDL_KEYDOWN: {
            auto sym = e.key.keysym.sym;
            if (sym == SDLK_w || sym == SDLK_UP)    input.up = true;
            if (sym == SDLK_s || sym == SDLK_DOWN)  input.down = true;
            if (sym == SDLK_a || sym == SDLK_LEFT)  input.left = true;
            if (sym == SDLK_d || sym == SDLK_RIGHT) input.right = true;
            if (sym == SDLK_SPACE) input.attack = true;
            if (sym == SDLK_e)     input.interact = true;
            if (sym == SDLK_i)     input.inventory = true;
            if (sym == SDLK_m)     input.map = true;
            if (sym == SDLK_ESCAPE) input.escape = true;
            if (sym == SDLK_h)     input.potion = true;
            if (sym == SDLK_g)     input.gem = true;
            if (sym == SDLK_1)     input.skillSlot = "slash";
            if (sym == SDLK_2)     input.skillSlot = "heal";
            if (sym == SDLK_3)     input.skillSlot = "fireball";

            // Shop choices
            if (sym >= SDLK_1 && sym <= SDLK_4)
                input.shopChoice = sym - SDLK_1;
            // Inventory choices
            if (sym >= SDLK_1 && sym <= SDLK_9)
                input.invChoice = sym - SDLK_1;
            break;
        }

        case SDL_KEYUP: {
            auto sym = e.key.keysym.sym;
            if (sym == SDLK_w || sym == SDLK_UP)    input.up = false;
            if (sym == SDLK_s || sym == SDLK_DOWN)  input.down = false;
            if (sym == SDLK_a || sym == SDLK_LEFT)  input.left = false;
            if (sym == SDLK_d || sym == SDLK_RIGHT) input.right = false;
            if (sym == SDLK_SPACE) input.attack = false;
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
        case SDL_FINGERDOWN:
            // Start screen click
            break;

        default:
            break;
        }
    }
}

void resetInput(InputState& input) {
    input.interact = false;
    input.inventory = false;
    input.map = false;
    input.escape = false;
    input.potion = false;
    input.gem = false;
    input.skillSlot.clear();
    input.shopChoice = -1;
    input.invChoice = -1;
}