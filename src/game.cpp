#include "game.h"
#include "world.h"
#include <cmath>
#include <algorithm>

static Equipment generateEquip(Random& rng, int slot, int level) {
    Equipment eq;
    eq.slot = slot;
    auto q = (Quality)rng.nextInt(5);
    eq.quality = q;
    eq.name = std::string(QUALITY_NAMES[(int)q]) + "_" + SLOT_NAMES[slot];
    float m = QUALITY_MULTS[(int)q];
    int ba = (slot == 0) ? 5 : 0;
    int bd = (slot >= 1 && slot <= 3) ? 4 : 0;
    int bh = (slot == 2 || slot == 4) ? 15 : 0;
    eq.atk = (int)((ba + rng.nextFloat(0, 4)) * m);
    eq.def = (int)((bd + rng.nextFloat(0, 3)) * m);
    eq.hp  = (int)((bh + rng.nextFloat(0, 6)) * m);
    eq.level = level;
    return eq;
}

void initGame(Game& g, int winW, int winH) {
    g.W = winW; g.H = winH;
    g.rng.reset(42);
    genWorld(g.world, g.rng);
    recalcStats(g.player);
    g.camera.x = g.player.x - winW / 2.0f;
    g.camera.y = g.player.y - winH / 2.0f;
}

void recalcStats(Player& p) {
    int atk = p.baseAtk, def = p.baseDef, hp = p.baseHp;
    for (int i = 0; i < 5; i++) {
        auto& e = p.equipment[i];
        atk += e.atk; def += e.def; hp += e.hp;
    }
    p.attack = atk; p.defense = def;
    p.maxHp = hp + p.level * 10;
    p.hp = std::min(p.hp, p.maxHp);
    p.maxMana = 50 + p.level * 5;
    p.mana = std::min(p.mana, p.maxMana);
}

bool checkPassable(const World& w, float x, float y, int ps) {
    float corners[4][2] = {
        {x - ps, y - ps}, {x + ps, y - ps},
        {x - ps, y + ps}, {x + ps, y + ps}
    };
    for (auto& c : corners) {
        int col = (int)(c[0] / TILE), row = (int)(c[1] / TILE);
        if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return false;
        if (!TERRAIN_PASSABLE[w.terrain[row][col]]) return false;
    }
    return true;
}

void addParticle(Game& g, float x, float y, uint32_t color, float life) {
    Particle p;
    p.x = x; p.y = y;
    p.vx = g.rng.nextFloat(-0.5f, 0.5f) * 60;
    p.vy = -40 - g.rng.nextFloat(0, 1) * 40;
    p.life = life; p.maxLife = life;
    p.color = color;
    p.size = 2 + g.rng.nextFloat(0, 1) * 3;
    g.particles.push_back(p);
}

void addFloatingText(Game& g, float x, float y, const std::string& text, uint32_t color) {
    FloatingText ft;
    ft.x = x; ft.y = y;
    ft.text = text;
    ft.color = color;
    ft.life = 1.2f; ft.maxLife = 1.2f;
    ft.vy = -45;
    g.floatingTexts.push_back(ft);
}

void meleeAttack(Game& g) {
    auto& p = g.player;
    float ax = p.x + std::cos(p.facing * 3.14159265f / 2) * 35;
    float ay = p.y + std::sin(p.facing * 3.14159265f / 2) * 35;

    for (auto& e : g.world.enemies) {
        if (e.hp <= 0) continue;
        if (std::hypot(ax - e.x, ay - e.y) < 40) {
            e.hp -= p.attack;
            addParticle(g, e.x, e.y, 0xFF6666);
        }
    }
    if (g.world.boss.hp > 0 && std::hypot(ax - g.world.boss.x, ay - g.world.boss.y) < 55) {
        g.world.boss.hp -= p.attack;
        addParticle(g, g.world.boss.x, g.world.boss.y, 0xFFAA00);
    }
}

void collectItem(Game& g, WorldItem& it) {
    it.collected = true;
    auto& p = g.player;
    switch (it.type) {
        case ItemType::Coin:         p.coins++;  break;
        case ItemType::HealthPotion: p.potions++; break;
        case ItemType::ManaGem:
            p.gems++;
            p.mana = std::min(p.maxMana, p.mana + 10);
            break;
        case ItemType::AncientRelic: p.relics++; break;
        case ItemType::Equipment:
            p.inventory.push_back(it.equip);
            addFloatingText(g, it.x, it.y, "get_equip!", 0xFFD700);
            break;
    }
}

