#include "Board.h"

#include <queue>
#include <stdexcept>

#include "Utils.h"

Board::Board(Random &random){
    int spikesNumber = random.Int(GameConfig::MinSpikeTiles, GameConfig::MaxSpikeTiles);
    int wallsNumber = random.Int(GameConfig::MinWallTiles, GameConfig::MaxWallTiles);

    std::array<int, width * height> candidates{};
    std::iota(candidates.begin(), candidates.end(), 0);
    random.Shuffle(candidates);

    for (const int index : candidates) {
        const int x = index % width;
        const int y = index / width;
        const GridPosition pos{.x = x, .y = y};

        if (spikesNumber > 0) {
            SetTileTypeAt(pos, Tile::Spike);
            if (AreWalkableTilesConnected()) {
                --spikesNumber;
            } else {
                SetTileTypeAt(pos, Tile::Floor);
            }
        } else if (wallsNumber > 0) {
            SetTileTypeAt(pos, Tile::Wall);
            if (AreWalkableTilesConnected()) {
                --wallsNumber;
            } else {
                SetTileTypeAt(pos, Tile::Floor);
            }
        }
        if (spikesNumber == 0 && wallsNumber == 0) {
            break;
        }
    }
    if (spikesNumber > 0 || wallsNumber > 0) {
        throw std::runtime_error("Failed to generate a valid board with the specified number of spikes and walls.");
    }
}

void Board::SetTileTypeAt(const GridPosition pos, const Tile tileType) {
    tiles[pos.y][pos.x] = tileType;
}

bool Board::AreWalkableTilesConnected() const {
    GridPosition start{};
    int walkableCount = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const GridPosition pos{.x = x, .y = y};
            if (!IsFloor(pos)) {
                continue;
            }

            if (walkableCount == 0) {
                start = pos;
            }
            ++walkableCount;
        }
    }

    if (walkableCount == 0) {
        return false;
    }

    std::array<std::array<bool, GameConfig::BoardWidth>, GameConfig::BoardHeight> visited{};
    std::queue<GridPosition> pending;

    pending.push(start);
    visited[start.y][start.x] = true;
    int reachedCount = 1;

    constexpr GridPosition directions[] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };

    while (!pending.empty()) {
        const GridPosition current = pending.front();
        pending.pop();

        for (const GridPosition direction : directions) {
            const GridPosition next{
                .x = current.x + direction.x,
                .y = current.y + direction.y
            };

            if (next.x < 0 || next.x >= width ||
                next.y < 0 || next.y >= height) {
                continue;
                }

            if (visited[next.y][next.x] || !IsFloor(next)) {
                continue;
            }

            visited[next.y][next.x] = true;
            pending.push(next);
            ++reachedCount;
        }
    }

    return reachedCount == walkableCount;
}

GridPosition Board::GetRandomFloorPosition(Random& random) const {
    while (true) {
        const int x = random.Int(0, width - 1);
        const int y = random.Int(0, height - 1);
        if (tiles[y][x] == Tile::Floor) {
            return {.x = x, .y = y};
        }
    }
}

std::vector<GridPosition> Board::GetAdjacentFloorPositions(const GridPosition pos) const {
    std::vector<GridPosition> adjacentPositions;
    constexpr std::array<GridPosition, 4> directions = {{{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};

    for (const auto& dir : directions) {
        const GridPosition adjacentPos{.x = pos.x + dir.x, .y = pos.y + dir.y};
        if (IsInsideBounds(adjacentPos) && IsFloor(adjacentPos)) {
            adjacentPositions.push_back(adjacentPos);
        }
    }

    return adjacentPositions;
}

std::vector<GridPosition> Board::GetPositions(const Tile tileType) const {
    std::vector<GridPosition> positions;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const GridPosition pos{.x = x, .y = y};
            if (GetTileTypeAt(pos) == tileType) {
                positions.push_back(pos);
            }
        }
    }
    return positions;
}

int Board::GetDistanceBetweenPositions(const GridPosition pos1, const GridPosition pos2) {
    return std::abs(pos1.x - pos2.x) + std::abs(pos1.y - pos2.y);
}

bool Board::ArePositionsAdjacent(const GridPosition pos1, const GridPosition pos2) {
    return GetDistanceBetweenPositions(pos1, pos2) == 1;
}
