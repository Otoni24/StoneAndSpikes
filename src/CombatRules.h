#pragma once
#include <span>

class Board;
struct GridPosition;
class Unit;

namespace CombatRules {
    bool TryApplyAttack(Unit& attacker, Unit& target);
    bool TryApplyMovement(Unit& unit, GridPosition pos, bool isPush, std::span<const Unit> units, const Board& board);
    bool TryApplyPush(Unit& attacker, Unit& target, std::span<const Unit> units, const Board& board);

    bool IsOccupied(std::span<const Unit> units, GridPosition pos);
}