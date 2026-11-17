#pragma once

#include <optional>
#include <vector>

#include "EnemyPlanner.h"

class Battle;

class EnemyAI {
public:
    void Update(Battle& battle, float deltaTime);
private:
    static bool ExecuteAction(Battle& battle, const EnemyPlanner::PlannedAction& action);

    std::vector<EnemyPlanner::PlannedAction> plan_;
    std::optional<std::size_t> plannedUnitIndex_;
    std::size_t nextAction_ = 0;
    float secondsUntilAction_ = 0.0f;
};
