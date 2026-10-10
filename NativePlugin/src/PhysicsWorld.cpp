#include "PhysicsWorld.h"

#include <algorithm>
#include <cmath>
#include <new>

namespace angrybirds {
namespace {

constexpr float kEpsilon = 1.0e-6f;
constexpr float kPositionSlop = 1.0e-3f;
constexpr float kPositionCorrection = 0.8f;

bool IsValidMaterial(float mass, float restitution, float friction) noexcept {
    if (!std::isfinite(mass) || mass < 0.0f ||
        !std::isfinite(restitution) || restitution < 0.0f || restitution > 1.0f ||
        !std::isfinite(friction) || friction < 0.0f) {
        return false;
    }

    return mass == 0.0f || std::isfinite(1.0f / mass);
}

float Clamp(float value, float minimum, float maximum) noexcept {
    return std::max(minimum, std::min(value, maximum));
}

}

PhysicsWorld::PhysicsWorld(Vec2 gravity) noexcept : gravity_(gravity) {}

std::int32_t PhysicsWorld::AddCircle(
    Vec2 position,
    float radius,
    float mass,
    float restitution,
    float friction) {
    if (!IsFinite(position) || !std::isfinite(radius) || radius <= 0.0f ||
        !IsValidMaterial(mass, restitution, friction)) {
        return 0;
    }

    Body body;
    body.shape = ShapeType::Circle;
    body.position = position;
    body.halfExtents = {radius, radius};
    body.inverseMass = mass == 0.0f ? 0.0f : 1.0f / mass;
    body.restitution = restitution;
    body.friction = friction;
    return AddBody(body);
}

std::int32_t PhysicsWorld::AddBox(
    Vec2 position,
    Vec2 halfExtents,
    float mass,
    float restitution,
    float friction) {
    if (!IsFinite(position) || !IsFinite(halfExtents) ||
        halfExtents.x <= 0.0f || halfExtents.y <= 0.0f ||
        !IsValidMaterial(mass, restitution, friction)) {
        return 0;
    }

    Body body;
    body.shape = ShapeType::Box;
    body.position = position;
    body.halfExtents = halfExtents;
    body.inverseMass = mass == 0.0f ? 0.0f : 1.0f / mass;
    body.restitution = restitution;
    body.friction = friction;
    return AddBody(body);
}

bool PhysicsWorld::RemoveBody(std::int32_t bodyId) noexcept {
    const auto body = std::find_if(
        bodies_.begin(),
        bodies_.end(),
        [bodyId](const Body& candidate) { return candidate.id == bodyId; });
    if (body == bodies_.end()) {
        return false;
    }

    bodies_.erase(body);
    return true;
}

bool PhysicsWorld::SetPosition(std::int32_t bodyId, Vec2 position) noexcept {
    Body* body = FindBody(bodyId);
    if (body == nullptr || !IsFinite(position)) {
        return false;
    }

    body->position = position;
    return true;
}

bool PhysicsWorld::SetVelocity(std::int32_t bodyId, Vec2 velocity) noexcept {
    Body* body = FindBody(bodyId);
    if (body == nullptr || body->inverseMass == 0.0f || !IsFinite(velocity)) {
        return false;
    }

    body->velocity = velocity;
    return true;
}

bool PhysicsWorld::SetActive(std::int32_t bodyId, bool active) noexcept {
    Body* body = FindBody(bodyId);
    if (body == nullptr) {
        return false;
    }

    body->active = active;
    return true;
}

bool PhysicsWorld::ApplyImpulse(std::int32_t bodyId, Vec2 impulse) noexcept {
    Body* body = FindBody(bodyId);
    if (body == nullptr || body->inverseMass == 0.0f || !IsFinite(impulse)) {
        return false;
    }

    body->velocity += impulse * body->inverseMass;
    return IsFinite(body->velocity);
}

bool PhysicsWorld::GetState(std::int32_t bodyId, BodyState& state) const noexcept {
    const Body* body = FindBody(bodyId);
    if (body == nullptr) {
        return false;
    }

    state.position = body->position;
    state.velocity = body->velocity;
    state.active = body->active;
    return true;
}

bool PhysicsWorld::Step(
    float deltaTime,
    std::int32_t substeps,
    std::int32_t solverIterations) noexcept {
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f ||
        substeps < 1 || substeps > 32 ||
        solverIterations < 1 || solverIterations > 32) {
        return false;
    }

