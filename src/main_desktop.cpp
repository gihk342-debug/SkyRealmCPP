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

int main(int argc, char* argv[]) {
    // ---- Init SDL ----
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font*     font     = nullptr;
    int W, H;
    initRenderer(window, renderer, font, W, H);
    initAudio();

    // ---- Init Game ----
    Game game;
    game.W = W; game.H = H;
    game.world.gameState = GameState::Start;
    // World will be generated on first start/reset
    game.rng.reset(42);
    recalcStats(game.player);

    // ---- Input state ----
    InputState input{};
    bool quit = false;

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

        // ---- Process Events ----
        processInput(input, quit);

        // ---- Handle game state transitions ----
        auto& gs = game.world.gameState;

        // Start screen
        if (gs == GameState::Start) {
            if (input.attack || input.interact || input.up || input.down ||
                input.left || input.right || input.shopChoice >= 0 || input.invChoice >= 0) {
                resetGame(game.world, game.player, game.rng);
                game.camera.x = game.player.x - game.W / 2.0f;
                game.camera.y = game.player.y - game.H / 2.0f;
                gs = GameState::Playing;
            }
        }

        // Victory screen
        if (gs == GameState::Victory) {
            if (input.attack || input.interact || input.up || input.down ||
                input.left || input.right) {
                resetGame(game.world, game.player, game.rng);
            }
        }

        // -------- Game Input Processing (only when Playing) --------
        if (gs == GameState::Playing) {
            // Movement
            float ix = 0, iy = 0;
            if (input.up)    iy = -1;
            if (input.down)  iy = 1;
            if (input.left)  ix = -1;
            if (input.right) ix = 1;

            // Normalize
            float mag = std::sqrt(ix * ix + iy * iy);
            if (mag > 1) { ix /= mag; iy /= mag; }

            // Update facing
            if (mag > 0.1f) {
                if (std::abs(ix) > std::abs(iy)) {
                    game.player.facing = (ix > 0) ? 0 : 2;
                } else {
                    game.player.facing = (iy > 0) ? 1 : 3;
                }
            }

            // Terrain speed
            auto& p = game.player;
            int col = (int)(p.x / TILE), row = (int)(p.y / TILE);
            float tmod = (row >= 0 && row < ROWS && col >= 0 && col < COLS) ?
                TERRAIN_SPEED[game.world.terrain[row][col]] : 1.0f;

            // Movement velocity
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

            // Attack
            if (input.attack && p.attackCd <= 0) {
                p.attackCd = 0.4f;
                meleeAttack(game);
                Sfx::sword();
                input.attack = false;
            }

            // Interact
            if (input.interact) {
                handleInteract(game);
                input.interact = false;
            }

            // Skills
            if (!input.skillSlot.empty()) {
                useSkill(game, input.skillSlot);
                Sfx::skill();
                input.skillSlot.clear();
            }

            // Potion / Gem
            if (input.potion && p.potions > 0) {
                p.hp = std::min(p.maxHp, p.hp + 30);
                p.potions--;
            }
            if (input.gem && p.gems > 0) {
                p.mana = std::min(p.maxMana, p.mana + 25);
                p.gems--;
            }

            // Inventory toggle
            if (input.inventory) {
                game.showInventory = !game.showInventory;
                if (game.showInventory)
                    game.world.gameState = GameState::Inventory;
                else if (game.world.gameState == GameState::Inventory)
                    game.world.gameState = GameState::Playing;
            }

            // Map toggle
            if (input.map) game.showBigMap = !game.showBigMap;
            if (input.escape) {
                game.showBigMap = false;
                game.showInventory = false;
            }
        }
        else if (gs == GameState::Dialog) {
            game.dialogTimer += dt;
            if (input.interact && game.dialogTimer > 0.3f) {
                if (game.dialogIdx < (int)game.dialogLines.size() - 1) {
                    game.dialogIdx++;
                    game.dialogTimer = 0;
                } else {
                    gs = GameState::Playing;
                }
                input.interact = false;
            }
            // Also advance after timeout
            if (game.dialogTimer > 3) {
                if (game.dialogIdx < (int)game.dialogLines.size() - 1) {
                    game.dialogIdx++;
                    game.dialogTimer = 0;
                } else {
                    gs = GameState::Playing;
                }
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
                } else {
                    addFloatingText(game, game.player.x, game.player.y, "Not enough coins!", 0xFF4444);
                }
            }
            if (input.escape) {
                gs = GameState::Playing;
            }
        }
        else if (gs == GameState::Inventory) {
            if (input.invChoice >= 0 && input.invChoice < (int)game.player.inventory.size()) {
                int idx = input.invChoice;
                auto eq = game.player.inventory[idx];
                int slot = eq.slot;
                if (game.player.equipment[slot].name.empty()) {
                    // Empty slot -- equip directly
                    game.player.equipment[slot] = eq;
                    game.player.inventory.erase(game.player.inventory.begin() + idx);
                } else {
                    // Swap
                    auto old = game.player.equipment[slot];
                    game.player.equipment[slot] = eq;
                    game.player.inventory[idx] = old;
                }
                recalcStats(game.player);
                Sfx::equip();
            }
            if (input.inventory) {
                game.showInventory = false;
                gs = GameState::Playing;
            }
            if (input.escape) {
                game.showInventory = false;
                gs = GameState::Playing;
            }
        }

        // ---- Update Game (if not paused) ----
        if (gs == GameState::Playing) {
            update(game, dt);
        }

        // ---- Render ----
        renderGame(renderer, font, game);
        SDL_RenderPresent(renderer);

        // ---- Reset per-frame inputs ----
        resetInput(input);

        // ---- Small sleep to avoid 100% CPU ----
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