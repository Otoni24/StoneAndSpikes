#pragma once

#pragma once

#include <cstddef>
#include <vector>

#include "GameTypes.h"
#include "Unit.h"

class Board;

namespace EnemyPlanner {

    struct PlanningState {
        std::vector<Unit> units;
        std::size_t activeUnitIndex;
    };

    struct PlannedAction {
        enum class Type { Move, Attack, Push };

        Type type{};
        GridPosition position;
    };

    struct Successor {
        PlannedAction action;
        PlanningState state;
    };

    [[nodiscard]] std::vector<Successor> GenerateSuccessors(const PlanningState& state, const Board& board);
    [[nodiscard]] static auto EvaluateState(const PlanningState& initial, const PlanningState& state, const Board& board);
    [[nodiscard]] std::vector<PlannedAction> FindBestPlan(const PlanningState& initial, const Board& board);

}
