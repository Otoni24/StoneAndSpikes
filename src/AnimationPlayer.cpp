#include "AnimationPlayer.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace {
    constexpr Animation AttackSlash{
        6, 80.0f, 64.0f, 20.0f, 0.0f, "assets/attack_slash.png"
    };

    constexpr Animation KnightDeath{
        6, 72.0f, 64.0f, 12.0f, 1.0f, "assets/knight_death.png"
    };

    constexpr Animation SkeletonDeath{
        6, 72.0f, 64.0f, 12.0f, 1.0f, "assets/skeleton_death.png"
    };
}

AnimationPlayer::~AnimationPlayer() {
    for (const auto& entry : textures_) {
        UnloadTexture(entry.second);
    }
}

void AnimationPlayer::Draw() const{
    for (const auto&[animation, texture, pos, elapsed] : instances_) {
        const int frame = std::min(static_cast<int>(elapsed * animation.fps), animation.frameCount - 1);

        const Rectangle source{
            .x = static_cast<float>(frame) * animation.frameWidth,
            .y = 0.0f,
            .width = animation.frameWidth,
            .height = animation.frameHeight
        };

        const Vector2 topLeft{
            .x = pos.x - animation.frameWidth * 0.5f,
            .y = pos.y - animation.frameHeight * 0.5f
        };

        const float fadeProgress = animation.fadeOutDuration > 0.0f ? std::clamp(
            (elapsed - animation.GetDuration()) / animation.fadeOutDuration, 0.0f, 1.0f
        ) : 0.0f;

        DrawTextureRec(texture, source, topLeft, Fade(WHITE, 1.0f - fadeProgress));
    }
}

void AnimationPlayer::Update(const float deltaTime) {
    for (auto& instance : instances_) {
        instance.elapsed += deltaTime;
    }

    std::erase_if(instances_, [](const AnimationInstance& instance) {
        return instance.elapsed >= instance.animation.GetDuration() + instance.animation.fadeOutDuration;
    });
}

void AnimationPlayer::Play(AnimationType animationType, const Vector2 pos) {
    const Animation& animation = GetAnimation(animationType);
    const Texture2D texture = GetOrLoadTexture(animation.texturePath);
    instances_.push_back({.animation = animation, .texture = texture, .pos = pos});
}

const Animation& AnimationPlayer::GetAnimation(const AnimationType animationType) {
    switch (animationType) {
        case AnimationType::Attack:
            return AttackSlash;
        case AnimationType::SkeletonDeath:
            return SkeletonDeath;
        case AnimationType::KnightDeath:
            return KnightDeath;
    }
    throw std::runtime_error("Invalid animation type");
}

Texture2D AnimationPlayer::GetOrLoadTexture(const char *texturePath) {
    if (!textures_.contains(texturePath)) {
        const Texture2D texture = LoadTexture(texturePath);
        if (!IsTextureValid(texture)) {
            throw std::runtime_error(std::string{"Cannot load animation texture: "} + texturePath);
        }
        try {
            textures_.emplace(texturePath, texture);
        } catch (...) {
            UnloadTexture(texture);
            throw;
        }
    }
    return textures_[texturePath];
}
