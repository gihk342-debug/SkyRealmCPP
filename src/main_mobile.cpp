#include "types.h"
#include "world.h"
#include "game.h"
#include "renderer.h"
#include "audio.h"
#include "input.h"
#include "random.h"

#include <SDL.h>
#include <cstdio>
#include <chrono>
#include <cmath>

// ---- Virtual Joystick constants ----
constexpr float JOY_RADIUS = 35.0f;
constexpr float JOY_DEADZONE = 8.0f;

// ---- Internal main ----
static int main_mobile(int argc, char* argv[]) {
    // ---- Init SDL ----
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font*     font     = nullptr;
    int W, H;
    initRenderer(window, renderer, font, W, H);
    initAudio();

    SDL_SetWindowTitle(window, "SkyRealm - Mobile Edition");

    // ---- Init Game ----
    Game game;
    game.W = W; game.H = H;
    game.world.gameState = GameState::Start;
    game.rng.reset(42);
    recalcStats(game.player);

    // ---- Input state ----
    InputState input{};
    bool quit = false;

    // ---- Joystick state ----
    SDL_FPoint joyPos{0, 0};
    bool joyActive = false;
    int joyFingerId = -1;
    // Joystick base position (screen coords)
    float joyBaseX = 80, joyBaseY = (float)H - 80;

    // ---- Touch action buttons layout ----
    // Columns at right side: 4 columns x 3 rows (but we need 9 buttons)
    struct Btn { SDL_Rect rect; const char* label; };
    Btn buttons[10];
    int btnCount = 0;

    auto addBtn = [&](int x, int y, int w, int h, const char* lbl) {
        if (btnCount < 10) {
            buttons[btnCount++] = {{x, y, w, h}, lbl};
        }
    };

    // Layout action buttons at bottom-right
    // Grid: 4 columns, rows from bottom
    int btnW = 52, btnH = 52, gap = 8;
    int startX = W - (btnW * 3 + gap * 2) - 10;
    int startY = H - (btnH * 3 + gap * 2) - 10;

    // Row 1: skill1, skill2, skill3, attack
    for (int c = 0; c < 3; c++)
        addBtn(startX + c * (btnW + gap), startY, btnW, btnH,
               c == 0 ? "S1" : c == 1 ? "S2" : "S3");
    addBtn(startX + 3 * (btnW + gap), startY, btnW, btnH, "ATK");
    // Row 2: interact, bag, map, potion
    addBtn(startX,                     startY + btnH + gap, btnW, btnH, "E");
    addBtn(startX +     (btnW + gap),  startY + btnH + gap, btnW, btnH, "BAG");
    addBtn(startX + 2 * (btnW + gap),  startY + btnH + gap, btnW, btnH, "MAP");
    addBtn(startX + 3 * (btnW + gap),  startY + btnH + gap, btnW, btnH, "POT");
    // Row 3: gem (bottom-right)
    addBtn(startX + 3 * (btnW + gap),  startY + 2 * (btnH + gap), btnW, btnH, "GEM");

    // Button action mapping
    auto getButtonAction = [&](float mx, float my) -> std::string {
        for (int i = 0; i < btnCount; i++) {
            auto& b = buttons[i];
            if (mx >= b.rect.x && mx <= b.rect.x + b.rect.w &&
                my >= b.rect.y && my <= b.rect.y + b.rect.h) {
                return b.label;
            }
        }
        return "";
    };

    // ---- Timing ----
    auto lastTime = std::chrono::steady_clock::now();

    // ---- Main Loop ----
    while (!quit) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        if (dt <= 0) dt = 0.016f;
        if (dt > 0.2f) dt = 0.2f;
        lastTime = now;
        game.gameTime += dt;

        // ---- Process Events (Touch-aware) ----
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
                if (sym >= SDLK_1 && sym <= SDLK_4) input.shopChoice = sym - SDLK_1;
                if (sym >= SDLK_1 && sym <= SDLK_9) input.invChoice = sym - SDLK_1;
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

        // ---- Touch input ----
        case SDL_FINGERDOWN: {
                float mx = e.tfinger.x * W;
                float my = e.tfinger.y * H;

                // If touch is in left half -> joystick
                if (mx < W * 0.45f) {
                    joyActive = true;
                    joyFingerId = e.tfinger.fingerId;
                    joyBaseX = mx; joyBaseY = my;
                } else {
                    // Action button
                    auto action = getButtonAction(mx, my);
                    if (action == "ATK") input.attack = true;
                    else if (action == "E") input.interact = true;
                    else if (action == "S1") input.skillSlot = "slash";
                    else if (action == "S2") input.skillSlot = "heal";
                    else if (action == "S3") input.skillSlot = "fireball";
                    else if (action == "BAG") input.inventory = true;
                    else if (action == "MAP") input.map = true;
                    else if (action == "POT") input.potion = true;
                    else if (action == "GEM") input.gem = true;
                    // Start/victory shortcut
                    if (game.world.gameState == GameState::Start) resetGame(game.world, game.player, game.rng);
                    if (game.world.gameState == GameState::Victory) resetGame(game.world, game.player, game.rng);
                }
                break;
            }
            case SDL_FINGERMOTION: {
                if (joyActive && (int)e.tfinger.fingerId == joyFingerId) {
                    float mx = e.tfinger.x * W;
                    float my = e.tfinger.y * H;
                    float dx = mx - joyBaseX;
                    float dy = my - joyBaseY;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if (dist > JOY_RADIUS) {
                        dx = dx / dist * JOY_RADIUS;
                        dy = dy / dist * JOY_RADIUS;
                    }
                    joyPos.x = dx;
                    joyPos.y = dy;
                    input.joyX = (dist > JOY_DEADZONE) ? dx / JOY_RADIUS : 0;
                    input.joyY = (dist > JOY_DEADZONE) ? dy / JOY_RADIUS : 0;
                }
                break;
            }
            case SDL_FINGERUP: {
                if (joyActive && (int)e.tfinger.fingerId == joyFingerId) {
                    joyActive = false;
                    joyFingerId = -1;
                    joyPos.x = 0; joyPos.y = 0;
                    input.joyX = 0; input.joyY = 0;
                }
                input.attack = false;
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                float mx = (float)e.button.x, my = (float)e.button.y;
                // Left click -> treat like joystick tap or action
                if (game.world.gameState == GameState::Start) {
                    resetGame(game.world, game.player, game.rng);
                }
                if (game.world.gameState == GameState::Victory) {
                    resetGame(game.world, game.player, game.rng);
                }
                if (mx > W * 0.45f) {
                    auto action = getButtonAction(mx, my);
                    if (action == "ATK") input.attack = true;
                    else if (action == "E") input.interact = true;
                    else if (action == "S1") input.skillSlot = "slash";
                    else if (action == "S2") input.skillSlot = "heal";
                    else if (action == "S3") input.skillSlot = "fireball";
                    else if (action == "BAG") input.inventory = true;
                    else if (action == "MAP") input.map = true;
                    else if (action == "POT") input.potion = true;
                    else if (action == "GEM") input.gem = true;
                }
                break;
            }
            default:
                break;
            }
        }

        // ---- Handle game state transitions ----
        auto& gs = game.world.gameState;

        if (gs == GameState::Victory && (input.attack || input.interact)) {
            resetGame(game.world, game.player, game.rng);
        }

        // -------- Game Input Processing --------
        if (gs == GameState::Playing) {
            // Movement (keyboard + joystick)
            float ix = input.joyX, iy = input.joyY;
            if (input.up)    iy = -1;
            if (input.down)  iy = 1;
            if (input.left)  ix = -1;
            if (input.right) ix = 1;

            float mag = std::sqrt(ix * ix + iy * iy);
            if (mag > 1) { ix /= mag; iy /= mag; }

            if (mag > 0.1f) {
                if (std::abs(ix) > std::abs(iy)) {
                    game.player.facing = (ix > 0) ? 0 : 2;
                } else {
                    game.player.facing = (iy > 0) ? 1 : 3;
                }
            }

            auto& p = game.player;
            int col = (int)(p.x / TILE), row = (int)(p.y / TILE);
            float tmod = (row >= 0 && row < ROWS && col >= 0 && col < COLS) ?
                TERRAIN_SPEED[game.world.terrain[row][col]] : 1.0f;

            p.vx += (ix * p.speed * tmod + p.knockX - p.vx) * dt * 10;
            p.vy += (iy * p.speed * tmod + p.knockY - p.vy) * dt * 10;

            float nx = p.x + p.vx * dt, ny = p.y + p.vy * dt;
            if (checkPassable(game.world, nx, ny, p.size)) {
                p.x = nx; p.y = ny;
            } else {
                p.vx = 0; p.vy = 0;
            }
            p.x = std::max((float)p.size, std::min((float)(WORLD_W - p.size), p.x));
            p.y = std::max((float)p.size, std::min((float)(WORLD_H - p.size), p.y));

            if (input.attack && p.attackCd <= 0) {
                p.attackCd = 0.4f;
                meleeAttack(game);
                Sfx::sword();
                input.attack = false;
            }
            if (input.interact) {
                handleInteract(game);
                input.interact = false;
            }
            if (!input.skillSlot.empty()) {
                useSkill(game, input.skillSlot);
                Sfx::skill();
                input.skillSlot.clear();
            }
            if (input.potion && p.potions > 0) {
                p.hp = std::min(p.maxHp, p.hp + 30);
                p.potions--;
            }
            if (input.gem && p.gems > 0) {
                p.mana = std::min(p.maxMana, p.mana + 25);
                p.gems--;
            }
            if (input.inventory) {
                game.showInventory = !game.showInventory;
                gs = game.showInventory ? GameState::Inventory : GameState::Playing;
            }
            if (input.map) game.showBigMap = !game.showBigMap;
            if (input.escape) {
                game.showBigMap = false;
                game.showInventory = false;
                if (gs == GameState::Shop || gs == GameState::Inventory)
                    gs = GameState::Playing;
            }
        }
        else if (gs == GameState::Dialog) {
            game.dialogTimer += dt;
            if ((input.interact && game.dialogTimer > 0.3f) || game.dialogTimer > 3) {
                if (game.dialogIdx < (int)game.dialogLines.size() - 1) {
                    game.dialogIdx++;
                    game.dialogTimer = 0;
                } else {
                    gs = GameState::Playing;
                }
                input.interact = false;
            }
        }
        else if (gs == GameState::Shop) {
            if (input.shopChoice >= 0 && input.shopChoice < (int)game.shopItems.size()) {
                int idx = input.shopChoice;
                int price = 10 + game.player.level * 8;
                if (game.player.coins >= price) {
                    game.player.coins -= price;
                    game.player.inventory.push_back(game.shopItems[idx]);
                    game.shopItems.erase(game.shopItems.begin() + idx);
                    Sfx::equip();
                    addFloatingText(game, game.player.x, game.player.y, "Purchased!", 0xFFD700);
                }
            }
            if (input.escape) gs = GameState::Playing;
        }
        else if (gs == GameState::Inventory) {
            if (input.invChoice >= 0 && input.invChoice < (int)game.player.inventory.size()) {
                int idx = input.invChoice;
                auto eq = game.player.inventory[idx];
                int slot = eq.slot;
                if (game.player.equipment[slot].name.empty()) {
                    game.player.equipment[slot] = eq;
                    game.player.inventory.erase(game.player.inventory.begin() + idx);
                } else {
                    auto old = game.player.equipment[slot];
                    game.player.equipment[slot] = eq;
                    game.player.inventory[idx] = old;
                }
                recalcStats(game.player);
                Sfx::equip();
            }
            if (input.inventory) { game.showInventory = false; gs = GameState::Playing; }
            if (input.escape)    { game.showInventory = false; gs = GameState::Playing; }
        }

        // ---- Update ----
        if (gs == GameState::Playing) {
            update(game, dt);
        }

        // ---- Render ----
        renderGame(renderer, font, game);

        // ---- Draw virtual joystick & action buttons ----
        // Joystick base
        SDL_Rect jsBase = {(int)(joyBaseX - 50), (int)(joyBaseY - 50), 100, 100};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 38);
        SDL_RenderDrawRect(renderer, &jsBase);
        // Draw circle outline
        for (int i = 0; i < 360; i += 10) {
            float rad = 3.14159f / 180.0f;
            int cx = (int)(joyBaseX + std::cos(i * rad) * 50);
            int cy = (int)(joyBaseY + std::sin(i * rad) * 50);
            SDL_RenderDrawPoint(renderer, cx, cy);
        }
        // Thumb
        int thumbX = (int)(joyBaseX + joyPos.x);
        int thumbY = (int)(joyBaseY + joyPos.y);
        SDL_Rect thumb = {thumbX - 22, thumbY - 22, 44, 44};
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 90);
        SDL_RenderFillRect(renderer, &thumb);
        // Thumb circle
        for (int i = 0; i < 360; i += 15) {
            float rad = 3.14159f / 180.0f;
            int cx = thumbX + (int)(std::cos(i * rad) * 22);
            int cy = thumbY + (int)(std::sin(i * rad) * 22);
            SDL_RenderDrawPoint(renderer, cx, cy);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        // ---- Draw action buttons ----
        for (int i = 0; i < btnCount; i++) {
            auto& b = buttons[i];
            SDL_SetRenderDrawColor(renderer, 20, 15, 10, 200);
            SDL_RenderFillRect(renderer, &b.rect);
            SDL_SetRenderDrawColor(renderer, 200, 160, 80, 140);
            SDL_RenderDrawRect(renderer, &b.rect);
            drawText(renderer, font, b.label, b.rect.x + b.rect.w / 2,
                     b.rect.y + b.rect.h / 2 - 8, {0xE2, 0xC0, 0x88, 255}, true);
        }

        SDL_RenderPresent(renderer);

        // ---- Reset per-frame inputs ----
        resetInput(input);

        SDL_Delay(1);
    }

    // ---- Cleanup ----
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

// ---- Platform entry points ----
#if defined(PLATFORM_MOBILE) || defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
int SDL_main(int argc, char* argv[]) {
    return main_mobile(argc, argv);
}
#endif