void handleInteract(Game& g) {
    auto& p = g.player;
    for (auto& n : g.world.npcs) {
        if (std::hypot(p.x - n.x, p.y - n.y) < 55) {
            if (n.role == "shop") {
                g.shopItems.clear();
                for (int i = 0; i < 4; i++)
                    g.shopItems.push_back(generateEquip(g.rng, i, p.level));
                g.setGameState(GameState::Shop);
                return;
            }
            if (n.name == "village_chief" && g.world.storyProgress == 0) {
                g.dialogNPC = n.name;
                g.dialogLines = n.dialogue;
                g.dialogIdx = 0;
                g.dialogTimer = 0;
                g.setGameState(GameState::Dialog);
                g.world.storyProgress = 1;
                return;
            }
            g.dialogNPC = n.name;
            g.dialogLines = {n.dialogue[g.rng.nextInt((int)n.dialogue.size())]};
            g.dialogIdx = 0;
            g.dialogTimer = 0;
            g.setGameState(GameState::Dialog);
            return;
        }
    }
    for (auto& c : g.world.chests) {
        if (!c.opened && std::hypot(p.x - c.x, p.y - c.y) < 45) {
            c.opened = true;
            p.coins += c.coins;
            p.potions += c.potions;
            p.gems += c.gems;
            p.relics += c.relics;
        }
    }
}

void useSkill(Game& g, const std::string& id) {
    auto& p = g.player;
    int idx = -1;
    if (id == "slash") idx = 0;
    else if (id == "heal") idx = 1;
    else if (id == "fireball") idx = 2;
    if (idx < 0) return;

    const auto& s = SKILL_DEFS[idx];
    if (p.mana < s.cost || p.skillCooldowns[id] > 0) return;

    p.mana -= s.cost;
    p.skillCooldowns[id] = s.cd;

    if (idx == 0) {
        // slash: instant melee
        meleeAttack(g);
    } else if (idx == 1) {
        p.hp = std::min(p.maxHp, p.hp + s.heal);
    } else if (idx == 2) {
        // fireball AoE
        float fx = p.x + std::cos(p.facing * 3.14159265f / 2) * s.range;
        float fy = p.y + std::sin(p.facing * 3.14159265f / 2) * s.range;
        for (auto& e : g.world.enemies) {
            if (e.hp <= 0) continue;
            if (std::hypot(fx - e.x, fy - e.y) < 60) {
                e.hp -= s.dmg;
                addParticle(g, e.x, e.y, 0xFF4444);
            }
        }
        if (g.world.boss.hp > 0 && std::hypot(fx - g.world.boss.x, fy - g.world.boss.y) < 70) {
            g.world.boss.hp -= s.dmg;
            addParticle(g, g.world.boss.x, g.world.boss.y, 0xFF00FF);
        }
        // visual puff
        for (int i = 0; i < 8; i++)
            addParticle(g, fx, fy, 0xFF8800, 0.3f);
    }
}

void updateQuests(Game& g) {
    auto& w = g.world;
    int deadEnemies = 0;
    for (auto& e : w.enemies) if (e.hp <= 0) deadEnemies++;

    if (w.storyProgress == 1 && g.player.coins >= 5 && deadEnemies >= 3) {
        w.storyProgress = 2;
        g.dialogNPC = "system";
        g.dialogLines = {"quest_complete", "find_hermit_for_next"};
        g.dialogIdx = 0; g.dialogTimer = 0;
        g.setGameState(GameState::Dialog);
    }
    if (w.storyProgress == 2 && g.player.relics >= 3) {
        w.storyProgress = 3;
        g.dialogNPC = "system";
        g.dialogLines = {"relics_collected", "defeat_swamp_behemoth"};
        g.dialogIdx = 0; g.dialogTimer = 0;
        g.setGameState(GameState::Dialog);
    }
    if (w.storyProgress == 3 && w.boss.hp <= 0) {
        w.storyProgress = 4;
        g.setGameState(GameState::Victory);
    }
}

