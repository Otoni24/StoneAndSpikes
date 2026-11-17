#include "EnemyPlanner.h"

#include <algorithm>
#include <array>
#include <queue>
#include <tuple>
#include <utility>

#include "Board.h"
#include "CombatRules.h"
#include "GameConfig.h"

namespace {
    constexpr int Unreachable = GameConfig::BoardWidth * GameConfig::BoardHeight;
    constexpr GridPosition Directions[]{{.x = 1, .y = 0}, {.x = -1, .y = 0}, {.x = 0, .y = 1}, {.x = 0, .y = -1}};

    int StepsToAttack(const EnemyPlanner::PlanningState& state, const Board& board) {
        const Unit& actor = state.units[state.activeUnitIndex];
        if (!actor.IsAlive()) {
            return Unreachable;
        }

        const auto canAttackFrom = [&](GridPosition pos) {
            return std::ranges::any_of(state.units, [&](const Unit& unit) {
                return unit.IsAlive() && unit.GetType() != actor.GetType()
                    && Board::ArePositionsAdjacent(pos, unit.GetPosition());
            });
        };

        std::array<std::array<bool, GameConfig::BoardWidth>, GameConfig::BoardHeight> visited{};
        std::queue<std::pair<GridPosition, int>> pending;

        const GridPosition start = actor.GetPosition();
        visited[start.y][start.x] = true;
        pending.emplace(start, 0);

        while (!pending.empty()) {
            const auto [pos, steps] = pending.front();
            pending.pop();

            if (canAttackFrom(pos)) {
                return steps;
            }

            for (const GridPosition direction : Directions) {
                const GridPosition next{.x = pos.x + direction.x, .y = pos.y + direction.y};

                if (!Board::IsInsideBounds(next) || visited[next.y][next.x]
                    || !board.IsFloor(next) || CombatRules::IsOccupied(state.units, next)) {
                    continue;
                }

                visited[next.y][next.x] = true;
                pending.emplace(next, steps + 1);
            }
        }

        return Unreachable;
    }

}

namespace EnemyPlanner {
    std::vector<Successor> GenerateSuccessors(const PlanningState& state, const Board& board) {
        std::vector<Successor> successors;
        const auto activeIndex = state.activeUnitIndex;
        const Unit& actor = state.units[activeIndex];

        if (!actor.IsAlive() || actor.GetActionPoints() <= 0) {
            return successors;
        }

        const GridPosition from = actor.GetPosition();
        constexpr GridPosition directions[] {
            {.x = 1, .y = 0},
            {.x = -1, .y = 0},
            {.x = 0, .y = 1},
            {.x = 0, .y = -1}
        };

        for (const GridPosition direction : directions) {
            const GridPosition destination{from.x + direction.x, from.y + direction.y};
            auto afterMove = state;

            if (CombatRules::TryApplyMovement(afterMove.units[activeIndex], destination, false, afterMove.units, board)) {
                successors.push_back({
                    .action = {
                        .type = PlannedAction::Type::Move,
                        .position = destination},
                    .state = std::move(afterMove)});
            }
        }

        for (std::size_t i = 0; i < state.units.size(); ++i) {
            const Unit& target = state.units[i];
            if (!target.IsAlive() || target.GetType() == actor.GetType()) {
                continue;
            }

            const GridPosition targetPos = target.GetPosition();

            auto afterAttack = state;
            if (CombatRules::TryApplyAttack(afterAttack.units[activeIndex], afterAttack.units[i])) {
                successors.push_back({
                    .action = {
                        .type = PlannedAction::Type::Attack,
                        .position = targetPos},
                    .state = std::move(afterAttack)});
            }

            auto afterPush = state;
            if (CombatRules::TryApplyPush(afterPush.units[activeIndex], afterPush.units[i],
            afterPush.units, board)) {
                successors.push_back({
                    .action = {
                        .type = PlannedAction::Type::Push,
                        .position = targetPos},
                    .state = std::move(afterPush)});
            }
        }
        return successors;
    }

    [[nodiscard]] static auto EvaluateState(const PlanningState &initial, const PlanningState &state, const Board &board) {
        int knightsAlive = 0;
        int knightsKilled = 0;
        int damageDealt = 0;

        for (std::size_t i = 0; i < state.units.size(); ++i) {
            const Unit& before = initial.units[i];
            const Unit& after = state.units[i];

            if (before.GetType() != UnitType::Ally) {
                continue;
            }

            knightsAlive += after.IsAlive();

            if (before.IsAlive()) {
                knightsKilled += !after.IsAlive();
                damageDealt += before.GetHealth() - after.GetHealth();
            }
        }

        const auto activeIndex = state.activeUnitIndex;
        const Unit& activeEnemy = state.units[activeIndex];
        const Unit& activeEnemyBefore = initial.units[activeIndex];
        const int damageTaken = activeEnemyBefore.GetHealth() - activeEnemy.GetHealth();

        const bool victory = knightsAlive == 0;
        const int distance = victory ? 0 : StepsToAttack(state, board);

        return std::tuple{victory, knightsKilled, damageDealt - damageTaken, -distance};
    }

    std::vector<PlannedAction> FindBestPlan(const PlanningState &initial, const Board &board) {
        struct Node {
            PlanningState state;
            std::vector<PlannedAction> plan;
        };

        std::queue<Node> pending;
        pending.push({.state = initial, .plan = {}});

        std::vector<PlannedAction> bestPlan;
        auto bestScore = EvaluateState(initial, initial, board);

        while (!pending.empty()) {
            auto node = std::move(pending.front());
            pending.pop();

            const auto score = EvaluateState(initial, node.state, board);
            if (score > bestScore) {
                bestScore = score;
                bestPlan = node.plan;
            }

            // Knights are dead
            if (std::get<0>(score)) {
                continue;
            }

            for (auto& successor : GenerateSuccessors(node.state, board)) {
                auto plan = node.plan;
                plan.push_back(successor.action);
                pending.push({std::move(successor.state), std::move(plan)});
            }
        }

        return bestPlan;
    }
}
