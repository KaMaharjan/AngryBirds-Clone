cmake_minimum_required(VERSION 3.16)
project(AngryBirdsPhysics LANGUAGES CXX)

include(CTest)

add_library(AngryBirdsPhysics SHARED
    src/PhysicsWorld.cpp
    src/NativeExports.cpp
)
target_compile_features(AngryBirdsPhysics PRIVATE cxx_std_17)
target_include_directories(AngryBirdsPhysics
    PUBLIC include
    PRIVATE src
)
set_target_properties(AngryBirdsPhysics PROPERTIES OUTPUT_NAME AngryBirdsPhysics)

if(MSVC)
    target_compile_options(AngryBirdsPhysics PRIVATE /W4 /permissive-)
else()
    target_compile_options(AngryBirdsPhysics PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(AngryBirdsPhysicsTests
        src/PhysicsWorld.cpp
        tests/PhysicsWorldTests.cpp
    )
    target_compile_features(AngryBirdsPhysicsTests PRIVATE cxx_std_17)
    target_include_directories(AngryBirdsPhysicsTests PRIVATE src)
    if(MSVC)
        target_compile_options(AngryBirdsPhysicsTests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(AngryBirdsPhysicsTests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME AngryBirdsPhysicsTests COMMAND AngryBirdsPhysicsTests)
endif()
