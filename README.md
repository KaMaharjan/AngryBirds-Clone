# AngryBirds-Clone

A small Unity 2D slingshot demo backed by a native C++ physics plugin. Unity gameplay components are C#; Unity does not attach C++ source files as `MonoBehaviour` scripts. The C++ solver is compiled as a native plugin and called through the C# bridge.

## Open and play

1. Open this folder in Unity **2022.3.36f1** or a compatible 2022.3 LTS editor.
2. If the sample scene is not present, choose **Angry Birds > Create Demo Scene** in the editor.
3. Open `Assets/Scenes/AngryBirdsDemo.unity` and press **Play**.
4. Drag the bird away from the slingshot anchor and release to launch it into the block tower.

The checked-in Windows x86-64 plugin is `Assets/Plugins/x86_64/AngryBirdsPhysics.dll`. To rebuild it and run the native tests, run `NativePlugin\build_windows.bat` from a Visual Studio x64 C++ tools command prompt (or double-click it). The script also locates Visual Studio's C++ tools when run from a regular command prompt.

## What is included

- `NativePlugin\src\PhysicsWorld.cpp` implements gravity, substeps, circle/box and box/box collision detection, impulse response, restitution, friction, and overlap correction.
- `NativePlugin\src\NativeExports.cpp` exposes the solver through a C ABI suitable for Unity P/Invoke.
- `Assets\Scripts\NativePhysicsWorld.cs` owns the native world and synchronizes Unity transforms.
- `Assets\Scripts\NativePhysicsBody.cs` maps Unity objects to native circles or axis-aligned boxes; mass `0` makes a body static.
- `Assets\Scripts\SlingshotLauncher.cs` handles mouse aiming and applies a launch impulse.
- `NativePlugin\tests\PhysicsWorldTests.cpp` checks gravity, floor collision, and wall collision.
- Choose **Angry Birds > Test Native Plugin** to run an in-editor native-plugin smoke test.

The solver is intentionally small: bodies translate but do not rotate, boxes stay axis-aligned, and collision detection is discrete. Use fixed-step substeps to reduce tunneling; this is a learning/demo implementation rather than a replacement for Unity's production physics engine.

## Rebuild with CMake

With CMake installed, build the plugin and tests with:

```powershell
cmake -S NativePlugin -B NativePlugin/build -A x64
cmake --build NativePlugin/build --config Release
ctest --test-dir NativePlugin/build -C Release --output-on-failure
```

Copy the generated Windows DLL from `NativePlugin\build\Release\AngryBirdsPhysics.dll` to `Assets\Plugins\x86_64\AngryBirdsPhysics.dll` after a CMake build. The Windows batch script performs the build, test run, and copy into the Unity plugin folder in one step.
