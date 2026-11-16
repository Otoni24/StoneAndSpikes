#pragma once

namespace GameConfig {
    inline constexpr int BoardWidth = 7;
    inline constexpr int BoardHeight = 7;

    inline constexpr int AllyMaxHealth = 3;
    inline constexpr int AllyActionPoints = 2;
    inline constexpr int EnemyMaxHealth = 2;
    inline constexpr int EnemyActionPoints = 2;
    inline constexpr int AllyAttackPower = 1;
    inline constexpr int EnemyAttackPower = 1;

    inline constexpr int SpikesDamage = 2;

    inline constexpr int MinSpikeTiles = 3;
    inline constexpr int MaxSpikeTiles = 5;
    inline constexpr int MinWallTiles = 4;
    inline constexpr int MaxWallTiles = 8;

    inline constexpr int NumberOfEnemies = 3;
    inline constexpr int NumberOfAllies = 1;
}