    const float stepTime = deltaTime / static_cast<float>(substeps);
    for (std::int32_t step = 0; step < substeps; ++step) {
        for (Body& body : bodies_) {
            if (!body.active || body.inverseMass == 0.0f) {
                continue;
            }

            body.velocity += gravity_ * stepTime;
            body.position += body.velocity * stepTime;
        }

        for (std::int32_t iteration = 0; iteration < solverIterations; ++iteration) {
            for (std::size_t firstIndex = 0; firstIndex < bodies_.size(); ++firstIndex) {
                Body& first = bodies_[firstIndex];
                if (!first.active) {
                    continue;
                }

                for (std::size_t secondIndex = firstIndex + 1; secondIndex < bodies_.size(); ++secondIndex) {
                    Body& second = bodies_[secondIndex];
                    if (!second.active) {
                        continue;
                    }

                    Contact contact;
                    if (!FindContact(first, second, contact)) {
                        continue;
                    }

                    if (iteration == 0) {
                        ResolveVelocity(first, second, contact);
                    }
                    ResolvePosition(first, second, contact);
                }
            }
        }
    }

    return true;
}

PhysicsWorld::Body* PhysicsWorld::FindBody(std::int32_t bodyId) noexcept {
    const auto body = std::find_if(
        bodies_.begin(),
        bodies_.end(),
        [bodyId](const Body& candidate) { return candidate.id == bodyId; });
    return body == bodies_.end() ? nullptr : &*body;
}

const PhysicsWorld::Body* PhysicsWorld::FindBody(std::int32_t bodyId) const noexcept {
    const auto body = std::find_if(
        bodies_.begin(),
        bodies_.end(),
        [bodyId](const Body& candidate) { return candidate.id == bodyId; });
    return body == bodies_.end() ? nullptr : &*body;
}

std::int32_t PhysicsWorld::AddBody(Body body) {
    if (nextBodyId_ <= 0) {
        return 0;
    }

    body.id = nextBodyId_;
    try {
        bodies_.push_back(body);
    } catch (const std::bad_alloc&) {
        return 0;
    }

    ++nextBodyId_;
    return body.id;
}

bool PhysicsWorld::FindContact(const Body& first, const Body& second, Contact& contact) noexcept {
    if (first.shape == ShapeType::Circle && second.shape == ShapeType::Circle) {
        const Vec2 difference = second.position - first.position;
        const float distanceSquared = LengthSquared(difference);
        const float combinedRadius = first.halfExtents.x + second.halfExtents.x;
        if (distanceSquared >= combinedRadius * combinedRadius) {
            return false;
        }

        const float distance = std::sqrt(distanceSquared);
        contact.normal = distance > kEpsilon ? difference / distance : Vec2{1.0f, 0.0f};
        contact.penetration = combinedRadius - distance;
        return true;
    }

    if (first.shape == ShapeType::Box && second.shape == ShapeType::Box) {
        const Vec2 difference = second.position - first.position;
        const float overlapX =
            first.halfExtents.x + second.halfExtents.x - std::fabs(difference.x);
        const float overlapY =
            first.halfExtents.y + second.halfExtents.y - std::fabs(difference.y);
        if (overlapX <= 0.0f || overlapY <= 0.0f) {
            return false;
        }

        if (overlapX < overlapY) {
            contact.normal = {difference.x < 0.0f ? -1.0f : 1.0f, 0.0f};
            contact.penetration = overlapX;
        } else {
            contact.normal = {0.0f, difference.y < 0.0f ? -1.0f : 1.0f};
            contact.penetration = overlapY;
        }
        return true;
    }

    if (first.shape == ShapeType::Circle) {
        return CircleVsBox(first, second, contact);
    }

    Contact circleToBox;
    if (!CircleVsBox(second, first, circleToBox)) {
        return false;
    }

    contact.normal = circleToBox.normal * -1.0f;
    contact.penetration = circleToBox.penetration;
    return true;
}

