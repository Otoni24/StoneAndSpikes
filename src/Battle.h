#pragma once

#include <memory>
#include <optional>
#include <queue>
#include <vector>
#include "GameTypes.h"
#include "Unit.h"
#include "Utils.h"
#include "Board.h"

class Unit;

struct Attack {
    GridPosition targetPos;
};

struct Death {
    GridPosition pos;
    UnitType type{};
};

class Battle {
public:
    Battle();
    explicit Battle(std::uint32_t seed);

    void NextTurn();

    bool TryMoveActiveUnit(GridPosition newPos);
    bool TryMoveUnit(Unit& unit, GridPosition newPos, bool isPush = false);
    bool TryAttackUnit(Unit& target);
    bool TryAttackUnit(GridPosition targetPos);
    bool TryPushUnit(Unit& target);
    bool TryPushUnit(GridPosition targetPos);

    [[nodiscard]] std::vector<GridPosition> GetAdjacentEmptyPositions(const Unit& unit) const;
    [[nodiscard]] std::vector<GridPosition> GetAdjacentOpponentPositions(const Unit& unit) const;

    [[nodiscard]] bool IsAllyTurn() const;
    [[nodiscard]] const Unit& GetActiveUnit() const { return units_[turnOrder_.front()]; }
    [[nodiscard]] size_t GetActiveUnitIndex() const { return turnOrder_.front(); }
    [[nodiscard]] UnitType GetActiveUnitType() const { return GetActiveUnit().GetType(); }
    [[nodiscard]] const std::vector<Unit>& GetUnits() const { return units_; }
    [[nodiscard]] size_t GetAliveUnitCount() const { return turnOrder_.size(); }
    [[nodiscard]] const Board& GetBoard() const { return board_; }
    [[nodiscard]] const Unit* FindUnitAt(GridPosition pos) const;
    [[nodiscard]] bool IsFinished() const;

    [[nodiscard]] std::optional<Attack> TakePendingAttack();
    [[nodiscard]] std::optional<Death> TakePendingDeath();

private:
    std::optional<GridPosition> ChooseUnitSpawn(UnitType unitType);
    void CheckActionPoints(const Unit& unit);
    void CheckDeath(const Unit& unit);

    [[nodiscard]] Unit* FindMutableUnitAt(GridPosition pos);
    [[nodiscard]] Unit& GetActiveMutableUnit() { return units_[turnOrder_.front()]; }

    Random random_;
    Board board_;
    std::vector<Unit> units_;
    std::queue<std::size_t> turnOrder_;

    std::optional<Attack> pendingAttack;
    std::optional<Death> pendingDeath;
};
