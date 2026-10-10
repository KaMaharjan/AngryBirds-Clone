using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

[DefaultExecutionOrder(-100)]
[DisallowMultipleComponent]
public sealed class NativePhysicsWorld : MonoBehaviour
{
    private const string PluginName = "AngryBirdsPhysics";

    [SerializeField] private Vector2 gravity = new Vector2(0.0f, -9.81f);
    [SerializeField, Range(1, 16)] private int substeps = 4;
    [SerializeField, Range(1, 16)] private int solverIterations = 8;

    private readonly List<NativePhysicsBody> bodies = new List<NativePhysicsBody>();
    private IntPtr nativeWorld = IntPtr.Zero;

    [DllImport(PluginName, EntryPoint = "AB_CreateWorld", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr CreateWorldNative(float gravityX, float gravityY);

    [DllImport(PluginName, EntryPoint = "AB_DestroyWorld", CallingConvention = CallingConvention.Cdecl)]
    private static extern void DestroyWorldNative(IntPtr world);

    [DllImport(PluginName, EntryPoint = "AB_CreateCircle", CallingConvention = CallingConvention.Cdecl)]
    private static extern int CreateCircleNative(
        IntPtr world,
        float positionX,
        float positionY,
        float radius,
        float mass,
        float restitution,
        float friction);

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

    [DllImport(PluginName, EntryPoint = "AB_DestroyBody", CallingConvention = CallingConvention.Cdecl)]
    private static extern int DestroyBodyNative(IntPtr world, int bodyId);

    [DllImport(PluginName, EntryPoint = "AB_SetBodyPosition", CallingConvention = CallingConvention.Cdecl)]
    private static extern int SetBodyPositionNative(IntPtr world, int bodyId, float x, float y);

    [DllImport(PluginName, EntryPoint = "AB_SetBodyActive", CallingConvention = CallingConvention.Cdecl)]
    private static extern int SetBodyActiveNative(IntPtr world, int bodyId, int active);

    [DllImport(PluginName, EntryPoint = "AB_ApplyImpulse", CallingConvention = CallingConvention.Cdecl)]
    private static extern int ApplyImpulseNative(IntPtr world, int bodyId, float impulseX, float impulseY);

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

    private void Awake()
    {
        nativeWorld = CreateWorldNative(gravity.x, gravity.y);
        if (nativeWorld == IntPtr.Zero)
        {
            throw new InvalidOperationException("The AngryBirdsPhysics plugin could not create a physics world.");
        }

        NativePhysicsBody[] childBodies = GetComponentsInChildren<NativePhysicsBody>(true);
        foreach (NativePhysicsBody body in childBodies)
        {
            RegisterBody(body);
        }
    }

    private void FixedUpdate()
    {
        if (nativeWorld == IntPtr.Zero)
        {
            return;
        }

        EnsureNativeSuccess(
            StepNative(nativeWorld, Time.fixedDeltaTime, substeps, solverIterations),
            "step the physics world");

        foreach (NativePhysicsBody body in bodies)
        {
            if (body == null)
            {
                continue;
            }

            EnsureNativeSuccess(
                GetBodyStateNative(
                    nativeWorld,
                    body.BodyId,
                    out float positionX,
                    out float positionY,
                    out float velocityX,
                    out float velocityY,
                    out int active),
                "read a body state");

            if (active != 0)
            {
                body.SynchronizeFromNative(
                    new Vector2(positionX, positionY),
                    new Vector2(velocityX, velocityY));
            }
        }
    }

    private void OnDestroy()
    {
        if (nativeWorld == IntPtr.Zero)
        {
            return;
        }

        foreach (NativePhysicsBody body in bodies)
        {
            if (body != null)
            {
                body.DetachFromWorld(this);
            }
        }

        bodies.Clear();
        DestroyWorldNative(nativeWorld);
        nativeWorld = IntPtr.Zero;
    }

    public int RegisterBody(NativePhysicsBody body)
    {
        if (body == null)
        {
            throw new ArgumentNullException(nameof(body));
        }

        if (nativeWorld == IntPtr.Zero)
        {
            throw new InvalidOperationException("A body cannot register before its NativePhysicsWorld is ready.");
        }

        if (bodies.Contains(body))
        {
            return body.BodyId;
        }

        Vector3 position = body.transform.position;
        int bodyId;
        switch (body.Shape)
        {
            case NativePhysicsShape.Circle:
                bodyId = CreateCircleNative(
                    nativeWorld,
                    position.x,
                    position.y,
                    body.Radius,
                    body.Mass,
                    body.Restitution,
                    body.Friction);
                break;
            case NativePhysicsShape.Box:
                bodyId = CreateBoxNative(
                    nativeWorld,
                    position.x,
                    position.y,
                    body.HalfExtents.x,
                    body.HalfExtents.y,
                    body.Mass,
                    body.Restitution,
                    body.Friction);
                break;
            default:
                throw new InvalidOperationException("The native physics body has an unsupported shape.");
        }

        if (bodyId == 0)
        {
            throw new InvalidOperationException(
                $"The AngryBirdsPhysics plugin rejected body '{body.name}'. Check its shape, size, mass, and material values.");
        }

        body.AttachToWorld(this, bodyId);
        bodies.Add(body);
        return bodyId;
    }

    internal void UnregisterBody(NativePhysicsBody body)
    {
        if (body == null || body.BodyId == 0 || nativeWorld == IntPtr.Zero)
        {
            return;
        }

        if (!bodies.Remove(body))
        {
            throw new InvalidOperationException($"Body '{body.name}' is not registered with this physics world.");
        }

        EnsureNativeSuccess(DestroyBodyNative(nativeWorld, body.BodyId), "remove a body");
        body.DetachFromWorld(this);
    }

    internal void SetBodyPosition(int bodyId, Vector2 position)
    {
        EnsureWorldIsReady();
        EnsureNativeSuccess(
            SetBodyPositionNative(nativeWorld, bodyId, position.x, position.y),
            "set a body position");
    }

    internal void SetBodyActive(int bodyId, bool active)
    {
        EnsureWorldIsReady();
        EnsureNativeSuccess(
            SetBodyActiveNative(nativeWorld, bodyId, active ? 1 : 0),
            "change a body's simulation state");
    }

    internal void ApplyImpulse(int bodyId, Vector2 impulse)
    {
        EnsureWorldIsReady();
        EnsureNativeSuccess(
            ApplyImpulseNative(nativeWorld, bodyId, impulse.x, impulse.y),
            "apply an impulse");
    }

    private void EnsureWorldIsReady()
    {
        if (nativeWorld == IntPtr.Zero)
        {
            throw new InvalidOperationException("The native physics world is not available.");
        }
    }

    private static void EnsureNativeSuccess(int result, string operation)
    {
        if (result == 0)
        {
            throw new InvalidOperationException(
                $"The AngryBirdsPhysics plugin could not {operation}. Check the body settings and plugin build.");
        }
    }
}
