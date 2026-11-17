#include "EnemyAI.h"

#include "Battle.h"

void EnemyAI::Update(Battle& battle, const float deltaTime) {
    constexpr float turnStartDelay = 0.5f;
    constexpr float actionDelay = 0.7f;

    if (battle.IsFinished() || battle.IsAllyTurn()) {
        plannedUnitIndex_.reset();
        return;
    }

    const std::size_t activeIndex = battle.GetActiveUnitIndex();

    if (plannedUnitIndex_ != activeIndex) {
        const EnemyPlanner::PlanningState initial{
            .units = battle.GetUnits(),
            .activeUnitIndex = activeIndex
        };

        plan_ = EnemyPlanner::FindBestPlan(initial, battle.GetBoard());
        plannedUnitIndex_ = activeIndex;
        nextAction_ = 0;
        secondsUntilAction_ = turnStartDelay;
        return;
    }

    secondsUntilAction_ -= deltaTime;
    if (secondsUntilAction_ > 0.0f) {
        return;
    }

    if (nextAction_ == plan_.size()) {
        plannedUnitIndex_.reset();
        battle.NextTurn();
        return;
    }

    const bool executed = ExecuteAction(battle, plan_[nextAction_]);

    if (battle.IsFinished() || battle.GetActiveUnitIndex() != activeIndex) {
        plannedUnitIndex_.reset();
        return;
    }

    if (!executed) {
        plannedUnitIndex_.reset();
        battle.NextTurn();
        return;
    }

    ++nextAction_;
    secondsUntilAction_ = actionDelay;
}

bool EnemyAI::ExecuteAction(Battle &battle, const EnemyPlanner::PlannedAction &action) {
    using Type = EnemyPlanner::PlannedAction::Type;

    switch (action.type) {
        case Type::Move:   return battle.TryMoveActiveUnit(action.position);
        case Type::Attack: return battle.TryAttackUnit(action.position);
        case Type::Push:   return battle.TryPushUnit(action.position);
    }
    return false;
}