void update(Game& g, float dt) {
    auto& w = g.world;
    auto& p = g.player;
    auto& cam = g.camera;

    if (w.gameState != GameState::Playing) {
        if (w.gameState == GameState::Dialog) {
            g.dialogTimer += dt;
        }
        return;
    }

    auto& weather = w.weather;
    // Weather cycle
    w.weatherTimer += dt;
    if (w.weatherTimer > 25) {
        w.weatherTimer = 0;
        float r = g.rng.next();
        weather = (r < 0.5f) ? "clear" : (r < 0.8f ? "rain" : "snow");
        w.weatherDur = 12 + g.rng.nextFloat(0, 18);
    }
    if (w.weatherDur > 0) w.weatherDur -= dt;
    else weather = "clear";

    // Cooldowns
    if (p.attackCd > 0) p.attackCd -= dt;
    if (p.invincible > 0) p.invincible -= dt;
    for (auto& [k, v] : p.skillCooldowns) if (v > 0) v -= dt;

    // Knockback decay
    p.knockX *= std::exp(-7 * dt);
    p.knockY *= std::exp(-7 * dt);
    if (std::abs(p.knockX) < 0.5f) p.knockX = 0;
    if (std::abs(p.knockY) < 0.5f) p.knockY = 0;

    // Enemies AI
    for (auto& e : w.enemies) {
        if (e.hp <= 0) continue;
        float d = std::hypot(p.x - e.x, p.y - e.y);
        if (d < 220) {
            float spd = e.speed * dt;
            e.x += (p.x - e.x) / d * spd;
            e.y += (p.y - e.y) / d * spd;
            if (d < 38 && e.cd <= 0 && p.invincible <= 0) {
                p.hp -= std::max(1, e.atk - p.defense);
                p.invincible = 0.6f; e.cd = 1.2f;
                p.knockX = (p.x - e.x) * 0.5f;
                p.knockY = (p.y - e.y) * 0.5f;
            }
        }
        if (e.cd > 0) e.cd -= dt;
    }

    // Boss AI
    auto& boss = w.boss;
    if (boss.hp > 0) {
        float d = std::hypot(p.x - boss.x, p.y - boss.y);
        if (d < 380 && boss.cd <= 0) {
            boss.cd = (boss.phase == 1) ? 1.2f : 1.8f;
            int cnt = (boss.phase == 1) ? 1 : 5;
            float ang = std::atan2(p.y - boss.y, p.x - boss.x);
            for (int i = 0; i < cnt; i++) {
                float a = ang + (cnt > 1 ? (i - 2) * 0.35f : 0);
                boss.projectiles.push_back({boss.x, boss.y, std::cos(a) * 160, std::sin(a) * 160, 2.5f});
            }
            if (boss.hp < 200 && boss.phase == 1) boss.phase = 2;
        }
        if (boss.cd > 0) boss.cd -= dt;
        for (auto& proj : boss.projectiles) {
            proj.x += proj.vx * dt;
            proj.y += proj.vy * dt;
            proj.life -= dt;
            if (proj.life > 0 && std::hypot(p.x - proj.x, p.y - proj.y) < 22 && p.invincible <= 0) {
                p.hp -= 20;
                p.invincible = 0.5f;
                proj.life = 0;
            }
        }
        boss.projectiles.erase(
            std::remove_if(boss.projectiles.begin(), boss.projectiles.end(),
                [](const Projectile& pr) { return pr.life <= 0; }),
            boss.projectiles.end());
    }

    // Death
    if (p.hp <= 0) {
        p.hp = p.maxHp;
        p.x = WORLD_W / 2.0f;
        p.y = WORLD_H / 2.0f + TILE * 2;
        p.coins = std::max(0, p.coins - 5);
    }

    // Level up
    if (p.exp >= p.expToLevel) {
        p.level++;
        p.exp -= p.expToLevel;
        p.expToLevel = (int)(p.expToLevel * 1.5f);
        p.baseHp += 10; p.baseAtk += 2; p.baseDef += 1;
        recalcStats(p);
    }

    // Camera
    cam.tx = p.x - g.W / 2.0f;
    cam.ty = p.y - g.H / 2.0f;
    cam.tx = std::max(0.0f, std::min((float)(WORLD_W - g.W), cam.tx));
    cam.ty = std::max(0.0f, std::min((float)(WORLD_H - g.H), cam.ty));
    cam.x += (cam.tx - cam.x) * dt * 7;
    cam.y += (cam.ty - cam.y) * dt * 7;

    // Particles
    for (int i = (int)g.particles.size() - 1; i >= 0; i--) {
        auto& pt = g.particles[i];
        pt.x += pt.vx * dt; pt.y += pt.vy * dt;
        pt.life -= dt;
        if (pt.life <= 0) g.particles.erase(g.particles.begin() + i);
    }

    // Floating texts
    for (int i = (int)g.floatingTexts.size() - 1; i >= 0; i--) {
        auto& ft = g.floatingTexts[i];
        ft.y += ft.vy * dt;
        ft.life -= dt;
        if (ft.life <= 0) g.floatingTexts.erase(g.floatingTexts.begin() + i);
    }

    // Weather particles (rain/snow)
    if (weather == "rain" && g.rng.next() < 0.5f) {
        RainDrop rd;
        rd.x = cam.x + g.rng.next() * g.W;
        rd.y = cam.y - 20;
        rd.vy = 500;
        rd.life = 0;
        g.rains.push_back(rd);
    }
    if (weather == "snow" && g.rng.next() < 0.3f) {
        SnowFlake sf;
        sf.x = cam.x + g.rng.next() * g.W;
        sf.y = cam.y - 20;
        sf.vy = 80;
        sf.vx = g.rng.nextFloat(-0.5f, 0.5f) * 40;
        sf.life = 0;
        g.snows.push_back(sf);
    }
    for (int i = (int)g.rains.size() - 1; i >= 0; i--) {
        auto& rd = g.rains[i];
        rd.y += rd.vy * dt;
        rd.life += dt;
        if (rd.life > 1.0f) g.rains.erase(g.rains.begin() + i);
    }
    for (int i = (int)g.snows.size() - 1; i >= 0; i--) {
        auto& sf = g.snows[i];
        sf.y += sf.vy * dt;
        sf.x += sf.vx * dt;
        sf.life += dt;
        if (sf.life > 3.0f) g.snows.erase(g.snows.begin() + i);
    }

    // Item pickups
    for (auto& it : w.items) {
        if (!it.collected && std::hypot(p.x - it.x, p.y - it.y) < 28) {
            collectItem(g, it);
        }
    }

    p.defeatedEnemies = 0;
    for (auto& e : w.enemies) if (e.hp <= 0) p.defeatedEnemies++;

    updateQuests(g);
    w.dayTime = std::fmod(w.dayTime + 0.005f * dt, 1.0f);
}

