#include "Battle.h"

#include <iostream>
#include <stdexcept>
#include <utility>

#include "Unit.h"
#include "Board.h"
#include "CombatRules.h"
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

    random_.Shuffle(units_);

    for (std::size_t i = 0; i < units_.size(); ++i) {
        turnOrder_.push(i);
    }
}

void Battle::NextTurn() {
    if (IsFinished()) {
        return;
    }

    if (turnOrder_.empty()) {
        throw std::runtime_error("Turn order is empty. No units to end a turn.");
    }

    // End the current turn, keeping only living units in the rotation.
    const std::size_t currentUnitIndex = turnOrder_.front();
    turnOrder_.pop();
    if (units_[currentUnitIndex].IsAlive()) {
        turnOrder_.push(currentUnitIndex);
    }

    while (!turnOrder_.empty() && !units_[turnOrder_.front()].IsAlive()) {
        turnOrder_.pop();
    }
    if (turnOrder_.empty()) {
        throw std::runtime_error("No living units remain to take a turn.");
    }

    Unit& newUnit = units_[turnOrder_.front()];
    newUnit.SetActionPoints(newUnit.GetMaxActionPoints());
    std::cout << "Unit at position (" << newUnit.GetPosition().x << ", " << newUnit.GetPosition().y << ") is taking its turn.\n";
}

bool Battle::TryMoveActiveUnit(const GridPosition newPos) {
    if (IsFinished()) {
        return false;
    }
    return TryMoveUnit(units_[turnOrder_.front()], newPos);
}

bool Battle::TryMoveUnit(Unit &unit, const GridPosition newPos, const bool isPush) {
    if (IsFinished()) {
        return false;
    }

    if (!CombatRules::TryApplyMovement(unit, newPos, isPush, units_, board_)) {
        return false;
    }

    CheckDeath(unit);

    return true;
}

bool Battle::TryAttackUnit(Unit &target) {
    if (IsFinished()) {
        return false;
    }

    Unit& attacker = GetActiveMutableUnit();
    if (!CombatRules::TryApplyAttack(attacker, target)) {
        return false;
    }

    pendingAttack = Attack{.targetPos = target.GetPosition()};

    CheckDeath(target);

    return true;
}

bool Battle::TryAttackUnit(const GridPosition targetPos) {
    Unit* target = FindMutableUnitAt(targetPos);
    return target != nullptr && TryAttackUnit(*target);
}

bool Battle::TryPushUnit(Unit &target) {
    if (IsFinished()) {
        return false;
    }
    if (!CombatRules::TryApplyPush(GetActiveMutableUnit(), target, units_, board_)) {
        return false;
    }

    CheckDeath(target);

    return true;
}

bool Battle::TryPushUnit(const GridPosition targetPos) {
    Unit* target = FindMutableUnitAt(targetPos);
    return target != nullptr && TryPushUnit(*target);
}

std::vector<GridPosition> Battle::GetAdjacentEmptyPositions(const Unit& unit) const {
    std::vector<GridPosition> adjacentPositions = board_.GetAdjacentNonWallPositions(unit.GetPosition());
    std::erase_if(adjacentPositions,[this](const GridPosition& p) {
        return FindUnitAt(p) != nullptr;
    });
    return adjacentPositions;
}

std::vector<GridPosition> Battle::GetAdjacentOpponentPositions(const Unit& unit) const {
    std::vector<GridPosition> adjacentPositions = board_.GetAdjacentNonWallPositions(unit.GetPosition());
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

std::vector<std::size_t> Battle::GetTurnOrder() const {
    auto remaining = turnOrder_;
    std::vector<std::size_t> order;
    order.reserve(remaining.size());

    while (!remaining.empty()) {
        const std::size_t index = remaining.front();
        remaining.pop();

        if (units_[index].IsAlive()) {
            order.push_back(index);
        }
    }

    return order;
}

const Unit* Battle::FindUnitAt(const GridPosition pos) const {
    for (const auto &unit : units_) {
        if (unit.IsAlive() && unit.GetPosition() == pos) {
            return &unit;
        }
    }
    return nullptr;
}

bool Battle::IsFinished() const {
    bool hasAlly = false;
    bool hasEnemy = false;
    for (const auto &unit : units_) {
        if (!unit.IsAlive()) {
            continue;
        }
        if (unit.GetType() == UnitType::Ally) {
            hasAlly = true;
        } else if (unit.GetType() == UnitType::Enemy) {
            hasEnemy = true;
        }
        if (hasAlly && hasEnemy) {
            return false;
        }
    }
    return true;
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

void Battle::CheckDeath(const Unit &unit) {
    if (!unit.IsAlive()) {
        pendingDeath = Death{.pos = unit.GetPosition(), .type = unit.GetType()};
        std::cout << "Unit at position (" << unit.GetPosition().x << ", " << unit.GetPosition().y << ") has been defeated.\n";
        if (&unit == &GetActiveUnit()) {
            NextTurn();
        }
    }
}

Unit* Battle::FindMutableUnitAt(const GridPosition pos) {
    return const_cast<Unit*>(std::as_const(*this).FindUnitAt(pos));
}
