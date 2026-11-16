#include "BattleHud.h"

#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include "Battle.h"
#include "Board.h"

namespace {
    constexpr Vector2 HpCenter{.x = 550.0f, .y = 53.0f};
    constexpr Vector2 ApCenter{.x = 784.0f, .y = 53.0f};
    constexpr Vector2 EndTurnCenter{.x = 1084.0f, .y = 364.0f};
    constexpr float EndTurnRadius = 62.0f;
    constexpr Color LabelColor{.r = 255, .g = 244, .b = 226, .a = 255};
    constexpr Rectangle BoardArea{387.0f, 134.0f, 503.0f, 490.0f};

    constexpr char BackgroundPath[] = "assets/background.png";
    constexpr char PillarPath[] = "assets/pillar.png";
    constexpr char SpikesPath[] = "assets/spikes.png";
    constexpr char KnightPath[] = "assets/knight.png";
    constexpr char SkeletonPath[] = "assets/skeleton.png";
    constexpr char KnightPortraitPath[] = "assets/knight_portrait.png";
    constexpr char SkeletonPortraitPath[] = "assets/skeleton_portrait.png";
    constexpr char HealthBadgePath[] = "assets/health_badge.png";
    constexpr char ActionMenuPath[] = "assets/action_menu.png";
    constexpr char SkeletonDeathPath[] = "assets/skeleton_death.png";
    constexpr char KnightDeathPath[] = "assets/knight_death.png";

    constexpr int DeathFrameCount = 6;
    constexpr float DeathFrameWidth = 72.0f;
    constexpr float DeathFrameHeight = 64.0f;
    constexpr float DeathFps = 12.0f;
    constexpr float DeathCollapseDuration = DeathFrameCount / DeathFps;
    constexpr float DeathFadeDuration = 1.0f;

    void DrawCenteredText(const Font &font, const char* text, const Vector2 center, const float fontSize) {
        constexpr float spacing = 1.0f;
        const Vector2 size = MeasureTextEx(font, text, fontSize, spacing);
        const Vector2 position{.x = center.x - size.x * 0.5f, .y = center.y - size.y * 0.5f};
        DrawTextEx(font, text, position, fontSize, spacing, LabelColor);
    }
}

BattleHud::BattleHud(const Font& font)
    : font_(font) {
    const auto loadTexture = [](Texture2D& texture, const char* path) {
        texture = LoadTexture(path);
        if (!IsTextureValid(texture)) {
            throw std::runtime_error(std::string{"Cannot load HUD texture: "} + path);
        }
    };

    try {
        loadTexture(background_, BackgroundPath);
        if (background_.width != Width || background_.height != Height) {
            throw std::runtime_error("HUD background must be 1280 x 720.");
        }

        loadTexture(pillar_, PillarPath);
        loadTexture(spikes_, SpikesPath);
        loadTexture(knight_, KnightPath);
        loadTexture(skeleton_, SkeletonPath);
        loadTexture(knightPortrait_, KnightPortraitPath);
        loadTexture(skeletonPortrait_, SkeletonPortraitPath);
        loadTexture(healthBadge_, HealthBadgePath);
        loadTexture(actionMenu_, ActionMenuPath);
        loadTexture(skeletonDeath_, SkeletonDeathPath);
        loadTexture(knightDeath_, KnightDeathPath);
    } catch (...) {
        UnloadTextures();
        throw;
    }
}

BattleHud::~BattleHud() {
    UnloadTextures();
}

bool BattleHud::EndTurnPressed() {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsEndTurnHovered();
}

void BattleHud::HandleTileClick(GridPosition pos, Battle& battle) {
    const InteractableTiles tiles = GetInteractableTiles(battle);

    if (std::ranges::find(tiles.moveTiles, pos) != tiles.moveTiles.end()) {
        if (battle.TryMoveActiveUnit(pos)) {
            selectedEnemy_.reset();
        }
        return;
    }

    if (std::ranges::find(tiles.enemyTiles, pos) != tiles.enemyTiles.end()) {
        selectedEnemy_ = pos;
    }
}

