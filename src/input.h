#pragma once
#include <SDL.h>
#include <string>

struct InputState {
    bool up = false, down = false, left = false, right = false;
    bool attack = false;
    bool interact = false;
    std::string skillSlot; // "slash", "heal", "fireball", or ""
    bool inventory = false;
    bool map = false;
    bool escape = false;
    bool potion = false;
    bool gem = false;
    int shopChoice = -1; // 0-3
    int invChoice = -1;  // 0-8

    // Joystick (mobile)
    float joyX = 0, joyY = 0;
    bool joyActive = false;
    float joyBaseX = 0, joyBaseY = 0; // center position
};

void processInput(InputState& input, bool& quit);
void resetInput(InputState& input);