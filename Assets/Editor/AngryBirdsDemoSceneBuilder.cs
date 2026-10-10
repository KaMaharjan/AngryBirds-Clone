using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.SceneManagement;

public static class AngryBirdsDemoSceneBuilder
{
    private const string ScenePath = "Assets/Scenes/AngryBirdsDemo.unity";

    [MenuItem("Angry Birds/Create Demo Scene")]
    public static void BuildDemoScene()
    {
        if (File.Exists(ScenePath) &&
            !Application.isBatchMode &&
            !EditorUtility.DisplayDialog(
                "Rebuild demo scene?",
                "This replaces the AngryBirdsDemo scene with the sample level.",
                "Rebuild",
                "Cancel"))
        {
            return;
        }

        Directory.CreateDirectory("Assets/Scenes");
        Scene scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);
        GameObject worldObject = new GameObject("Native Physics World");
        worldObject.AddComponent<NativePhysicsWorld>();

        Camera camera = CreateCamera(worldObject.transform);
        CreateBody(
            "Ground",
            PrimitiveType.Cube,
            worldObject.transform,
            new Vector3(0.0f, -3.0f, 0.0f),
            new Vector3(18.0f, 0.8f, 0.6f),
            NativePhysicsShape.Box,
            new Vector2(9.0f, 0.4f),
            0.35f,
            0.0f,
            0.05f,
            0.8f);
        CreateBody(
            "Left Tower Block",
            PrimitiveType.Cube,
            worldObject.transform,
            new Vector3(2.2f, -1.65f, 0.0f),
            new Vector3(0.5f, 1.9f, 0.6f),
            NativePhysicsShape.Box,
            new Vector2(0.25f, 0.95f),
            0.35f,
            2.0f,
            0.05f,
            0.6f);
        CreateBody(
            "Right Tower Block",
            PrimitiveType.Cube,
            worldObject.transform,
            new Vector3(3.5f, -1.65f, 0.0f),
            new Vector3(0.5f, 1.9f, 0.6f),
            NativePhysicsShape.Box,
            new Vector2(0.25f, 0.95f),
            0.35f,
            2.0f,
            0.05f,
            0.6f);
        CreateBody(
            "Bridge Block",
            PrimitiveType.Cube,
            worldObject.transform,
            new Vector3(2.85f, -0.475f, 0.0f),
            new Vector3(1.8f, 0.45f, 0.6f),
            NativePhysicsShape.Box,
            new Vector2(0.9f, 0.225f),
            0.35f,
            1.5f,
            0.05f,
            0.6f);
        NativePhysicsBody projectile = CreateBody(
            "Bird",
            PrimitiveType.Sphere,
            worldObject.transform,
            new Vector3(-4.0f, -2.2f, 0.0f),
            new Vector3(0.7f, 0.7f, 0.7f),
            NativePhysicsShape.Circle,
            new Vector2(0.35f, 0.35f),
            0.35f,
            1.0f,
            0.15f,
            0.45f);

        GameObject launcherObject = new GameObject("Slingshot Launcher");
        launcherObject.transform.SetParent(worldObject.transform);
        SlingshotLauncher launcher = launcherObject.AddComponent<SlingshotLauncher>();
        SerializedObject launcherSettings = new SerializedObject(launcher);
        launcherSettings.FindProperty("projectile").objectReferenceValue = projectile;
        launcherSettings.FindProperty("targetCamera").objectReferenceValue = camera;
        launcherSettings.FindProperty("anchorPosition").vector2Value = new Vector2(-4.0f, -2.2f);
        launcherSettings.ApplyModifiedPropertiesWithoutUndo();

        if (!EditorSceneManager.SaveScene(scene, ScenePath))
        {
            throw new IOException($"Could not save the demo scene to '{ScenePath}'.");
        }

        List<EditorBuildSettingsScene> buildScenes = new List<EditorBuildSettingsScene>(EditorBuildSettings.scenes);
        int sceneIndex = buildScenes.FindIndex(candidate => candidate.path == ScenePath);
        if (sceneIndex >= 0)
        {
            buildScenes[sceneIndex].enabled = true;
        }
        else
        {
            buildScenes.Add(new EditorBuildSettingsScene(ScenePath, true));
        }
        EditorBuildSettings.scenes = buildScenes.ToArray();

        Debug.Log($"Created the Angry Birds demo scene at {ScenePath}.");
    }

    private static Camera CreateCamera(Transform parent)
    {
        GameObject cameraObject = new GameObject("Main Camera");
        cameraObject.transform.SetParent(parent);
        cameraObject.transform.position = new Vector3(0.0f, 0.0f, -10.0f);
        cameraObject.tag = "MainCamera";

        Camera camera = cameraObject.AddComponent<Camera>();
        camera.orthographic = true;
        camera.orthographicSize = 5.0f;
        camera.backgroundColor = new Color(0.62f, 0.82f, 0.96f);
        camera.clearFlags = CameraClearFlags.SolidColor;
        return camera;
    }

    private static NativePhysicsBody CreateBody(
        string bodyName,
        PrimitiveType visualShape,
        Transform parent,
        Vector3 position,
        Vector3 scale,
        NativePhysicsShape physicsShape,
        Vector2 halfExtents,
        float radius,
        float mass,
        float restitution,
        float friction)
    {
        GameObject bodyObject = GameObject.CreatePrimitive(visualShape);
        bodyObject.name = bodyName;
        bodyObject.transform.SetParent(parent);
        bodyObject.transform.position = position;
        bodyObject.transform.localScale = scale;

        Collider[] colliders = bodyObject.GetComponents<Collider>();
        foreach (Collider collider in colliders)
        {
            Object.DestroyImmediate(collider);
        }

        NativePhysicsBody body = bodyObject.AddComponent<NativePhysicsBody>();
        SerializedObject bodySettings = new SerializedObject(body);
        bodySettings.FindProperty("shape").enumValueIndex = (int)physicsShape;
        bodySettings.FindProperty("halfExtents").vector2Value = halfExtents;
        bodySettings.FindProperty("radius").floatValue = radius;
        bodySettings.FindProperty("mass").floatValue = mass;
        bodySettings.FindProperty("restitution").floatValue = restitution;
        bodySettings.FindProperty("friction").floatValue = friction;
        bodySettings.ApplyModifiedPropertiesWithoutUndo();
        return body;
    }
}
