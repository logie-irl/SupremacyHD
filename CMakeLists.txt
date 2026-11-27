cmake_minimum_required(VERSION 3.20)
project(Supremacy C)

# macOS (README.md has the steps): brew install cmake sdl3, then
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
#   sh tools/make_macos_app.sh / sh tools/make_hd_app.sh for the apps

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

set(CORE_SOURCES
  src/core/game.c
  src/core/economy.c
  src/core/setup.c
  src/core/craft.c
  src/core/combat.c
  src/core/enemy.c
  src/core/events.c
  src/core/day.c
  src/core/cheats.c
)

# The game core as a static library
add_library(supcore STATIC ${CORE_SOURCES})
target_include_directories(supcore PUBLIC src/core)

# ---- sound: the original's sequencer + a Paula mixer ----
add_library(supsound STATIC src/sound/sequencer.c src/sound/paula.c src/sound/endmusic.c)
target_include_directories(supsound PUBLIC src/sound)

# ---- assets: loading the original game files at run time ----
add_library(supassets STATIC src/assets/gamedata.c)
target_include_directories(supassets PUBLIC src/assets)

# ---- ui: the original's screen buffer and text routines ----
file(GLOB UI_SOURCES CONFIGURE_DEPENDS src/ui/*.c)
add_library(supui STATIC ${UI_SOURCES})
target_include_directories(supui PUBLIC src/ui)
target_link_libraries(supui PUBLIC supcore supassets)

# ---- the game (SDL3 front end) ----
# The version banner names the commit built (cmake/build_id.cmake, run at
# every build: build_id.h only changes when the commit or -dirty does).
set(SUP_VERSION "1.0")
add_custom_target(build_id
  COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_SOURCE_DIR} -DOUT=${CMAKE_BINARY_DIR}/gen/build_id.h
          -DVERSION=${SUP_VERSION}
          -P ${CMAKE_SOURCE_DIR}/cmake/build_id.cmake
  BYPRODUCTS ${CMAKE_BINARY_DIR}/gen/build_id.h)
option(SUP_HD "Build the separate SupremacyHD app (own settings folder, hi-res picture packs)" OFF)
set(GAME_SOURCES src/platform/main.c src/platform/launcher.c src/platform/extract.c
                 src/platform/hires.c)
find_package(SDL3 CONFIG QUIET)
if(SDL3_FOUND)
  add_executable(supremacy ${GAME_SOURCES})
  target_link_libraries(supremacy PRIVATE supui supassets supsound supcore SDL3::SDL3)
else()
  message(STATUS "SDL3 not found: the game executable is not built (core library only)")
endif()
if(TARGET supremacy)
  add_dependencies(supremacy build_id)
  target_include_directories(supremacy PRIVATE ${CMAKE_BINARY_DIR}/gen)
  if(SUP_HD)
    target_compile_definitions(supremacy PRIVATE SUP_HD=1)   # main.c and launcher.c
  endif()
endif()
