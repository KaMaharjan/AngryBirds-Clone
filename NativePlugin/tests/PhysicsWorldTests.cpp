#include "PhysicsWorld.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

using angrybirds::BodyState;
using angrybirds::PhysicsWorld;
using angrybirds::Vec2;

namespace {

void Require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void GravityAcceleratesDynamicBodies() {
    PhysicsWorld world({0.0f, -9.81f});
    const std::int32_t bodyId = world.AddCircle({0.0f, 10.0f}, 0.5f, 1.0f, 0.0f, 0.5f);
    Require(bodyId != 0, "create falling circle");
    Require(world.Step(0.5f, 4, 8), "step gravity simulation");

    BodyState state;
    Require(world.GetState(bodyId, state), "read falling circle state");
    Require(state.position.y < 10.0f, "gravity changes position");
    Require(std::fabs(state.velocity.y + 4.905f) < 0.02f, "gravity changes velocity");
}

void GroundStopsFallingCircle() {
    PhysicsWorld world({0.0f, -9.81f});
    const std::int32_t floorId = world.AddBox({0.0f, -0.5f}, {10.0f, 0.5f}, 0.0f, 0.0f, 0.8f);
    const std::int32_t circleId = world.AddCircle({0.0f, 4.0f}, 0.5f, 1.0f, 0.0f, 0.8f);
    Require(floorId != 0 && circleId != 0, "create ground collision bodies");

    for (int frame = 0; frame < 240; ++frame) {
        Require(world.Step(1.0f / 120.0f, 2, 8), "step ground collision simulation");
    }

    BodyState state;
    Require(world.GetState(circleId, state), "read grounded circle state");
    Require(state.position.y >= 0.48f, "circle does not pass through the floor");
    Require(std::fabs(state.velocity.y) < 0.15f, "circle settles on the floor");
}

void CircleStopsAgainstStaticBox() {
    PhysicsWorld world({0.0f, 0.0f});
    const std::int32_t wallId = world.AddBox({1.5f, 0.0f}, {0.25f, 0.75f}, 0.0f, 0.0f, 0.5f);
    const std::int32_t circleId = world.AddCircle({0.0f, 0.0f}, 0.25f, 1.0f, 0.0f, 0.5f);
    Require(wallId != 0 && circleId != 0, "create wall collision bodies");
    Require(world.SetVelocity(circleId, {10.0f, 0.0f}), "set circle velocity");

    for (int frame = 0; frame < 20; ++frame) {
        Require(world.Step(0.01f, 4, 8), "step wall collision simulation");
    }

    BodyState state;
    Require(world.GetState(circleId, state), "read wall collision state");
    Require(state.position.x <= 1.02f, "circle does not pass through the wall");
    Require(std::fabs(state.velocity.x) < 0.1f, "inelastic wall collision stops the circle");
}

void BoxStopsAgainstStaticBox() {
    PhysicsWorld world({0.0f, 0.0f});
    const std::int32_t wallId = world.AddBox({2.0f, 0.0f}, {0.25f, 1.0f}, 0.0f, 0.0f, 0.5f);
    const std::int32_t boxId = world.AddBox({0.0f, 0.0f}, {0.5f, 0.5f}, 1.0f, 0.0f, 0.5f);
    Require(wallId != 0 && boxId != 0, "create box collision bodies");
    Require(world.SetVelocity(boxId, {10.0f, 0.0f}), "set box velocity");

    for (int frame = 0; frame < 20; ++frame) {
        Require(world.Step(0.01f, 4, 8), "step box collision simulation");
    }

    BodyState state;
    Require(world.GetState(boxId, state), "read box collision state");
    Require(state.position.x <= 1.27f, "dynamic box does not pass through the static box");
    Require(std::fabs(state.velocity.x) < 0.1f, "inelastic box collision stops the dynamic box");
}

}

int main() {
    GravityAcceleratesDynamicBodies();
    GroundStopsFallingCircle();
    CircleStopsAgainstStaticBox();
    BoxStopsAgainstStaticBox();
    std::cout << "All native physics tests passed.\n";
    return EXIT_SUCCESS;
}
