#include "Battle.h"

#include <iostream>
#include <ostream>
#include <utility>

#include "Unit.h"
#include "Board.h"
#include "GameConfig.h"

Battle::Battle()
    : Battle{std::random_device{}()} {}

Battle::Battle(const std::uint32_t seed)
    : random_(seed), board_{random_} {
    std::cout << "Battle seed: " << seed << '\n';

    for (int i = 0; i < GameConfig::NumberOfAllies; ++i) {
        const auto spawnPos = ChooseUnitSpawn(UnitType::Ally);
        if (!spawnPos) {
            throw std::runtime_error("Failed to find a valid spawn position for an ally unit.");
        }
        units_.emplace_back(UnitType::Ally, *spawnPos, GameConfig::AllyMaxHealth, GameConfig::AllyAttackPower, GameConfig::AllyActionPoints);
    }

    for (int i = 0; i < GameConfig::NumberOfEnemies; ++i) {
        const auto spawnPos = ChooseUnitSpawn(UnitType::Enemy);
        if (!spawnPos) {
            throw std::runtime_error("Failed to find a valid spawn position for an enemy unit.");
        }
        units_.emplace_back(UnitType::Enemy, *spawnPos, GameConfig::EnemyMaxHealth, GameConfig::EnemyAttackPower, GameConfig::EnemyActionPoints);
    }

    //TODO: random.Shuffle(units);

    for (std::size_t i = 0; i < units_.size(); ++i) {
        turnOrder_.push(i);
    }
}

void Battle::NextTurn() {
    if (turnOrder_.empty()) {
        throw std::runtime_error("Turn order is empty. No units to end a turn.");
    }

    const Unit& currentUnit = units_[turnOrder_.front()];

    if (!currentUnit.IsAlive()) {
        turnOrder_.pop();
        NextTurn(); // Skip dead units
    }

    std::cout << "Unit at position (" << currentUnit.GetPosition().x << ", " << currentUnit.GetPosition().y << ") is taking its turn.\n";

    const std::size_t currentUnitIndex = turnOrder_.front();
    turnOrder_.pop();
    turnOrder_.push(currentUnitIndex);
    Unit& newUnit = units_[turnOrder_.front()];
    newUnit.SetActionPoints(newUnit.GetMaxActionPoints());
}

bool Battle::TryMoveActiveUnit(const GridPosition newPos) {
    return TryMoveUnit(units_[turnOrder_.front()], newPos);
}

bool Battle::TryMoveUnit(Unit &unit, const GridPosition newPos, const bool push) {
    if (!board_.IsInsideBounds(newPos)) {
        return false;
    }

    if (board_.IsWall(newPos)) {
        return false;
    }

    if (FindUnitAt(newPos) != nullptr) {
        return false;
    }

    unit.SetPosition(newPos);

    if (board_.IsSpike(newPos)) {
        DamageUnit(unit, GameConfig::SpikesDamage);
    }

    if (push) {
        return true;
    }

    unit.SetActionPoints(unit.GetActionPoints() - 1);
    if (unit.GetActionPoints() == 0) {
        //TODO: NextTurn();
    }
    return true;
}

bool Battle::TryAttackUnit(Unit &target) {
    const Unit& attacker = GetActiveUnit();
    if (attacker.GetType() == target.GetType()) {
        return false;
    }

    if (!Board::ArePositionsAdjacent(attacker.GetPosition(), target.GetPosition())) {
        return false;
    }

    pendingAttack = Attack{.targetPos = target.GetPosition()};
    DamageUnit(target, attacker.GetAttackPower());
    if (!target.IsAlive()) {
        std::cout << "Unit at position (" << target.GetPosition().x << ", " << target.GetPosition().y << ") has been defeated.\n";
    }
    return true;
}

bool Battle::TryAttackUnit(const GridPosition targetPos) {
    Unit* target = FindUnitAt(targetPos);
    return target != nullptr && TryAttackUnit(*target);
}

