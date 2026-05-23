#pragma once
#include "types.h"
#include "world.h"
#include "random.h"
#include <vector>
#include <string>

// ---- Skill definitions ----
inline const SkillDef SKILL_DEFS[] = {
    {"heavy_slash", 10, 2.5f, 2.0f, 0, 0, 0},
    {"heal",        20, 8.0f, 0,    35, 0, 0},
    {"fireball",    15, 4.0f, 0,    0,  40, 120},
};

// ---- Global game state ----
struct Game {
    World world;
    Player player;
    Camera camera;
    Random rng;

    // Particles & effects
    std::vector<Particle>    particles;
    std::vector<FloatingText> floatingTexts;
    std::vector<std::pair<float, float>> raindrops;  // (x, vy)
    std::vector<std::pair<float, float>> snowflakes; // (x, vy) -- x is drift
    struct SnowFlake { float x, y, vx, vy, life; };
    struct RainDrop { float x, y, vy, life; };
    std::vector<SnowFlake>  snows;
    std::vector<RainDrop>   rains;

    // Dialog state
    std::string dialogNPC;
    std::vector<std::string> dialogLines;
    int dialogIdx = 0;
    float dialogTimer = 0;

    // Shop items
    std::vector<Equipment> shopItems;

    // Time
    float gameTime = 0;

    // UI flags
    bool showBigMap    = false;
    bool showInventory = false;

    // Window dimensions
    int W = MAX_W, H = MAX_H;

    // Game state
    GameState gameState() const { return world.gameState; }
    void setGameState(GameState s) { world.gameState = s; }
};

// ---- Functions ----
void initGame(Game& g, int winW, int winH);
bool checkPassable(const World& w, float x, float y, int playerSize);
void recalcStats(Player& p);

void addParticle(Game& g, float x, float y, uint32_t color, float life = 0.5f);
void addFloatingText(Game& g, float x, float y, const std::string& text, uint32_t color = 0xFFFFFF);

void meleeAttack(Game& g);
void collectItem(Game& g, WorldItem& it);
void handleInteract(Game& g);
void useSkill(Game& g, const std::string& id);
void updateQuests(Game& g);

void update(Game& g, float dt);
void updateInputMotion(Game& g, float ix, float iy, bool attack, bool interact, const std::string& skillSlot);