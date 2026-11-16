#include "PlayerController.h"
#include "Battle.h"
#include <raylib.h>

/*void PlayerController::Update(Battle& battle, const BoardView& view) {
    if (IsKeyPressed(KEY_ENTER)) {
        battle.EndTurn();
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (const auto cell = view.ScreenToGrid(GetMousePosition())) {
            // Per ora basta tentare la mossa. Poi puoi mostrare un feedback.
            (void)battle.TryMove(*cell);
        }
    }
}*/