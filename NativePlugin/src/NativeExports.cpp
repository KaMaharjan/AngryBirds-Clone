#include "AngryBirdsPhysics.h"

#include <new>

#include "PhysicsWorld.h"

struct ABWorld {
    explicit ABWorld(angrybirds::Vec2 gravity) noexcept : physics(gravity) {}

    angrybirds::PhysicsWorld physics;
};

ABWorld* AB_CALL AB_CreateWorld(float gravityX, float gravityY) {
    const angrybirds::Vec2 gravity{gravityX, gravityY};
    if (!angrybirds::IsFinite(gravity)) {
        return nullptr;
    }

    return new (std::nothrow) ABWorld(gravity);
}

void AB_CALL AB_DestroyWorld(ABWorld* world) {
    delete world;
}

int32_t AB_CALL AB_CreateCircle(
    ABWorld* world,
    float positionX,
    float positionY,
    float radius,
    float mass,
    float restitution,
    float friction) {
    if (world == nullptr) {
        return 0;
    }

    return world->physics.AddCircle(
        {positionX, positionY},
        radius,
        mass,
        restitution,
        friction);
}

int32_t AB_CALL AB_CreateBox(
    ABWorld* world,
    float positionX,
    float positionY,
    float halfWidth,
    float halfHeight,
    float mass,
    float restitution,
    float friction) {
    if (world == nullptr) {
        return 0;
    }

    return world->physics.AddBox(
        {positionX, positionY},
        {halfWidth, halfHeight},
        mass,
        restitution,
        friction);
}

int32_t AB_CALL AB_DestroyBody(ABWorld* world, int32_t bodyId) {
    return world != nullptr && world->physics.RemoveBody(bodyId) ? 1 : 0;
}

int32_t AB_CALL AB_SetBodyPosition(ABWorld* world, int32_t bodyId, float x, float y) {
    return world != nullptr && world->physics.SetPosition(bodyId, {x, y}) ? 1 : 0;
}

int32_t AB_CALL AB_SetBodyVelocity(ABWorld* world, int32_t bodyId, float x, float y) {
    return world != nullptr && world->physics.SetVelocity(bodyId, {x, y}) ? 1 : 0;
}

int32_t AB_CALL AB_SetBodyActive(ABWorld* world, int32_t bodyId, int32_t active) {
    return world != nullptr && world->physics.SetActive(bodyId, active != 0) ? 1 : 0;
}

int32_t AB_CALL AB_ApplyImpulse(ABWorld* world, int32_t bodyId, float impulseX, float impulseY) {
    return world != nullptr && world->physics.ApplyImpulse(bodyId, {impulseX, impulseY}) ? 1 : 0;
}

int32_t AB_CALL AB_GetBodyState(
    ABWorld* world,
    int32_t bodyId,
    float* positionX,
    float* positionY,
    float* velocityX,
    float* velocityY,
    int32_t* active) {
    if (world == nullptr || positionX == nullptr || positionY == nullptr ||
        velocityX == nullptr || velocityY == nullptr || active == nullptr) {
        return 0;
    }

    angrybirds::BodyState state;
    if (!world->physics.GetState(bodyId, state)) {
        return 0;
    }

    *positionX = state.position.x;
    *positionY = state.position.y;
    *velocityX = state.velocity.x;
    *velocityY = state.velocity.y;
    *active = state.active ? 1 : 0;
    return 1;
}

int32_t AB_CALL AB_Step(
    ABWorld* world,
    float deltaTime,
    int32_t substeps,
    int32_t solverIterations) {
    return world != nullptr && world->physics.Step(deltaTime, substeps, solverIterations) ? 1 : 0;
}
