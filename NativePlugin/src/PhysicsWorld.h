#pragma once

#include <cstdint>
#include <vector>

#include "Vec2.h"

namespace angrybirds {

enum class ShapeType : std::uint8_t {
    Circle,
    Box
};

struct BodyState {
    Vec2 position;
    Vec2 velocity;
    bool active = true;
};

class PhysicsWorld {
public:
    explicit PhysicsWorld(Vec2 gravity) noexcept;

    std::int32_t AddCircle(
        Vec2 position,
        float radius,
        float mass,
        float restitution,
        float friction);

    std::int32_t AddBox(
        Vec2 position,
        Vec2 halfExtents,
        float mass,
        float restitution,
        float friction);

    bool RemoveBody(std::int32_t bodyId) noexcept;
    bool SetPosition(std::int32_t bodyId, Vec2 position) noexcept;
    bool SetVelocity(std::int32_t bodyId, Vec2 velocity) noexcept;
    bool SetActive(std::int32_t bodyId, bool active) noexcept;
    bool ApplyImpulse(std::int32_t bodyId, Vec2 impulse) noexcept;
    bool GetState(std::int32_t bodyId, BodyState& state) const noexcept;
    bool Step(float deltaTime, std::int32_t substeps, std::int32_t solverIterations) noexcept;

private:
    struct Body {
        std::int32_t id = 0;
        ShapeType shape = ShapeType::Circle;
        Vec2 position;
        Vec2 velocity;
        Vec2 halfExtents;
        float inverseMass = 0.0f;
        float restitution = 0.0f;
        float friction = 0.5f;
        bool active = true;
    };

    struct Contact {
        Vec2 normal;
        float penetration = 0.0f;
    };

    Body* FindBody(std::int32_t bodyId) noexcept;
    const Body* FindBody(std::int32_t bodyId) const noexcept;
    std::int32_t AddBody(Body body);

    static bool FindContact(const Body& first, const Body& second, Contact& contact) noexcept;
    static bool CircleVsBox(const Body& circle, const Body& box, Contact& contact) noexcept;
    static void ResolveVelocity(Body& first, Body& second, const Contact& contact) noexcept;
    static void ResolvePosition(Body& first, Body& second, const Contact& contact) noexcept;

    Vec2 gravity_;
    std::int32_t nextBodyId_ = 1;
    std::vector<Body> bodies_;
};

}
