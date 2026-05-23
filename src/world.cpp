#include "world.h"
#include <cmath>

void genWorld(World& w, Random& rng) {
    w.terrain.assign(ROWS, std::vector<Terrain>(COLS, Terrain::GRASS));
    int cx = COLS / 2, cy = ROWS / 2;

    struct Biome { int c, r; float rad; Terrain ter; int fall; };
    Biome biomes[] = {
        {cx,      cy,      15, Terrain::LUSH,     6},
        {cx - 19, cy - 11, 11, Terrain::SAND,     5},
        {cx + 17, cy - 9,  13, Terrain::FOREST,   6},
        {cx + 22, cy + 6,   9, Terrain::DENSE,    4},
        {cx - 9,  cy - 20, 10, Terrain::MOUNTAIN, 5},
        {cx + 6,  cy + 18, 12, Terrain::WATER,    6},
        {cx - 15, cy + 9,   8, Terrain::SWAMP,    4},
        {cx - 24, cy - 18,  8, Terrain::SAND,     5},
        {cx + 25, cy - 3,  10, Terrain::DARK,     5},
    };

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            float best = 1e9f, sbest = 1e9f;
            Terrain bter = Terrain::GRASS, ster = Terrain::GRASS;
            for (auto& b : biomes) {
                float dr = (float)(r - b.r), dc = (float)(c - b.c);
                float dist = std::sqrt(dr * dr + dc * dc) + rng.nextFloat(-1.4f, 1.4f);
                if (dist < b.rad + b.fall) {
                    if (dist < best) {
                        sbest = best; ster = bter;
                        best = dist; bter = b.ter;
                    } else if (dist < sbest) {
                        sbest = dist; ster = b.ter;
                    }
                }
            }
            w.terrain[r][c] = (best < sbest - 3.5f) ? bter :
                (sbest < 22 ? (rng.next() < 0.6f ? bter : ster) : Terrain::GRASS);
        }
    }

    // Roads (5 spokes from center+2)
    for (int i = 0; i < 5; i++) {
        float ang = i * 3.14159265f * 2 / 5 + rng.nextFloat(0, 0.5f);
        float px = (float)cx, py = (float)(cy + 2);
        for (int s = 0; s < 15; s++) {
            px += std::cos(ang) * 0.8f;
            py += std::sin(ang) * 0.8f;
            int rc = (int)std::round(py), cc = (int)std::round(px);
            if (rc >= 0 && rc < ROWS && cc >= 0 && cc < COLS &&
                TERRAIN_PASSABLE[w.terrain[rc][cc]])
                w.terrain[rc][cc] = Terrain::ROAD;
        }
    }

    // Village
    for (int r = cy - 3; r <= cy + 3; r++) {
        for (int c = cx - 4; c <= cx + 4; c++) {
            if (r >= 0 && r < ROWS && c >= 0 && c < COLS &&
                std::hypot((float)(r - cy), (float)(c - cx)) < 5 &&
                TERRAIN_PASSABLE[w.terrain[r][c]])
                w.terrain[r][c] = Terrain::VILLAGE;
        }
    }

    // Clear dynamic objects
    w.items.clear();
    w.npcs.clear();
    w.chests.clear();
    w.enemies.clear();

    // NPCs
    w.npcs.push_back({(float)cx * TILE, (float)(cy + 2) * TILE,
        "village_chief", "quest",
        {"welcome_hero", "collect_5_coins_defeat_3_enemies"}, 0xE8C86A});
    w.npcs.push_back({(float)(cx + 16) * TILE, (float)(cy - 6) * TILE,
        "forest_hermit", "hint",
        {"find_ancient_relics"}, 0x7DCEA0});
    w.npcs.push_back({(float)(cx - 15) * TILE, (float)(cy + 8) * TILE,
        "traveling_merchant", "shop",
        {"welcome"}, 0xF0C078});

    // Items (80)
    for (int i = 0; i < 80; i++) {
        float ix = rng.next() * WORLD_W, iy = rng.next() * WORLD_H;
        int tr = (int)(iy / TILE), tc = (int)(ix / TILE);
        if (tr >= 0 && tr < ROWS && tc >= 0 && tc < COLS && TERRAIN_PASSABLE[w.terrain[tr][tc]]) {
            WorldItem it;
            it.x = ix; it.y = iy; it.sparkTimer = rng.next() * 6.283185f;
            float roll = rng.next();
            if (roll < 0.4f) it.type = ItemType::Coin;
            else if (roll < 0.6f) it.type = ItemType::HealthPotion;
            else if (roll < 0.75f) it.type = ItemType::ManaGem;
            else if (roll < 0.9f) it.type = ItemType::AncientRelic;
            else {
                it.type = ItemType::Equipment;
                int slot = rng.nextInt(5);
                int qualityIdx = rng.nextInt(5);
                it.equip.slot = slot;
                it.equip.name = std::string(QUALITY_NAMES[qualityIdx]) + "_" + SLOT_NAMES[slot];
                it.equip.quality = (Quality)qualityIdx;
                float m = QUALITY_MULTS[qualityIdx];
                int ba = (slot == 0) ? 5 : 0;
                int bd = (slot >= 1 && slot <= 3) ? 4 : 0;
                int bh = (slot == 2 || slot == 4) ? 15 : 0;
                it.equip.atk = (int)((ba + rng.nextFloat(0, 4)) * m);
                it.equip.def = (int)((bd + rng.nextFloat(0, 3)) * m);
                it.equip.hp  = (int)((bh + rng.nextFloat(0, 6)) * m);
                it.equip.level = 1;
            }
            w.items.push_back(it);
        }
    }

    // Enemies (16 slimes)
    for (int i = 0; i < 16; i++) {
        float ex = rng.next() * WORLD_W, ey = rng.next() * WORLD_H;
        int tr = (int)(ey / TILE), tc = (int)(ex / TILE);
        if (tr >= 0 && tr < ROWS && tc >= 0 && tc < COLS &&
            TERRAIN_PASSABLE[w.terrain[tr][tc]] && w.terrain[tr][tc] != Terrain::VILLAGE) {
            Enemy e;
            e.x = ex; e.y = ey;
            e.speed = 35 + rng.nextFloat(0, 20);
            w.enemies.push_back(e);
        }
    }

    // Boss
    w.boss.x = (float)(cx + 4) * TILE;
    w.boss.y = (float)(cy + 19) * TILE;
    w.boss.hp = 400; w.boss.maxHp = 400;
    w.boss.atk = 28; w.boss.phase = 1; w.boss.cd = 0;
    w.boss.projectiles.clear();

    // Chests
    w.chests.push_back({(float)(cx + 14) * TILE, (float)(cy - 6) * TILE, false, 20, 3, 2, 0});
    w.chests.push_back({(float)(cx - 12) * TILE, (float)(cy + 10) * TILE, false, 15, 0, 0, 1});
}

void resetGame(World& w, Player& p, Random& rng) {
    rng.reset(42);
    genWorld(w, rng);
    p.x = WORLD_W / 2.0f; p.y = WORLD_H / 2.0f + TILE * 2;
    p.hp = p.maxHp; p.mana = p.maxMana;
    p.coins = 0; p.potions = 3; p.gems = 1; p.relics = 0;
    p.level = 1; p.exp = 0;
    for (int i = 0; i < 5; i++) p.equipment[i] = {};
    p.inventory.clear();
    p.skills = {"slash"};
    p.skillCooldowns.clear();
    p.defeatedEnemies = 0;
    w.storyProgress = 0;
    w.gameState = GameState::Playing;
    w.dayTime = 0.35f;
    w.weather = "clear";
    w.weatherTimer = 0;
    w.weatherDur = 0;
}