bool PhysicsWorld::CircleVsBox(const Body& circle, const Body& box, Contact& contact) noexcept {
    const float left = box.position.x - box.halfExtents.x;
    const float right = box.position.x + box.halfExtents.x;
    const float bottom = box.position.y - box.halfExtents.y;
    const float top = box.position.y + box.halfExtents.y;
    const Vec2 closestPoint{
        Clamp(circle.position.x, left, right),
        Clamp(circle.position.y, bottom, top)};
    const Vec2 difference = closestPoint - circle.position;
    const float distanceSquared = LengthSquared(difference);
    const float radius = circle.halfExtents.x;
    if (distanceSquared > radius * radius) {
        return false;
    }

    if (distanceSquared > kEpsilon * kEpsilon) {
        const float distance = std::sqrt(distanceSquared);
        contact.normal = difference / distance;
        contact.penetration = radius - distance;
        return true;
    }

    const float toLeft = circle.position.x - left;
    const float toRight = right - circle.position.x;
    const float toBottom = circle.position.y - bottom;
    const float toTop = top - circle.position.y;
    const float nearestSide = std::min(std::min(toLeft, toRight), std::min(toBottom, toTop));

    if (nearestSide == toLeft) {
        contact.normal = {1.0f, 0.0f};
    } else if (nearestSide == toRight) {
        contact.normal = {-1.0f, 0.0f};
    } else if (nearestSide == toBottom) {
        contact.normal = {0.0f, 1.0f};
    } else {
        contact.normal = {0.0f, -1.0f};
    }
    contact.penetration = radius + nearestSide;
    return true;
}

void PhysicsWorld::ResolveVelocity(Body& first, Body& second, const Contact& contact) noexcept {
    const float inverseMassSum = first.inverseMass + second.inverseMass;
    if (inverseMassSum <= 0.0f) {
        return;
    }

    Vec2 relativeVelocity = second.velocity - first.velocity;
    const float normalSpeed = Dot(relativeVelocity, contact.normal);
    if (normalSpeed >= 0.0f) {
        return;
    }

    const float restitution = std::min(first.restitution, second.restitution);
    const float impulseMagnitude = -(1.0f + restitution) * normalSpeed / inverseMassSum;
    const Vec2 impulse = contact.normal * impulseMagnitude;
    first.velocity -= impulse * first.inverseMass;
    second.velocity += impulse * second.inverseMass;

    relativeVelocity = second.velocity - first.velocity;
    Vec2 tangent = relativeVelocity - contact.normal * Dot(relativeVelocity, contact.normal);
    const float tangentLengthSquared = LengthSquared(tangent);
    if (tangentLengthSquared <= kEpsilon * kEpsilon) {
        return;
    }

    tangent = tangent / std::sqrt(tangentLengthSquared);
    float frictionImpulse = -Dot(relativeVelocity, tangent) / inverseMassSum;
    const float frictionLimit = impulseMagnitude * std::sqrt(first.friction * second.friction);
    frictionImpulse = Clamp(frictionImpulse, -frictionLimit, frictionLimit);
    const Vec2 friction = tangent * frictionImpulse;
    first.velocity -= friction * first.inverseMass;
    second.velocity += friction * second.inverseMass;
}

void PhysicsWorld::ResolvePosition(Body& first, Body& second, const Contact& contact) noexcept {
    const float inverseMassSum = first.inverseMass + second.inverseMass;
    if (inverseMassSum <= 0.0f) {
        return;
    }

    const float correctionMagnitude =
        std::max(contact.penetration - kPositionSlop, 0.0f) *
        kPositionCorrection / inverseMassSum;
    const Vec2 correction = contact.normal * correctionMagnitude;
    first.position -= correction * first.inverseMass;
    second.position += correction * second.inverseMass;
}

}
