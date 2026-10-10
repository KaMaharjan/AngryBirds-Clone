using System;
using UnityEngine;

public enum NativePhysicsShape
{
    Circle,
    Box
}

[DisallowMultipleComponent]
public sealed class NativePhysicsBody : MonoBehaviour
{
    [SerializeField] private NativePhysicsShape shape = NativePhysicsShape.Circle;
    [SerializeField, Min(0.01f)] private float radius = 0.35f;
    [SerializeField] private Vector2 halfExtents = new Vector2(0.5f, 0.5f);
    [SerializeField, Min(0.0f)] private float mass = 1.0f;
    [SerializeField, Range(0.0f, 1.0f)] private float restitution = 0.1f;
    [SerializeField, Min(0.0f)] private float friction = 0.5f;

    private NativePhysicsWorld physicsWorld;

    internal int BodyId { get; private set; }
    internal NativePhysicsShape Shape => shape;
    internal float Radius => radius;
    internal Vector2 HalfExtents => halfExtents;
    internal float Mass => mass;
    internal float Restitution => restitution;
    internal float Friction => friction;
    public Vector2 Velocity { get; private set; }

    private void Start()
    {
        if (BodyId != 0)
        {
            return;
        }

        NativePhysicsWorld parentWorld = GetComponentInParent<NativePhysicsWorld>();
        if (parentWorld == null)
        {
            throw new InvalidOperationException(
                $"Body '{name}' must be a child of a GameObject with a NativePhysicsWorld component.");
        }

        parentWorld.RegisterBody(this);
    }

    private void OnDestroy()
    {
        if (physicsWorld != null)
        {
            physicsWorld.UnregisterBody(this);
        }
    }

    public void SetSimulationEnabled(bool enabled)
    {
        EnsureRegistered();
        physicsWorld.SetBodyActive(BodyId, enabled);
    }

    public void Teleport(Vector2 position)
    {
        EnsureRegistered();
        Vector3 currentPosition = transform.position;
        transform.position = new Vector3(position.x, position.y, currentPosition.z);
        physicsWorld.SetBodyPosition(BodyId, position);
    }

    public void Launch(Vector2 impulse)
    {
        EnsureRegistered();
        if (mass <= 0.0f)
        {
            throw new InvalidOperationException($"Static body '{name}' cannot be launched.");
        }

        Vector3 currentPosition = transform.position;
        physicsWorld.SetBodyPosition(BodyId, new Vector2(currentPosition.x, currentPosition.y));
        physicsWorld.ApplyImpulse(BodyId, impulse);
        physicsWorld.SetBodyActive(BodyId, true);
    }

    internal void AttachToWorld(NativePhysicsWorld world, int bodyId)
    {
        if (physicsWorld != null && physicsWorld != world)
        {
            throw new InvalidOperationException($"Body '{name}' is already registered with another world.");
        }

        physicsWorld = world;
        BodyId = bodyId;
    }

    internal void DetachFromWorld(NativePhysicsWorld world)
    {
        if (physicsWorld != world)
        {
            return;
        }

        physicsWorld = null;
        BodyId = 0;
    }

    internal void SynchronizeFromNative(Vector2 position, Vector2 velocity)
    {
        Velocity = velocity;
        Vector3 currentPosition = transform.position;
        transform.position = new Vector3(position.x, position.y, currentPosition.z);
    }

    private void EnsureRegistered()
    {
        if (physicsWorld == null || BodyId == 0)
        {
            throw new InvalidOperationException($"Body '{name}' is not registered with a NativePhysicsWorld.");
        }
    }
}