void BattleHud::Draw(const Battle& battle) const {
    DrawTexture(background_, 0, 0, WHITE);
    DrawBoardTiles(battle.GetBoard());

    DrawInteractableTiles(GetInteractableTiles(battle));
    DrawDeathAnimations();
    DrawUnits(battle);
    if (selectedEnemy_) DrawActionMenu(GetScreenPositionAtGridPosition(*selectedEnemy_));

    const Unit unit = battle.GetActiveUnit();

    DrawCenteredText(font_, TextFormat("HP %d/%d", unit.GetHealth(), unit.GetMaxHealth()), HpCenter, 26.0f);
    DrawCenteredText(font_, TextFormat("AP %d/%d", unit.GetActionPoints(), unit.GetMaxActionPoints()), ApCenter, 26.0f);

    DrawCenteredText(font_, "END", {.x = EndTurnCenter.x, .y = EndTurnCenter.y - 15.0f}, 25.0f);
    DrawCenteredText(font_, "TURN", {.x = EndTurnCenter.x, .y = EndTurnCenter.y + 15.0f}, 25.0f);

    if (unit.GetType() == UnitType::Enemy) {
        DrawCircleV(EndTurnCenter, EndTurnRadius, Color{.r = 15, .g = 12, .b = 10, .a = 155});
    } else if (IsEndTurnHovered()) {
        DrawCircleV(EndTurnCenter, EndTurnRadius, Color{.r = 255, .g = 216, .b = 123, .a = 45});
    }
}

