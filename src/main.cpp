#include <raylib.h>

#include "Battle.h"
#include "BattleHud.h"

#include <exception>
#include <iostream>

int main() {
    InitWindow(BattleHud::Width, BattleHud::Height, "Stone & Spikes");
    SetTargetFPS(60);

    int exitCode = 0;

    try {
        Battle battle{};
        BattleHud hud{};

        while (!WindowShouldClose()) {
            hud.Update(battle);
            if (!battle.IsFinished()) {
                switch (battle.GetActiveUnitType()) {
                    case UnitType::Ally:
                        break;

                    case UnitType::Enemy:
                        // enemyAI.Update(battle);
                        break;
                }
            }

            const bool playerTurn = !battle.IsFinished()
                && battle.GetActiveUnitType() == UnitType::Ally;

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
