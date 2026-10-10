#pragma once

#include <stdint.h>

#if defined(_WIN32)
#define AB_API __declspec(dllexport)
#define AB_CALL __cdecl
#else
#define AB_API __attribute__((visibility("default")))
#define AB_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ABWorld ABWorld;

AB_API ABWorld* AB_CALL AB_CreateWorld(float gravityX, float gravityY);
AB_API void AB_CALL AB_DestroyWorld(ABWorld* world);

AB_API int32_t AB_CALL AB_CreateCircle(
    ABWorld* world,
    float positionX,
    float positionY,
    float radius,
    float mass,
    float restitution,
    float friction);

AB_API int32_t AB_CALL AB_CreateBox(
    ABWorld* world,
    float positionX,
    float positionY,
    float halfWidth,
    float halfHeight,
    float mass,
    float restitution,
    float friction);

AB_API int32_t AB_CALL AB_DestroyBody(ABWorld* world, int32_t bodyId);
AB_API int32_t AB_CALL AB_SetBodyPosition(ABWorld* world, int32_t bodyId, float x, float y);
AB_API int32_t AB_CALL AB_SetBodyVelocity(ABWorld* world, int32_t bodyId, float x, float y);
AB_API int32_t AB_CALL AB_SetBodyActive(ABWorld* world, int32_t bodyId, int32_t active);
AB_API int32_t AB_CALL AB_ApplyImpulse(ABWorld* world, int32_t bodyId, float impulseX, float impulseY);

AB_API int32_t AB_CALL AB_GetBodyState(
    ABWorld* world,
    int32_t bodyId,
    float* positionX,
    float* positionY,
    float* velocityX,
    float* velocityY,
    int32_t* active);

AB_API int32_t AB_CALL AB_Step(
    ABWorld* world,
    float deltaTime,
    int32_t substeps,
    int32_t solverIterations);

#ifdef __cplusplus
}
#endif