void BattleHud::Update(Battle& battle) {
    const auto newDeaths = battle.TakePendingDeaths();
    deathAnimations_.insert(deathAnimations_.end(), newDeaths.begin(), newDeaths.end());

    const float deltaTime = GetFrameTime();
    for (UnitDeath& death : deathAnimations_) {
        death.elapsed += deltaTime;
    }

    std::erase_if(deathAnimations_, [](const UnitDeath& death) {
        return death.elapsed >= DeathCollapseDuration + DeathFadeDuration;
    });

    if (battle.IsFinished() || !battle.IsAllyTurn()) {
        selectedEnemy_.reset();
        return;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    const Vector2 mousePos = GetMousePosition();

    if (selectedEnemy_){
        const Vector2 unitCenter = GetScreenPositionAtGridPosition(*selectedEnemy_);
        const auto [x, y] = GetActionMenuPosition(unitCenter);
        constexpr float radius = 23.0f;

        const Vector2 attackCenter{.x = x + 28.0f, .y = y + 27.0f};
        const Vector2 pushCenter{.x = x + 75.0f, .y = y + 27.0f};

        if (CheckCollisionPointCircle(mousePos, attackCenter, radius)) {
            std::cout << "Attack clicked" << std::endl;
            battle.TryAttackUnit(*selectedEnemy_);
            selectedEnemy_.reset();
            return;
        }

        if (CheckCollisionPointCircle(mousePos, pushCenter, radius)) {
            battle.TryPushUnit(*selectedEnemy_);
            selectedEnemy_.reset();
            return;
        }
    }

    if (const auto pos = GetGridPositionAtScreenPosition(mousePos)) {
        HandleTileClick(*pos, battle);
    }
}

bool BattleHud::IsEndTurnHovered() {
    return CheckCollisionPointCircle(GetMousePosition(), EndTurnCenter, EndTurnRadius);
}

std::optional<GridPosition> BattleHud::GetGridPositionAtScreenPosition(const Vector2 screenPos) {
    const float localX = screenPos.x - BoardArea.x;
    const float localY = screenPos.y - BoardArea.y;

    if (localX < 0.0f || localX >= BoardArea.width ||
        localY < 0.0f || localY >= BoardArea.height) {
        return std::nullopt;
    }

    constexpr float tileWidth = BoardArea.width / GameConfig::BoardWidth;
    constexpr float tileHeight = BoardArea.height / GameConfig::BoardHeight;

    return GridPosition{
        .x = static_cast<int>(localX / tileWidth),
        .y = static_cast<int>(localY / tileHeight)
    };
}

Vector2 BattleHud::GetScreenPositionAtGridPosition(const GridPosition pos) {
    const Rectangle tile = GetGridPositionRect(pos);

    return Vector2{
        .x = tile.x + tile.width * 0.5f,
        .y = tile.y + tile.height * 0.5f
    };
}

Rectangle BattleHud::GetGridPositionRect(const GridPosition pos) {
    constexpr float tileWidth = BoardArea.width / GameConfig::BoardWidth;
    constexpr float tileHeight = BoardArea.height / GameConfig::BoardHeight;

    return Rectangle{
        .x = BoardArea.x + static_cast<float>(pos.x) * tileWidth,
        .y = BoardArea.y + static_cast<float>(pos.y) * tileHeight,
        .width = tileWidth,
        .height = tileHeight
    };
}

InteractableTiles BattleHud::GetInteractableTiles(const Battle &battle) {
    if (battle.IsFinished() || !battle.IsAllyTurn()) {
        return {};
    }

    const Unit& unit = battle.GetActiveUnit();

    if (!unit.IsAlive() || unit.GetType() == UnitType::Enemy) {
        return {};
    }

    return {
        .moveTiles = battle.GetAdjacentEmptyPositions(unit),
        .enemyTiles = battle.GetAdjacentOpponentPositions(unit)
    };
}

Vector2 BattleHud::GetActionMenuPosition(Vector2 unitCenter) const {
    constexpr float gap = 20.0f;

    return {
        .x = unitCenter.x - static_cast<float>(actionMenu_.width) * 0.5f,
        .y = unitCenter.y - static_cast<float>(skeleton_.height) * 0.5f - static_cast<float>(actionMenu_.height) - gap
    };
}

void BattleHud::DrawBoardTiles(const Board& board) const {
    for (const GridPosition pos : board.GetPositions(Tile::Wall)) {
        DrawOnTile(pillar_, pos);
    }

    for (const GridPosition pos : board.GetPositions(Tile::Spike)) {
        DrawOnTile(spikes_, pos);
    }
}

void BattleHud::DrawUnits(const Battle &battle) const {
    for (const Unit& unit : battle.GetUnits()) {
        if (!unit.IsAlive()) {
            continue;
        }
        switch (unit.GetType()) {
            case UnitType::Ally:
                DrawOnTile(knight_, unit.GetPosition());
                break;
            case UnitType::Enemy:
                DrawOnTile(skeleton_, unit.GetPosition());
                break;
        }
    }
    for (const Unit& unit : battle.GetUnits()) {
        if (!unit.IsAlive()) {
            continue;
        }
        DrawHealthBadge(unit, GetScreenPositionAtGridPosition(unit.GetPosition()));
    }
}

void BattleHud::DrawActionMenu(const Vector2 unitCenter) const {
    DrawTextureV(actionMenu_, GetActionMenuPosition(unitCenter), WHITE);
}

void BattleHud::DrawHealthBadge(const Unit& unit, const Vector2 unitCenter) const {
    constexpr float gap = 3.0f;

    const Vector2 badgePosition{
        .x = unitCenter.x - static_cast<float>(healthBadge_.width) * 0.5f,
        .y = unitCenter.y - static_cast<float>(skeleton_.height) * 0.5f - static_cast<float>(healthBadge_.height) - gap
    };

    DrawTextureV(healthBadge_, badgePosition, WHITE);

    const Vector2 textCenter{
        .x = badgePosition.x + 31.0f,
        .y = badgePosition.y + 9.0f
    };

    DrawCenteredText(font_, TextFormat("%d/%d", unit.GetHealth(), unit.GetMaxHealth()), textCenter, 12.0f);
}

Vector2 BattleHud::DrawOnTile(const Texture2D &texture, const GridPosition pos) {
    const Rectangle tile = GetGridPositionRect(pos);
    const Vector2 spritePosition{
        .x = tile.x + (tile.width - static_cast<float>(texture.width)) * 0.5f,
        .y = tile.y + (tile.height - static_cast<float>(texture.height)) * 0.5f
    };

    DrawTextureV(texture, spritePosition, WHITE);
    return spritePosition;
}

void BattleHud::DrawInteractableTiles(const InteractableTiles &tiles) {
    const Color moveColor = Fade(SKYBLUE, 0.30f);
    const Color enemyColor = Fade(RED, 0.30f);

    for (const GridPosition pos : tiles.moveTiles) {
        DrawRectangleRec(GetGridPositionRect(pos), moveColor);
    }

    for (const GridPosition pos : tiles.enemyTiles) {
        DrawRectangleRec(GetGridPositionRect(pos), enemyColor);
    }
}

void BattleHud::DrawDeathAnimations() const {
    for (const UnitDeath& death : deathAnimations_) {
        if (death.type != UnitType::Enemy) {
            continue;
        }

        const int frame = std::min(static_cast<int>(death.elapsed * DeathFps), DeathFrameCount - 1);

        const Rectangle source{
            .x = static_cast<float>(frame) * DeathFrameWidth,
            .y = 0.0f,
            .width = DeathFrameWidth,
            .height = DeathFrameHeight
        };

        const Vector2 center = GetScreenPositionAtGridPosition(death.pos);
        const Vector2 topLeft{
            .x = center.x - DeathFrameWidth * 0.5f,
            .y = center.y - DeathFrameHeight * 0.5f
        };

        const float fadeProgress = std::clamp(
            (death.elapsed - DeathCollapseDuration) / DeathFadeDuration, 0.0f, 1.0f
        );

        Texture2D texture;
        death.type == UnitType::Enemy ? texture = skeletonDeath_ : texture = knightDeath_;

        DrawTextureRec(texture, source, topLeft, Fade(WHITE, 1.0f - fadeProgress));
    }
}

void BattleHud::UnloadTextures() noexcept {
    for (const Texture2D texture : {
             background_, pillar_, spikes_, knight_, skeleton_,
             knightPortrait_, skeletonPortrait_, healthBadge_, actionMenu_
         }) {
        if (texture.id != 0) {
            UnloadTexture(texture);
        }
    }
}
