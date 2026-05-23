#include "renderer.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

static SDL_Color hexToColor(uint32_t hex) {
    return {
        (Uint8)((hex >> 16) & 0xFF),
        (Uint8)((hex >> 8) & 0xFF),
        (Uint8)(hex & 0xFF),
        255
    };
}

void initRenderer(SDL_Window*& win, SDL_Renderer*& renderer, TTF_Font*& font, int& W, int& H) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

    W = 960; H = 640;
    win = SDL_CreateWindow("SkyRealm - Sky Realm RPG",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        W, H, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Try to load a CJK font; fallback to built-in
    font = TTF_OpenFont("C:/Windows/Fonts/simhei.ttf", 16);
    if (!font) font = TTF_OpenFont("C:/Windows/Fonts/msyh.ttc", 16);
    if (!font) font = TTF_OpenFont("/System/Library/Fonts/PingFang.ttc", 16);
    if (!font) font = TTF_OpenFont("/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc", 16);
    // If still null, we'll render without text or use SDL's built-in glyph surface
}

// Simple text rendering helper
static void drawText(SDL_Renderer* r, TTF_Font* font, const char* text, int x, int y,
                     SDL_Color color, bool centered = false) {
    if (!font) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) { surf = TTF_RenderText_Blended(font, text, color); }
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    if (centered) { dst.x = x - surf->w / 2; }
    SDL_RenderCopy(r, tex, nullptr, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

static void drawCircle(SDL_Renderer* r, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)std::sqrt((float)(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

static void fillCircle(SDL_Renderer* r, int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)std::sqrt((float)(radius * radius - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void renderGame(SDL_Renderer* renderer, TTF_Font* font, Game& g) {
    int W = g.W, H = g.H;
    auto& cam = g.camera;
    auto& w = g.world;
    auto& p = g.player;

    // ---- Start screen ----
    if (w.gameState == GameState::Start) {
        SDL_SetRenderDrawColor(renderer, 10, 10, 20, 255);
        SDL_RenderClear(renderer);
        drawText(renderer, font, "SkyRealm", W / 2, H / 3,
                 {0xE2, 0xB0, 0x4A, 255}, true);
        drawText(renderer, font, "Open World RPG Adventure", W / 2, H / 3 + 50,
                 {0xDD, 0xDD, 0xDD, 255}, true);
        drawText(renderer, font, "Click or press any key to Start", W / 2, H / 2 + 40,
                 {0xFF, 0xFF, 0xFF, 255}, true);
        return;
    }

    // ---- Victory screen ----
    if (w.gameState == GameState::Victory) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 230);
        SDL_RenderClear(renderer);
        drawText(renderer, font, "Victory!", W / 2, H / 3,
                 {0xFF, 0xD7, 0x00, 255}, true);
        drawText(renderer, font, "You defeated the Swamp Behemoth!", W / 2, H / 3 + 60,
                 {0xFF, 0xFF, 0xFF, 255}, true);
        return;
    }

    // ---- Sky background (day/night) ----
    float night = std::max(0.0f, 1.0f - std::abs(w.dayTime - 0.5f) * 2.2f);
    int r_bg = (int)(100 - night * 80);
    int g_bg = (int)(160 - night * 130);
    int b_bg = (int)(220 - night * 180);
    SDL_SetRenderDrawColor(renderer, (Uint8)r_bg, (Uint8)g_bg, (Uint8)b_bg, 255);
    SDL_RenderClear(renderer);

    // ---- Terrain tiles (camera-culled) ----
    int sc = (int)(cam.x / TILE) - 1;
    int ec = (int)((cam.x + W) / TILE) + 1;
    int sr = (int)(cam.y / TILE) - 1;
    int er = (int)((cam.y + H) / TILE) + 1;

    for (int r = sr; r <= er; r++) {
        for (int c = sc; c <= ec; c++) {
            if (r < 0 || r >= ROWS || c < 0 || c >= COLS) continue;
            uint32_t col = TERRAIN_COLORS[w.terrain[r][c]];
            auto sdlc = hexToColor(col);
            SDL_SetRenderDrawColor(renderer, sdlc.r, sdlc.g, sdlc.b, 255);
            SDL_Rect rect = {
                (int)(c * TILE - cam.x), (int)(r * TILE - cam.y),
                TILE + 1, TILE + 1
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    // ---- Items ----
    for (auto& it : w.items) {
        if (it.collected) continue;
        int sx = (int)(it.x - cam.x);
        int sy = (int)(it.y - cam.y + std::sin(it.sparkTimer + g.gameTime * 3) * 3);
        if (sx < -15 || sx > W + 15 || sy < -15 || sy > H + 15) continue;

        uint32_t color;
        switch (it.type) {
            case ItemType::Coin:         color = 0xFFD700; break;
            case ItemType::HealthPotion: color = 0xFF4444; break;
            case ItemType::ManaGem:      color = 0x4488FF; break;
            case ItemType::AncientRelic: color = 0xC080FF; break;
            case ItemType::Equipment:     color = 0xFF8800; break;
            default: color = 0xFFFFFF;
        }
        auto sdlc = hexToColor(color);
        SDL_SetRenderDrawColor(renderer, sdlc.r, sdlc.g, sdlc.b, 255);
        int rad = (it.type == ItemType::Equipment) ? 7 : 5;
        fillCircle(renderer, sx, sy, rad);
    }

    // ---- Chests ----
    for (auto& c : w.chests) {
        int sx = (int)(c.x - cam.x), sy = (int)(c.y - cam.y);
        auto col = hexToColor(c.opened ? 0x6B4C2Au : 0xC4944Au);
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
        SDL_Rect rect = {sx - 10, sy - 6, 20, 14};
        SDL_RenderFillRect(renderer, &rect);
    }

    // ---- NPCs ----
    for (auto& n : w.npcs) {
        int sx = (int)(n.x - cam.x), sy = (int)(n.y - cam.y);
        auto body = hexToColor(n.color);
        SDL_SetRenderDrawColor(renderer, body.r, body.g, body.b, 255);
        fillCircle(renderer, sx, sy - 4, 9);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xE0, 0xC0, 255);
        fillCircle(renderer, sx, sy - 10, 7);
        drawText(renderer, font, n.name.c_str(), sx, sy - 22, {255, 255, 255, 255}, true);
    }

    // ---- Enemies ----
    for (auto& e : w.enemies) {
        if (e.hp <= 0) continue;
        int sx = (int)(e.x - cam.x), sy = (int)(e.y - cam.y);
        SDL_SetRenderDrawColor(renderer, 0x8B, 0x00, 0x00, 255);
        fillCircle(renderer, sx, sy, 12);
        // HP bar
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x44, 0x44, 255);
        int barW = (int)(20.0f * e.hp / e.maxHp);
        SDL_Rect bar = {sx - 10, sy - 16, barW, 4};
        SDL_RenderFillRect(renderer, &bar);
    }

    // ---- Boss ----
    if (w.boss.hp > 0) {
        int sx = (int)(w.boss.x - cam.x), sy = (int)(w.boss.y - cam.y);
        SDL_SetRenderDrawColor(renderer, 0x4A, 0x00, 0x80, 255);
        fillCircle(renderer, sx, sy, 24);
        // HP bar
        SDL_SetRenderDrawColor(renderer, 0xAA, 0x44, 0xFF, 255);
        int barW = (int)(50.0f * w.boss.hp / w.boss.maxHp);
        SDL_Rect bar = {sx - 25, sy - 30, barW, 6};
        SDL_RenderFillRect(renderer, &bar);
        // Projectiles
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0xFF, 255);
        for (auto& proj : w.boss.projectiles) {
            int px = (int)(proj.x - cam.x), py = (int)(proj.y - cam.y);
            fillCircle(renderer, px, py, 8);
        }
    }

    // ---- Player ----
    int px = (int)(p.x - cam.x), py2 = (int)(p.y - cam.y);
    bool blink = (p.invincible > 0 && (int)(g.gameTime * 20) % 2 == 0);
    if (!blink) {
        SDL_SetRenderDrawColor(renderer, 0x4A, 0x90, 0xD9, 255);
        SDL_Rect body = {px - 8, py2 - 2, 16, 16};
        SDL_RenderFillRect(renderer, &body);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xE4, 0xC4, 255);
        fillCircle(renderer, px, py2 - 8, 8);
    }

    // ---- Particles ----
    for (auto& pt : g.particles) {
        auto col = hexToColor(pt.color);
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b,
                               (Uint8)(255 * pt.life / pt.maxLife));
        int radius = (int)pt.size;
        if (radius < 1) radius = 1;
        fillCircle(renderer, (int)(pt.x - cam.x), (int)(pt.y - cam.y), radius);
    }

    // ---- Floating texts ----
    for (auto& ft : g.floatingTexts) {
        auto col = hexToColor(ft.color);
        col.a = (Uint8)(255 * ft.life / ft.maxLife);
        drawText(renderer, font, ft.text.c_str(),
                 (int)(ft.x - cam.x), (int)(ft.y - cam.y), col, true);
    }

    // ---- Weather (rain/snow) ----
    if (w.weather == "rain") {
        SDL_SetRenderDrawColor(renderer, 180, 200, 255, 100);
        for (auto& rd : g.rains) {
            SDL_RenderDrawLine(renderer,
                               (int)(rd.x - cam.x), (int)(rd.y - cam.y),
                               (int)(rd.x - cam.x), (int)(rd.y - cam.y) + 7);
        }
    }
    if (w.weather == "snow") {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
        for (auto& sf : g.snows) {
            int sx = (int)(sf.x - cam.x), sy2 = (int)(sf.y - cam.y);
            fillCircle(renderer, sx, sy2, 2);
        }
    }

    // ---- Night overlay ----
    if (night > 0.4f) {
        SDL_SetRenderDrawColor(renderer, 10, 15, 30, (Uint8)((night - 0.4f) * 1.1f * 255));
        SDL_Rect full = {0, 0, W, H};
        SDL_RenderFillRect(renderer, &full);
    }

    // ---- HUD ----
    // HP bar
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect hpBg = {10, 10, 170, 20}; SDL_RenderFillRect(renderer, &hpBg);
    SDL_SetRenderDrawColor(renderer, 0xE0, 0x40, 0x40, 255);
    SDL_Rect hpBar = {11, 11, (int)(168.0f * p.hp / p.maxHp), 18};
    SDL_RenderFillRect(renderer, &hpBar);
    char buf[64];
    snprintf(buf, sizeof(buf), "HP %d/%d", p.hp, p.maxHp);
    drawText(renderer, font, buf, 95, 12, {255, 255, 255, 255}, true);

    // Mana bar
    SDL_Rect manaBg = {10, 32, 170, 20}; SDL_RenderFillRect(renderer, &manaBg);
    SDL_SetRenderDrawColor(renderer, 0x40, 0x80, 0xE0, 255);
    SDL_Rect manaBar = {11, 33, (int)(168.0f * p.mana / p.maxMana), 18};
    SDL_RenderFillRect(renderer, &manaBar);
    snprintf(buf, sizeof(buf), "MP %d/%d", p.mana, p.maxMana);
    drawText(renderer, font, buf, 95, 34, {255, 255, 255, 255}, true);

    // Resources
    snprintf(buf, sizeof(buf), "Coins:%d Pot:%d Gem:%d Rel:%d Lv.%d",
             p.coins, p.potions, p.gems, p.relics, p.level);
    drawText(renderer, font, buf, 16, 58, {0xDD, 0xDD, 0xDD, 255});
    snprintf(buf, sizeof(buf), "EXP:%d/%d", p.exp, p.expToLevel);
    drawText(renderer, font, buf, 16, 72, {0xDD, 0xDD, 0xDD, 255});

    // ---- Mini-map ----
    int mmW = 130, mmH = 90, mmX = W - mmW - 12, mmY = 10;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect mmBg = {mmX, mmY, mmW, mmH}; SDL_RenderFillRect(renderer, &mmBg);
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            auto col = hexToColor(TERRAIN_COLORS[w.terrain[r][c]]);
            SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
            SDL_Rect tr = {mmX + (int)(c * mmW / (float)COLS),
                           mmY + (int)(r * mmH / (float)ROWS),
                           (int)(mmW / (float)COLS + 0.5f),
                           (int)(mmH / (float)ROWS + 0.5f)};
            SDL_RenderFillRect(renderer, &tr);
        }
    }
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x44, 0x44, 255);
    fillCircle(renderer,
               mmX + (int)(p.x / WORLD_W * mmW),
               mmY + (int)(p.y / WORLD_H * mmH), 3);

    // ---- Quest HUD ----
    SDL_Rect qBg = {10, H - 70, 200, 60};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &qBg);
    drawText(renderer, font, "Quest", 16, H - 56, {0xFF, 0xD7, 0x00, 255});

    const char* qText = "";
    switch (w.storyProgress) {
        case 0: qText = "Talk to Village Chief"; break;
        case 1:
            snprintf(buf, sizeof(buf), "Coins(%d/5) Enemies(%d/3)", p.coins, p.defeatedEnemies);
            drawText(renderer, font, buf, 16, H - 42, {0xCC, 0xCC, 0xCC, 255});
            return;
        case 2:
            snprintf(buf, sizeof(buf), "Relics(%d/3)", p.relics);
            drawText(renderer, font, buf, 16, H - 42, {0xCC, 0xCC, 0xCC, 255});
            return;
        case 3: drawText(renderer, font, "Defeat Boss", 16, H - 42, {0xCC, 0xCC, 0xCC, 255}); return;
        case 4: drawText(renderer, font, "Complete!", 16, H - 42, {0xCC, 0xCC, 0xCC, 255}); return;
    }

    // ---- Skill hints ----
    SDL_Rect sBg = {W - 180, H - 35, 170, 28};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &sBg);
    drawText(renderer, font, "[1]Slash [2]Heal [3]Fireball", W - 95, H - 25, {255, 255, 255, 255}, true);

    // ---- Inventory panel ----
    if (g.showInventory) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 230);
        SDL_Rect pan = {40, 40, W - 80, H - 80}; SDL_RenderFillRect(renderer, &pan);
        SDL_SetRenderDrawColor(renderer, 0xE2, 0xB0, 0x4A, 255);
        SDL_RenderDrawRect(renderer, &pan);
        drawText(renderer, font, "Bag ([I] to close)", W / 2, 55, {0xFF, 0xD7, 0x00, 255}, true);
        if (p.inventory.empty()) {
            drawText(renderer, font, "(empty)", W / 2, 115, {255, 255, 255, 255}, true);
        } else {
            int y = 115;
            for (int i = 0; i < (int)p.inventory.size(); i++) {
                auto& eq = p.inventory[i];
                snprintf(buf, sizeof(buf), "[%d] %s ATK:%d DEF:%d HP:%d %s",
                         i + 1, eq.name.c_str(), eq.atk, eq.def, eq.hp,
                         QUALITY_NAMES[(int)eq.quality]);
                auto qc = hexToColor(QUALITY_COLORS[(int)eq.quality]);
                drawText(renderer, font, buf, 60, y, qc);
                y += 25;
                if (y > H - 100) break;
            }
        }
    }

    // ---- Shop panel ----
    if (w.gameState == GameState::Shop) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 230);
        SDL_Rect pan = {80, 60, W - 160, H - 120}; SDL_RenderFillRect(renderer, &pan);
        SDL_SetRenderDrawColor(renderer, 0xE2, 0xB0, 0x4A, 255);
        SDL_RenderDrawRect(renderer, &pan);
        drawText(renderer, font, "Traveling Merchant", W / 2, 75, {0xFF, 0xD7, 0x00, 255}, true);
        drawText(renderer, font, "Press number key to buy | [Esc] to close", W / 2, 95, {255, 255, 255, 255}, true);
        if (g.shopItems.empty()) {
            drawText(renderer, font, "(Sold out)", W / 2, 140, {255, 255, 255, 255}, true);
        } else {
            int y = 140;
            for (int i = 0; i < (int)g.shopItems.size(); i++) {
                auto& eq = g.shopItems[i];
                int price = 10 + p.level * 8;
                auto qc = hexToColor(QUALITY_COLORS[(int)eq.quality]);
                snprintf(buf, sizeof(buf), "[%d] %s ATK:%d DEF:%d HP:%d Price:%d",
                         i + 1, eq.name.c_str(), eq.atk, eq.def, eq.hp, price);
                drawText(renderer, font, buf, 100, y, qc);
                y += 28;
            }
        }
    }

    // ---- Dialog ----
    if (w.gameState == GameState::Dialog) {
        SDL_Rect dlg = {20, H - 120, W - 40, 100};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 210);
        SDL_RenderFillRect(renderer, &dlg);
        drawText(renderer, font, g.dialogNPC.c_str(), 30, H - 100, {0xFF, 0xD7, 0x00, 255});
        if (g.dialogIdx < (int)g.dialogLines.size())
            drawText(renderer, font, g.dialogLines[g.dialogIdx].c_str(), 30, H - 75, {255, 255, 255, 255});
        drawText(renderer, font, "[E] Continue", W - 80, H - 38, {255, 255, 255, 255});
    }

    // ---- Big map ----
    if (g.showBigMap) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 210);
        SDL_Rect full = {0, 0, W, H}; SDL_RenderFillRect(renderer, &full);
        drawText(renderer, font, "World Map [M] to close", W / 2, 20, {255, 255, 255, 255}, true);
        int bw = (int)(W * 0.7f), bh = (int)(H * 0.7f);
        int bx = (W - bw) / 2, by = (H - bh) / 2 + 20;
        SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, 255);
        SDL_Rect mapBg = {bx, by, bw, bh}; SDL_RenderFillRect(renderer, &mapBg);
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                auto col = hexToColor(TERRAIN_COLORS[w.terrain[r][c]]);
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect tr = {
                    bx + (int)(c * bw / (float)COLS),
                    by + (int)(r * bh / (float)ROWS),
                    (int)(bw / (float)COLS + 0.5f),
                    (int)(bh / (float)ROWS + 0.5f)
                };
                SDL_RenderFillRect(renderer, &tr);
            }
        }
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x44, 0x44, 255);
        fillCircle(renderer,
                   bx + (int)(p.x / WORLD_W * bw),
                   by + (int)(p.y / WORLD_H * bh), 4);
    }
}