bool Battle::TryPushUnit(Unit &target) {
    const Unit& attacker = GetActiveUnit();
    if (attacker.GetType() == target.GetType()) {
        return false;
    }

    if (!Board::ArePositionsAdjacent(attacker.GetPosition(), target.GetPosition())) {
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

    return TryMoveUnit(target, destination);
}

bool Battle::TryPushUnit(const GridPosition targetPos) {
    Unit* target = FindUnitAt(targetPos);
    return target != nullptr && TryPushUnit(*target);
}

std::vector<GridPosition> Battle::GetAdjacentEmptyPositions(const Unit& unit) const {
    std::vector<GridPosition> adjacentPositions = board_.GetAdjacentFloorPositions(unit.GetPosition());
    std::erase_if(adjacentPositions,[this](const GridPosition& p) {
        return FindUnitAt(p) != nullptr;
    });
    return adjacentPositions;
}

std::vector<GridPosition> Battle::GetAdjacentOpponentPositions(const Unit& unit) const {
    std::vector<GridPosition> adjacentPositions = board_.GetAdjacentFloorPositions(unit.GetPosition());
    std::erase_if(adjacentPositions,[this,unit](const GridPosition& p) {
        const Unit* opponentUnit = FindUnitAt(p);
        return opponentUnit == nullptr || opponentUnit->GetType() == unit.GetType();
    });
    return adjacentPositions;
}

bool Battle::IsAllyTurn() const {
    if (turnOrder_.empty()) {
        return false;
    }
    return units_[turnOrder_.front()].GetType() == UnitType::Ally;
}

const Unit* Battle::FindUnitAt(const GridPosition pos) const {
    for (const auto &unit : units_) {
        if (unit.IsAlive() && unit.GetPosition() == pos) {
            return &unit;
        }
    }
    return nullptr;
}

std::optional<Attack> Battle::TakePendingAttack() {
    const std::optional<Attack> attack = pendingAttack;
    pendingAttack.reset();
    return attack;
}

std::optional<Death> Battle::TakePendingDeath() {
    const std::optional<Death> death = pendingDeath;
    pendingDeath.reset();
    return death;
}

std::optional<GridPosition> Battle::ChooseUnitSpawn(const UnitType unitType) {
    const UnitType opposingType = (unitType == UnitType::Ally) ? UnitType::Enemy : UnitType::Ally;
    std::vector<GridPosition> candidates;
    std::vector<int> weights;

    for (int y = 0; y < GameConfig::BoardHeight; ++y) {
        for (int x = 0; x < GameConfig::BoardWidth; ++x) {
            const GridPosition pos{.x = x, .y = y};

            if (!board_.IsFloor(pos) || FindUnitAt(pos) != nullptr) {
                continue;
            }

            std::optional<int> nearestDistance;
            for (const Unit& unit : units_) {
                if (unit.GetType() != opposingType) {
                    continue;
                }

                const GridPosition enemy = unit.GetPosition();
                const int distance = Board::GetDistanceBetweenPositions(pos, enemy);

                if (!nearestDistance || distance < *nearestDistance) {
                    nearestDistance = distance;
                }
            }

            const int d = nearestDistance.value_or(1);
            candidates.push_back(pos);
            weights.push_back(d * d);
        }

    }

    if (candidates.empty()) {
        return std::nullopt;
    }

    return candidates[random_.WeightedIndex(weights)];
}

void Battle::DamageUnit(Unit &unit, const int damage) {
    const bool wasAlive = unit.IsAlive();
    unit.TakeDamage(damage);
    if (wasAlive && !unit.IsAlive()) {
        pendingDeath = Death{.pos = unit.GetPosition(), .type = unit.GetType()};
    }
}

Unit* Battle::FindUnitAt(const GridPosition pos) {
    return const_cast<Unit*>(std::as_const(*this).FindUnitAt(pos));
}
