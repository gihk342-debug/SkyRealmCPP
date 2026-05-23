#pragma once
#include <SDL.h>
#include <string>

// Simple oscillator-based sound effects using SDL audio
void initAudio();
void playTone(float freq, float dur, float vol = 0.08f);

namespace Sfx {
    void pickup();
    void coin();
    void equip();
    void skill();
    void hit();
    void sword();
    void chest();
    void victory();
}