#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <cmath>
#include <algorithm>

// ---- Constants ----
constexpr int MAX_W       = 960;
constexpr int MAX_H       = 640;
constexpr int TILE        = 40;
constexpr int COLS        = 64;
constexpr int ROWS        = 50;
constexpr int WORLD_W     = COLS * TILE;   // 2560
constexpr int WORLD_H     = ROWS * TILE;   // 2000
constexpr int PLAYER_SIZE = 16;

// ---- Terrain ----
enum Terrain : int {
    DEEP = 0, WATER, SAND, GRASS, LUSH, FOREST,
    DENSE, MOUNTAIN, SNOW, ROAD, VILLAGE, SWAMP, DARK, NUM_TERRAIN
};

inline const char* TERRAIN_NAMES[] = {
    "deep", "water", "sand", "grass", "lush", "forest",
    "dense", "mountain", "snow", "road", "village", "swamp", "dark"
};

// Colors as 0xRRGGBB hex
inline const uint32_t TERRAIN_COLORS[] = {
    0x1A5276, 0x2980B9, 0xE8D5A3, 0x7CB342, 0x5D9A2E, 0x3D6B1E,
    0x1E4A0A, 0x7F8C8D, 0xE8E8E8, 0xC4A882, 0xD4B878, 0x5A7247, 0x2A3A1A
};

inline const bool TERRAIN_PASSABLE[] = {
    false, false, true, true, true, true, true, false, false, true, true, true, true
};

inline const float TERRAIN_SPEED[] = {
    0.0f, 0.0f, 0.7f, 1.0f, 0.9f, 0.75f, 0.55f, 0.0f, 0.0f, 1.2f, 1.0f, 0.5f, 0.65f
};

// ---- Equipment ----
enum class Quality : int { Common, Uncommon, Rare, Epic, Legendary, NUM };

inline const char* QUALITY_NAMES[] = {"common", "uncommon", "rare", "epic", "legendary"};
inline uint32_t QUALITY_COLORS[] = { 0xFFFFFF, 0x55FF55, 0x4488FF, 0xCC44FF, 0xFFAA00 };
inline float QUALITY_MULTS[] = { 1.0f, 1.3f, 1.6f, 2.0f, 2.8f };

inline const char* SLOT_NAMES[] = {"weapon", "head", "chest", "legs", "ring"};
inline const int SLOT_COUNT = 5;

struct Equipment {
    int slot = 0;
    std::string name;
    Quality quality = Quality::Common;
    int atk = 0, def = 0, hp = 0;
    int level = 1;
};

// ---- Items ----
enum class ItemType : int { Coin, HealthPotion, ManaGem, AncientRelic, Equipment };

struct WorldItem {
    float x, y;
    ItemType type;
    bool collected = false;
    float sparkTimer = 0;
    Equipment equip;
};

// ---- NPC ----
struct NPC {
    float x, y;
    std::string name;
    std::string role; // quest, hint, shop
    std::vector<std::string> dialogue;
    uint32_t color = 0xFFFFFF;
};

// ---- Chest ----
struct Chest {
    float x, y;
    bool opened = false;
    int coins = 0, potions = 0, gems = 0, relics = 0;
};

// ---- Enemy ----
struct Enemy {
    float x, y;
    int hp = 40, maxHp = 40;
    int atk = 10;
    float speed = 45;
    float cd = 0;
    std::string type = "slime";
};

// ---- Boss Projectile ----
struct Projectile {
    float x, y;
    float vx, vy;
    float life = 2.5f;
};

// ---- Boss ----
struct Boss {
    float x, y;
    int hp = 400, maxHp = 400;
    int atk = 28;
    int phase = 1;
    float cd = 0;
    std::vector<Projectile> projectiles;
    std::string name = "swamp_behemoth";
};

// ---- Particle / Floating Text ----
struct Particle {
    float x, y;
    float vx, vy;
    float life, maxLife;
    uint32_t color;
    float size;
};

struct FloatingText {
    float x, y;
    std::string text;
    uint32_t color = 0xFFFFFF;
    float life = 1.2f, maxLife = 1.2f;
    float vy = -45;
};

// ---- Skills ----
struct SkillDef {
    std::string name;
    int cost;
    float cd;
    // slash
    float dmgMult = 2.0f;
    // heal
    int heal = 35;
    // fireball
    int dmg = 40;
    float range = 120;
};

// ---- Player ----
struct Player {
    float x = WORLD_W / 2.0f, y = WORLD_H / 2.0f + TILE * 2;
    float vx = 0, vy = 0;
    float speed = 185;
    int size = PLAYER_SIZE;

    int baseHp = 100, baseAtk = 5, baseDef = 2;
    int hp = 100, maxHp = 100;
    int mana = 50, maxMana = 50;

    int coins = 0, keys = 0;
    int potions = 3, gems = 1, relics = 0;
    int level = 1;
    int exp = 0, expToLevel = 60;

    int facing = 0;   // 0=right 1=down 2=left 3=up
    float walkCycle = 0;
    float attackCd = 0;
    float invincible = 0;
    float knockX = 0, knockY = 0;

    Equipment equipment[5] = {};  // weapon, head, chest, legs, ring
    std::vector<Equipment> inventory;
    std::vector<std::string> skills = {"slash"};
    std::unordered_map<std::string, float> skillCooldowns;

    int defeatedEnemies = 0;

    int attack = 0, defense = 0;
};

// ---- Camera ----
struct Camera {
    float x = 0, y = 0;
    float tx = 0, ty = 0;
    float shake = 0, shakeDur = 0;
};

// ---- Game State ----
enum class GameState : int { Start, Playing, Dialog, Shop, Inventory, Victory };

struct Pair2F { float x, y; };