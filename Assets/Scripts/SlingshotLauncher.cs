using System;
using UnityEngine;

[DisallowMultipleComponent]
public sealed class SlingshotLauncher : MonoBehaviour
{
    [SerializeField] private NativePhysicsBody projectile;
    [SerializeField] private Camera targetCamera;
    [SerializeField] private Vector2 anchorPosition = new Vector2(-4.0f, -2.2f);
    [SerializeField, Min(0.1f)] private float maxPullDistance = 2.5f;
    [SerializeField, Min(0.1f)] private float pickRadius = 0.8f;
    [SerializeField, Min(0.0f)] private float launchImpulse = 10.0f;

    private bool isDragging;

    private void Awake()
    {
        if (projectile == null)
        {
            projectile = GetComponent<NativePhysicsBody>();
        }

        if (projectile == null)
        {
            throw new InvalidOperationException("SlingshotLauncher needs a reference to a NativePhysicsBody projectile.");
        }

        if (targetCamera == null)
        {
            targetCamera = Camera.main;
        }

        if (targetCamera == null)
        {
            throw new InvalidOperationException("SlingshotLauncher needs a camera to convert mouse input to world space.");
        }
    }

    private void Update()
    {
        if (!isDragging && Input.GetMouseButtonDown(0))
        {
            BeginDragIfSelected();
        }

        if (!isDragging)
        {
            return;
        }

        Vector2 draggedPosition = GetClampedPointerPosition();
        Vector3 currentPosition = projectile.transform.position;
        projectile.transform.position = new Vector3(draggedPosition.x, draggedPosition.y, currentPosition.z);

        if (Input.GetMouseButtonUp(0))
        {
            isDragging = false;
            projectile.Launch((anchorPosition - draggedPosition) * launchImpulse);
        }
    }

    private void BeginDragIfSelected()
    {
        Vector2 pointerPosition = GetPointerWorldPosition();
        Vector2 projectilePosition = projectile.transform.position;
        if ((pointerPosition - projectilePosition).sqrMagnitude > pickRadius * pickRadius)
        {
            return;
        }

        projectile.SetSimulationEnabled(false);
        isDragging = true;
    }

    private Vector2 GetClampedPointerPosition()
    {
        Vector2 dragOffset = GetPointerWorldPosition() - anchorPosition;
        if (dragOffset.sqrMagnitude > maxPullDistance * maxPullDistance)
        {
            dragOffset = dragOffset.normalized * maxPullDistance;
        }

        return anchorPosition + dragOffset;
    }

    private Vector2 GetPointerWorldPosition()
    {
        float depth = projectile.transform.position.z - targetCamera.transform.position.z;
        Vector3 screenPosition = Input.mousePosition;
        screenPosition.z = depth;
        Vector3 worldPosition = targetCamera.ScreenToWorldPoint(screenPosition);
        return new Vector2(worldPosition.x, worldPosition.y);
    }
}
