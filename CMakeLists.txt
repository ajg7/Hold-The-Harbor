cmake_minimum_required(VERSION 3.28)

project(HoldTheHarbor
    VERSION 0.1.0
    DESCRIPTION "Single-screen pixel-art pirate brawler (raylib / C++23)"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(HTH_BUILD_TESTS "Build the doctest unit tests" ON)
option(HTH_ENABLE_ASAN "Build with AddressSanitizer" OFF)

# ---------------------------------------------------------------------------
# Dependencies (FetchContent, pinned to exact tags)
# ---------------------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# raylib: build only the library, no examples/games.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.2
    GIT_SHALLOW    TRUE)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(raylib nlohmann_json)

if(HTH_BUILD_TESTS)
    set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.5.3
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(doctest)
endif()

# ---------------------------------------------------------------------------
# Shared compile settings
# ---------------------------------------------------------------------------
add_library(hth_options INTERFACE)
target_compile_features(hth_options INTERFACE cxx_std_23)

if(MSVC)
    target_compile_options(hth_options INTERFACE /W4 /permissive- /utf-8 /Zc:__cplusplus)
    if(HTH_ENABLE_ASAN)
        target_compile_options(hth_options INTERFACE /fsanitize=address /Zi)
        # ASan on MSVC is incompatible with incremental linking and /RTC.
        target_link_options(hth_options INTERFACE /INCREMENTAL:NO)
        string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    endif()
else()
    target_compile_options(hth_options INTERFACE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
    if(HTH_ENABLE_ASAN)
        target_compile_options(hth_options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(hth_options INTERFACE -fsanitize=address)
    endif()
endif()

# ---------------------------------------------------------------------------
# Game code: a library (so tests can link it) plus a thin executable.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE HTH_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER HTH_SOURCES EXCLUDE REGEX ".*/src/main\\.cpp$")

add_library(hth_core STATIC ${HTH_SOURCES})
target_include_directories(hth_core PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(hth_core PUBLIC raylib nlohmann_json::nlohmann_json hth_options)
target_compile_definitions(hth_core PUBLIC
    HTH_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets")

add_executable(hold_the_harbor src/main.cpp)
target_link_libraries(hold_the_harbor PRIVATE hth_core)
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT hold_the_harbor)
set_target_properties(hold_the_harbor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

if(HTH_BUILD_TESTS)
    enable_testing()
    file(GLOB_RECURSE HTH_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/*.cpp")
    add_executable(hth_tests ${HTH_TEST_SOURCES})
    target_link_libraries(hth_tests PRIVATE hth_core doctest::doctest)
    add_test(NAME hth_tests COMMAND hth_tests)
endif()
