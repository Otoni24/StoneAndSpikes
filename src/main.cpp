#include <raylib.h>

#include "Battle.h"
#include "BattleHud.h"
#include "EnemyAI.h"

#include <exception>
#include <iostream>

int main() {
    InitWindow(BattleHud::Width, BattleHud::Height, "Stone & Spikes");
    SetTargetFPS(60);

    int exitCode = 0;

    try {
        Battle battle{};
        BattleHud hud{};
        EnemyAI enemyAI{};

        while (!WindowShouldClose()) {
            hud.Update(battle);
            enemyAI.Update(battle, GetFrameTime());

            BeginDrawing();
            ClearBackground(BLACK);
            hud.Draw(battle);
            EndDrawing();
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        exitCode = 1;
    }

    CloseWindow();
    return exitCode;
}
