#include "Unit.h"

Unit::Unit(const UnitType unitType, const GridPosition pos, const int hp, const int attack, const int actionPoints)
    : type(unitType), position(pos), maxHealth(hp), health(hp), attackPower(attack), actionPoints(actionPoints), maxActionPoints(actionPoints) {
}

void Unit::TakeDamage(const int damage) {
    health -= damage;
    if (health < 0) health = 0;
}

void Unit::SetPosition(const GridPosition pos) {
    position = pos;
}

void Unit::SetActionPoints(const int newActionPoints) {
    actionPoints = newActionPoints;
    if (actionPoints < 0) actionPoints = 0;
}
