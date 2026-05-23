#pragma once
#include "types.h"
#include "random.h"
#include <vector>

// Terrain grid: [row][col]
using TerrainGrid = std::vector<std::vector<Terrain>>;

struct World {
    TerrainGrid terrain;
    std::vector<WorldItem> items;
    std::vector<NPC> npcs;
    std::vector<Chest> chests;
    std::vector<Enemy> enemies;
    Boss boss;
    GameState gameState = GameState::Start;
    int storyProgress = 0;
    float dayTime = 0.35f;
    float weatherTimer = 0, weatherDur = 0;
    std::string weather = "clear";
};

void genWorld(World& w, Random& rng);
void resetGame(World& w, Player& p, Random& rng);