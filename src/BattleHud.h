#pragma once

#include <optional>
#include <raylib.h>
#include <vector>

#include "AnimationPlayer.h"
#include "GameTypes.h"

class Unit;
class Board;
class Battle;

struct InteractableTiles {
    std::vector<GridPosition> moveTiles;
    std::vector<GridPosition> enemyTiles;
};

class BattleHud {
public:
    static constexpr int Width = 1280;
    static constexpr int Height = 720;

    // Create after InitWindow. The font is borrowed and must outlive this HUD.
    explicit BattleHud(const Font &font = GetFontDefault());
    ~BattleHud();

    BattleHud(const BattleHud&) = delete;
    BattleHud& operator=(const BattleHud&) = delete;

    void HandleTileClick(GridPosition pos, Battle& battle);
    void Draw(const Battle& battle) const;
    void Update(Battle& battle);

private:
    [[nodiscard]] static bool EndTurnPressed();
    [[nodiscard]] static bool IsEndTurnHovered();
    [[nodiscard]] static std::optional<GridPosition> GetGridPositionAtScreenPosition(Vector2 screenPos);
    [[nodiscard]] static Vector2 GetScreenPositionAtGridPosition(GridPosition pos);
    [[nodiscard]] static Rectangle GetGridPositionRect(GridPosition pos);
    [[nodiscard]] static InteractableTiles GetInteractableTiles(const Battle& battle);

    [[nodiscard]] Vector2 GetActionMenuPosition(Vector2 unitCenter) const;
    [[nodiscard]] Vector2 GetTurnArrowPosition(Vector2 unitCenter) const;

    void DrawBoardTiles(const Board& board) const;
    void DrawUnits(const Battle& battle) const;

    void DrawActionMenu(Vector2 unitCenter) const;
    void DrawTurnArrow(Vector2 unitCenter) const;
    void DrawHealthBadge(const Unit& unit, Vector2 unitCenter) const;
    void DrawTurnOrder(const Battle& battle) const;
    static Vector2 DrawOnTile(const Texture2D &texture, GridPosition pos);
    static void DrawInteractableTiles(const InteractableTiles& tiles);

    void UnloadTextures() noexcept;

    std::optional<GridPosition> selectedEnemy_{};
    AnimationPlayer animationPlayer_{};

    Texture2D background_{};
    Texture2D spikes_{};
    Texture2D pillar_{};
    Texture2D knight_{};
    Texture2D skeleton_{};
    Texture2D knightPortrait_{};
    Texture2D skeletonPortrait_{};
    Texture2D healthBadge_{};
    Texture2D actionMenu_{};
    Texture2D turnArrow_{};
    Font font_{};
};