void updateInputMotion(Game& g, float ix, float iy, bool attack, bool interact, const std::string& skillId) {
    auto& p = g.player;

    if (g.world.gameState != GameState::Playing) return;

    // Normalize
    float mag = std::sqrt(ix * ix + iy * iy);
    if (mag > 1) { ix /= mag; iy /= mag; }

    // Terrain speed modifier
    int col = (int)(p.x / TILE), row = (int)(p.y / TILE);
    float tmod = (row >= 0 && row < ROWS && col >= 0 && col < COLS) ?
        TERRAIN_SPEED[g.world.terrain[row][col]] : 1.0f;
    if (tmod <= 0) tmod = 1.0f;

    // Update facing
    if (mag > 0.1f) {
        if      (std::abs(ix) > std::abs(iy) && ix > 0) p.facing = 0;
        else if (std::abs(iy) > std::abs(ix) && iy > 0) p.facing = 1;
        else if (std::abs(ix) > std::abs(iy) && ix < 0) p.facing = 2;
        else if (std::abs(iy) > std::abs(ix) && iy < 0) p.facing = 3;
    }

    // Movement (velocity-based for smoothness)
    float dt = 0.016f; // caller should pass actual dt, but we use a placeholder here
    // Actually we can't use proper dt since this is called per-frame independently.
    // We'll handle actual movement apply in main loop using the direction stored on player.
    // Store direction info on player for main update loop
    // The actual velocity integration is done in update() but it depends on ix,iy
    // For simplicity, we compute movement here and let update handle the rest
    // This function is called once per frame, so we use the player's vx/vy as accumulators
    // The main loop calls update(dt) which also does movement -- we need to unify this.
    // Let's just set a flag or handle this in the main loop directly.
    // For the C++ version, movement will be handled inline in the main loop.
    // This function is kept for reference/mobile input processing.
}