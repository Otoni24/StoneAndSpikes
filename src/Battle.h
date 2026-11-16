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

enum class TurnPhase {
    SelectionPhase,
    ActionPhase
};

struct UnitDeath {
    GridPosition pos{};
    UnitType type{};
    float elapsed = 0.0f;
};

class Battle {
public:
    Battle();
    explicit Battle(std::uint32_t seed);

    void NextTurn();

    bool TryMoveActiveUnit(GridPosition newPos);
    bool TryMoveUnit(Unit& unit, GridPosition newPos, bool push = false);
    bool TryAttackUnit(Unit& target);
    bool TryAttackUnit(GridPosition targetPos);
    bool TryPushUnit(Unit& target);
    bool TryPushUnit(GridPosition targetPos);
    std::vector<UnitDeath> TakePendingDeaths();

    [[nodiscard]] std::vector<GridPosition> GetAdjacentEmptyPositions(const Unit& unit) const;
    [[nodiscard]] std::vector<GridPosition> GetAdjacentOpponentPositions(const Unit& unit) const;

    [[nodiscard]] bool IsAllyTurn() const;
    [[nodiscard]] const Unit& GetActiveUnit() const { return units_[turnOrder_.front()]; }
    [[nodiscard]] UnitType GetActiveUnitType() const { return GetActiveUnit().GetType(); }
    [[nodiscard]] bool IsFinished() const { return GetAliveUnitCount() <= 1; }
    [[nodiscard]] const std::vector<Unit>& GetUnits() const { return units_; }
    [[nodiscard]] size_t GetAliveUnitCount() const { return turnOrder_.size(); }
    [[nodiscard]] const Board& GetBoard() const { return board_; }
    [[nodiscard]] const Unit* FindUnitAt(GridPosition pos) const;

private:
    std::optional<GridPosition> ChooseUnitSpawn(UnitType unitType);
    void DamageUnit(Unit& unit, int damage);

    [[nodiscard]] Unit* FindUnitAt(GridPosition pos);

    Random random_;
    Board board_;
    std::vector<Unit> units_;
    std::queue<std::size_t> turnOrder_;
    std::vector<UnitDeath> pendingDeaths_;
};