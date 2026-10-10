@echo off
setlocal

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo Visual Studio Installer vswhere.exe was not found.
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL (
    echo Visual Studio C++ x64 build tools were not found.
    exit /b 1
)

call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b %errorlevel%

set "BUILD_DIR=%~dp0build"
set "PLUGIN_DIR=%~dp0..\Assets\Plugins\x86_64"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%PLUGIN_DIR%" mkdir "%PLUGIN_DIR%"

pushd "%~dp0"
cl /nologo /std:c++17 /EHsc /W4 /I src src\PhysicsWorld.cpp tests\PhysicsWorldTests.cpp /Fe:"%BUILD_DIR%\AngryBirdsPhysicsTests.exe"
if errorlevel 1 exit /b %errorlevel%

"%BUILD_DIR%\AngryBirdsPhysicsTests.exe"
if errorlevel 1 exit /b %errorlevel%

cl /nologo /std:c++17 /EHsc /O2 /LD /I include /I src src\PhysicsWorld.cpp src\NativeExports.cpp /link /OUT:"%PLUGIN_DIR%\AngryBirdsPhysics.dll" /IMPLIB:"%BUILD_DIR%\AngryBirdsPhysics.lib"
if errorlevel 1 exit /b %errorlevel%

popd
echo Built Assets\Plugins\x86_64\AngryBirdsPhysics.dll
