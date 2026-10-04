cmake_minimum_required(VERSION 3.22.1)
project(apex)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(apex SHARED main.cpp physics.cpp race.cpp asset_manifest.cpp asset_catalog.cpp track_asset_catalog.cpp glb_loader.cpp glb_mesh.cpp)
target_link_libraries(apex android log EGL GLESv3)

add_executable(apex_physics_test ../../../tests/physics_test.cpp physics.cpp race.cpp asset_manifest.cpp asset_catalog.cpp track_asset_catalog.cpp)
target_include_directories(apex_physics_test PRIVATE .)

add_executable(apex_track_test ../../../tests/track_test.cpp physics.cpp race.cpp asset_manifest.cpp asset_catalog.cpp track_asset_catalog.cpp)
target_include_directories(apex_track_test PRIVATE .)

add_executable(apex_asset_manifest_test ../../../tests/asset_manifest_test.cpp asset_manifest.cpp)
target_include_directories(apex_asset_manifest_test PRIVATE .)

add_executable(apex_asset_catalog_test ../../../tests/asset_catalog_test.cpp asset_catalog.cpp)

add_executable(apex_glb_loader_test ../../../tests/glb_loader_test.cpp glb_loader.cpp)
target_include_directories(apex_asset_catalog_test PRIVATE .)

target_include_directories(apex_glb_loader_test PRIVATE .)

add_executable(apex_track_asset_catalog_test ../../../tests/track_asset_catalog_test.cpp track_asset_catalog.cpp)
target_include_directories(apex_track_asset_catalog_test PRIVATE .)
