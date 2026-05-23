cmake_minimum_required(VERSION 3.14)
project(SkyRealmCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Find SDL2, SDL2_ttf, SDL2_mixer - platform-specific approach
# ============================================================

if(WIN32)
    # ---- Windows: use vcpkg (find_package CONFIG works with SDL2 targets) ----
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(SDL2_mixer CONFIG REQUIRED)

    set(SDL2_LIBS  SDL2::SDL2)
    set(SDL_TTF_LIBS  SDL2::SDL2_ttf)
    set(SDL_MIXER_LIBS SDL2::SDL2_mixer)

elseif(APPLE)
    # ---- macOS: brew provides SDL2 but without SDL2_ttf target. Use sdl2-config. ----
    execute_process(COMMAND sdl2-config --cflags --libs OUTPUT_VARIABLE SDL2_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^-I" "" SDL2_INC "${SDL2_CFLAGS}")
    string(REGEX REPLACE "-lSDL2.*" "" SDL2_INC "${SDL2_INC}")
    string(REGEX REPLACE ".*-I" "" SDL2_INC "${SDL2_INC}")

    execute_process(COMMAND pkg-config --libs sdl2 OUTPUT_VARIABLE SDL2_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND pkg-config --libs SDL2_ttf OUTPUT_VARIABLE TTF_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND pkg-config --libs SDL2_mixer OUTPUT_VARIABLE MIXER_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Brew puts SDL2 in /opt/homebrew on arm64, /usr/local on x86_64
    set(BREW_PREFIX "/opt/homebrew")
    if(NOT EXISTS "${BREW_PREFIX}/include/SDL2/SDL.h")
        set(BREW_PREFIX "/usr/local")
    endif()

    include_directories("${BREW_PREFIX}/include/SDL2" "${BREW_PREFIX}/include" "${CMAKE_SOURCE_DIR}/src")
    link_directories("${BREW_PREFIX}/lib")

    set(SDL2_LIBS  "-lSDL2")
    set(SDL_TTF_LIBS  "-lSDL2_ttf")
    set(SDL_MIXER_LIBS "-lSDL2_mixer")

else()
    # ---- Linux ----
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

    include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/src")

    set(SDL2_LIBS  ${SDL2_LIBRARIES})
    set(SDL_TTF_LIBS  ${SDL2_TTF_LIBRARIES})
    set(SDL_MIXER_LIBS ${SDL2_MIXER_LIBRARIES})
endif()

# ---- Common Source Files ----
set(COMMON_SOURCES
    src/types.h
    src/random.h
    src/world.h        src/world.cpp
    src/game.h         src/game.cpp
    src/renderer.h     src/renderer.cpp
    src/audio.h        src/audio.cpp
    src/input.h        src/input.cpp
)

# ---- Desktop Target ----
add_executable(SkyRealm_Desktop src/main_desktop.cpp ${COMMON_SOURCES})
target_link_libraries(SkyRealm_Desktop PRIVATE ${SDL2_LIBS} ${SDL_TTF_LIBS} ${SDL_MIXER_LIBS})

# ---- Mobile Target ----
add_executable(SkyRealm_Mobile src/main_mobile.cpp ${COMMON_SOURCES})
target_compile_definitions(SkyRealm_Mobile PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(SkyRealm_Mobile PRIVATE ${SDL2_LIBS} ${SDL_TTF_LIBS} ${SDL_MIXER_LIBS})

# ---- Platform-specific defines ----
if(WIN32)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_WINDOWS)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_WINDOWS PLATFORM_MOBILE)
elseif(APPLE)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_MACOS)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_IOS PLATFORM_MOBILE)
    target_link_libraries(SkyRealm_Desktop PRIVATE SDL2::SDL2main)
    target_link_libraries(SkyRealm_Mobile  PRIVATE SDL2::SDL2main)
elseif(ANDROID)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_LINUX)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_ANDROID PLATFORM_MOBILE)
else()
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_LINUX)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_LINUX PLATFORM_MOBILE)
endif()