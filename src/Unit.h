#pragma once

#include "GameTypes.h"

enum class UnitType {
    Ally,
    Enemy
};

class Unit {
public:
    Unit(UnitType unitType, GridPosition pos, int hp, int attack, int actionPoints);
    void TakeDamage(int damage);
    void SetPosition(GridPosition pos);
    void SetHealth(int newHealth);
    void SetActionPoints(int newActionPoints);

    [[nodiscard]] UnitType GetType() const { return type; }
    [[nodiscard]] GridPosition GetPosition() const { return position; }
    [[nodiscard]] bool IsAlive() const { return health > 0; }
    [[nodiscard]] int GetHealth() const { return health; }
    [[nodiscard]] int GetMaxHealth() const { return maxHealth; }
    [[nodiscard]] int GetAttackPower() const { return attackPower; }
    [[nodiscard]] int GetActionPoints() const { return actionPoints; }
    [[nodiscard]] int GetMaxActionPoints() const { return maxActionPoints; }

private:
    UnitType type{};
    GridPosition position{};
    int maxHealth{};
    int health{};
    int attackPower{};
    int actionPoints{};
    int maxActionPoints{};
};