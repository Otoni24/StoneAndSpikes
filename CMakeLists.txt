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
        src/BattleHud.cpp
        src/BattleHud.h
        src/AnimationPlayer.cpp
        src/AnimationPlayer.h
        src/CombatRules.cpp
        src/CombatRules.h
        src/EnemyPlanner.cpp
        src/EnemyPlanner.h
        src/EnemyAI.cpp
        src/EnemyAI.h
)
if(WIN32)
    set_target_properties(StoneAndSpikes PROPERTIES
            WIN32_EXECUTABLE "$<CONFIG:Release>"
    )
endif()
target_compile_features(StoneAndSpikes PRIVATE cxx_std_20)
target_link_libraries(StoneAndSpikes PRIVATE raylib)

# Keep relative asset paths valid when launching from the build directory.
add_custom_command(TARGET StoneAndSpikes POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:StoneAndSpikes>/assets"
        VERBATIM
)

install(TARGETS StoneAndSpikes
        RUNTIME DESTINATION .
        COMPONENT Game
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        DESTINATION .
        COMPONENT Game
)

if(MINGW)
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    foreach(DLL IN ITEMS
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll
    )
        install(FILES "${MINGW_BIN_DIR}/${DLL}"
                DESTINATION .
                COMPONENT Game
        )
    endforeach()
endif()

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "StoneAndSpikes")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_COMPONENTS_ALL Game)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_ARCHIVE_GAME_FILE_NAME "StoneAndSpikes-Windows")

include(CPack)