#pragma once
#include <unordered_map>
#include <vector>

#include "raylib.h"

enum class AnimationType {
    Attack,
    SkeletonDeath,
    KnightDeath
};

struct Animation {
    constexpr Animation(int frameCount, float frameWidth, float frameHeight, float fps, float fadeOutTime, const char* texturePath)
        : frameCount(frameCount),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        fps(fps),
        fadeOutDuration(fadeOutTime),
        texturePath(texturePath) {}

    [[nodiscard]] float GetDuration() const {
        return static_cast<float>(frameCount) / fps;
    }

    int frameCount{};
    float frameWidth{};
    float frameHeight{};
    float fps{};
    float fadeOutDuration{};
    const char* texturePath;
};

struct AnimationInstance {
    Animation animation;
    Texture2D texture{};
    Vector2 pos{};
    float elapsed = 0.0f;
};

class AnimationPlayer {
public:
    AnimationPlayer() = default;
    ~AnimationPlayer();

    AnimationPlayer(const AnimationPlayer&) = delete;
    AnimationPlayer& operator=(const AnimationPlayer&) = delete;

    void Draw() const;
    void Update(float deltaTime);
    void Play(AnimationType animationType, Vector2 pos);

private:
    [[nodiscard]] static const Animation& GetAnimation(AnimationType animationType);
    [[nodiscard]] Texture2D GetOrLoadTexture(const char* texturePath);

    std::unordered_map<const char*, Texture2D> textures_;
    std::vector<AnimationInstance> instances_;
};
