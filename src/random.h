#pragma once
#include <cstdint>

// xorshift random generator (matches JS version)
class Random {
public:
    Random(uint32_t s = 42) : seed(s) {}

    float next() {
        uint32_t t = seed += 0x6D2B79F5;
        t = (t ^ (t >> 15)) * (t | 1);
        t ^= t + ((t ^ (t >> 7)) * (t | 61));
        seed = t;
        return (float)((t ^ (t >> 14)) & 0xFFFFFFFFu) / 4294967296.0f;
    }

    int nextInt(int max) { return (int)(next() * max); }
    float nextFloat(float min, float max) { return min + next() * (max - min); }
    void reset(uint32_t s = 42) { seed = s; }

private:
    uint32_t seed;
};