#include "CombatRules.h"

#include <algorithm>

#include "Unit.h"
#include "Board.h"

bool CombatRules::TryApplyAttack(Unit &attacker, Unit &target) {
    if (!attacker.IsAlive() || !target.IsAlive() ||
        attacker.GetType() == target.GetType() ||
        attacker.GetActionPoints() <= 0 ||
        !Board::ArePositionsAdjacent(attacker.GetPosition(), target.GetPosition())) {
        return false;
    }

    attacker.SetActionPoints(attacker.GetActionPoints() - 1);
    target.SetHealth(target.GetHealth() - attacker.GetAttackPower());
    return true;
}

bool CombatRules::TryApplyMovement(Unit &unit, const GridPosition pos, const bool isPush, const std::span<const Unit> units, const Board &board) {
    if (!unit.IsAlive() ||
        (!isPush && unit.GetActionPoints() <= 0) ||
        !Board::IsInsideBounds(pos) ||
        !Board::ArePositionsAdjacent(unit.GetPosition(), pos) ||
        board.IsWall(pos) ||
        IsOccupied(units, pos)) {
        return false;
    }

    unit.SetPosition(pos);

    if (!isPush) {
        unit.SetActionPoints(unit.GetActionPoints() - 1);
    }

    if (board.IsSpike(pos)) {
        unit.SetHealth(unit.GetHealth() - GameConfig::SpikesDamage);
    }
    return true;
}

bool CombatRules::TryApplyPush(Unit &attacker, Unit& target, const std::span<const Unit> units, const Board &board) {
    if (attacker.GetType() == target.GetType() ||
        attacker.GetActionPoints() <= 0 ||
        !Board::ArePositionsAdjacent(attacker.GetPosition(), target.GetPosition()) ||
        !attacker.IsAlive() || !target.IsAlive()) {
        return false;
    }

    const GridPosition attackerPos = attacker.GetPosition();
    const GridPosition targetPos = target.GetPosition();

    const int dx = targetPos.x - attackerPos.x;
    const int dy = targetPos.y - attackerPos.y;

    const GridPosition destination{
        .x = targetPos.x + dx,
        .y = targetPos.y + dy
    };

    if (TryApplyMovement(target, destination, true, units, board)) {
        attacker.SetActionPoints(attacker.GetActionPoints() - 1);
        return true;
    }
    return false;
}

bool CombatRules::IsOccupied(std::span<const Unit> units, GridPosition pos) {
    return std::ranges::any_of(units, [pos](const Unit& unit) {
        return unit.IsAlive() && unit.GetPosition() == pos;
    });
}
