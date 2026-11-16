cmake_minimum_required(VERSION 3.24)
project(StoneAndSpikes LANGUAGES C CXX)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "Build raylib examples")

FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
)
FetchContent_MakeAvailable(raylib)

add_executable(StoneAndSpikes src/main.cpp
        src/Board.cpp
        src/Board.h
        src/GameConfig.h
        src/Utils.cpp
        src/Utils.h
        src/Battle.cpp
        src/Battle.h
        src/Unit.cpp
        src/Unit.h
        src/GameTypes.h
        src/PlayerController.cpp
        src/PlayerController.h
        src/BattleHud.cpp
        src/BattleHud.h
        src/AnimationPlayer.cpp
        src/AnimationPlayer.h
)
target_compile_features(StoneAndSpikes PRIVATE cxx_std_20)
target_link_libraries(StoneAndSpikes PRIVATE raylib)

# Keep relative asset paths valid when launching from the build directory.
add_custom_command(TARGET StoneAndSpikes POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:StoneAndSpikes>/assets"
        VERBATIM
)
