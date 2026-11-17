#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "GameConfig.h"
#include "GameTypes.h"

class Random;

enum class Tile : std::uint8_t {
    Floor = 0,
    Wall = 1,
    Spike = 2
};

class Board {
public:
    explicit Board(Random& random);

    [[nodiscard]] bool IsFloor(const GridPosition pos) const { return tiles[pos.y][pos.x] == Tile::Floor; }
    [[nodiscard]] bool IsWall(const GridPosition pos) const { return tiles[pos.y][pos.x] == Tile::Wall; }
    [[nodiscard]] bool IsSpike(const GridPosition pos) const { return tiles[pos.y][pos.x] == Tile::Spike; }
    [[nodiscard]] GridPosition GetRandomFloorPosition(Random& random) const;
    [[nodiscard]] Tile GetTileTypeAt(const GridPosition pos) const { return tiles[pos.y][pos.x]; }
    [[nodiscard]] std::vector<GridPosition> GetAdjacentNonWallPositions(GridPosition pos) const;
    [[nodiscard]] std::vector<GridPosition> GetPositions(Tile tileType) const;

    [[nodiscard]] static bool IsInsideBounds(const GridPosition pos) { return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height; }
    [[nodiscard]] static int GetDistanceBetweenPositions(GridPosition pos1, GridPosition pos2);
    [[nodiscard]] static bool ArePositionsAdjacent(GridPosition pos1, GridPosition pos2);

private:
    void SetTileTypeAt(GridPosition pos, Tile tileType);
    [[nodiscard]] bool AreWalkableTilesConnected() const;

    static constexpr int width = GameConfig::BoardWidth;
    static constexpr int height = GameConfig::BoardHeight;

    std::array<std::array<Tile, GameConfig::BoardWidth>, GameConfig::BoardHeight> tiles{};
};