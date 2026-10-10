using System;
using System.Runtime.InteropServices;
using UnityEditor;
using UnityEngine;

public static class NativePhysicsPluginSmokeTest
{
    private const string PluginName = "AngryBirdsPhysics";

    [DllImport(PluginName, EntryPoint = "AB_CreateWorld", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr CreateWorldNative(float gravityX, float gravityY);

    [DllImport(PluginName, EntryPoint = "AB_DestroyWorld", CallingConvention = CallingConvention.Cdecl)]
    private static extern void DestroyWorldNative(IntPtr world);

    [DllImport(PluginName, EntryPoint = "AB_CreateBox", CallingConvention = CallingConvention.Cdecl)]
    private static extern int CreateBoxNative(
        IntPtr world,
        float positionX,
        float positionY,
        float halfWidth,
        float halfHeight,
        float mass,
        float restitution,
        float friction);

    [DllImport(PluginName, EntryPoint = "AB_CreateCircle", CallingConvention = CallingConvention.Cdecl)]
    private static extern int CreateCircleNative(
        IntPtr world,
        float positionX,
        float positionY,
        float radius,
        float mass,
        float restitution,
        float friction);

    [DllImport(PluginName, EntryPoint = "AB_GetBodyState", CallingConvention = CallingConvention.Cdecl)]
    private static extern int GetBodyStateNative(
        IntPtr world,
        int bodyId,
        out float positionX,
        out float positionY,
        out float velocityX,
        out float velocityY,
        out int active);

    [DllImport(PluginName, EntryPoint = "AB_Step", CallingConvention = CallingConvention.Cdecl)]
    private static extern int StepNative(IntPtr world, float deltaTime, int substeps, int solverIterations);

    [MenuItem("Angry Birds/Test Native Plugin")]
    public static void Run()
    {
        IntPtr world = CreateWorldNative(0.0f, -9.81f);
        if (world == IntPtr.Zero)
        {
            throw new InvalidOperationException("The native plugin did not create a test world.");
        }

        try
        {
            int floorId = CreateBoxNative(world, 0.0f, -0.5f, 10.0f, 0.5f, 0.0f, 0.0f, 0.8f);
            int circleId = CreateCircleNative(world, 0.0f, 4.0f, 0.5f, 1.0f, 0.0f, 0.8f);
            if (floorId == 0 || circleId == 0)
            {
                throw new InvalidOperationException("The native plugin could not create smoke-test bodies.");
            }

            for (int frame = 0; frame < 240; ++frame)
            {
                if (StepNative(world, 1.0f / 120.0f, 2, 8) == 0)
                {
                    throw new InvalidOperationException("The native plugin failed while stepping the smoke test.");
                }
            }

            if (GetBodyStateNative(
                    world,
                    circleId,
                    out float positionX,
                    out float positionY,
                    out float velocityX,
                    out float velocityY,
                    out int active) == 0)
            {
                throw new InvalidOperationException("The native plugin could not return the smoke-test body state.");
            }

            if (positionY < 0.48f || positionY > 0.65f || Mathf.Abs(velocityY) > 0.2f || active == 0)
            {
                throw new InvalidOperationException(
                    $"Native gravity/collision smoke test failed: y={positionY}, vy={velocityY}, active={active}.");
            }

            Debug.Log("Native plugin smoke test passed: gravity and floor collision are working.");
        }
        finally
        {
            DestroyWorldNative(world);
        }
    }
}
