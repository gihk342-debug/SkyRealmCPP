cmake_minimum_required(VERSION 3.14)
project(SkyRealm_Android)

set(CMAKE_CXX_STANDARD 17)

# ---- SDL2 (from SDL source/android-project) ----
# Assumes SDL2, SDL2_ttf, SDL2_mixer source trees exist at ../../../SDL/
# OR use prebuilt libs from Maven/native
add_library(SDL2 SHARED IMPORTED)
add_library(SDL2_ttf SHARED IMPORTED)
add_library(SDL2_mixer SHARED IMPORTED)

set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../../../SDL/lib/${ANDROID_ABI}/libSDL2.so)
set_target_properties(SDL2_ttf PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../../../SDL_ttf/lib/${ANDROID_ABI}/libSDL2_ttf.so)
set_target_properties(SDL2_mixer PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../../../SDL_mixer/lib/${ANDROID_ABI}/libSDL2_mixer.so)

# ---- Source files ----
add_library(skyrealm_jni SHARED
    ${CMAKE_SOURCE_DIR}/../../../src/types.h
    ${CMAKE_SOURCE_DIR}/../../../src/random.h
    ${CMAKE_SOURCE_DIR}/../../../src/world.h
    ${CMAKE_SOURCE_DIR}/../../../src/world.cpp
    ${CMAKE_SOURCE_DIR}/../../../src/game.h
    ${CMAKE_SOURCE_DIR}/../../../src/game.cpp
    ${CMAKE_SOURCE_DIR}/../../../src/renderer.h
    ${CMAKE_SOURCE_DIR}/../../../src/renderer.cpp
    ${CMAKE_SOURCE_DIR}/../../../src/audio.h
    ${CMAKE_SOURCE_DIR}/../../../src/audio.cpp
    ${CMAKE_SOURCE_DIR}/../../../src/input.h
    ${CMAKE_SOURCE_DIR}/../../../src/input.cpp
    ${CMAKE_SOURCE_DIR}/../../../src/main_mobile.cpp
)

target_include_directories(skyrealm_jni PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../SDL/include
    ${CMAKE_SOURCE_DIR}/../../../SDL_ttf/include
    ${CMAKE_SOURCE_DIR}/../../../SDL_mixer/include
    ${CMAKE_SOURCE_DIR}/../../../src
)

target_compile_definitions(skyrealm_jni PRIVATE PLATFORM_ANDROID PLATFORM_MOBILE)

target_link_libraries(skyrealm_jni
    SDL2 SDL2_ttf SDL2_mixer
    android log
)