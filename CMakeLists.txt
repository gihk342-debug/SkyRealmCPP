cmake_minimum_required(VERSION 3.14)
project(SkyRealmCPP LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# ============================================================
# SDL2 detection - use sdl2-config everywhere (most reliable)
# ============================================================

# Try sdl2-config first (works on macOS/Linux/MinGW)
find_program(SDL2_CONFIG sdl2-config)
if(SDL2_CONFIG)
    execute_process(COMMAND ${SDL2_CONFIG} --cflags OUTPUT_VARIABLE SDL2_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${SDL2_CONFIG} --libs   OUTPUT_VARIABLE SDL2_LIBS   OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${SDL2_CONFIG} --prefix OUTPUT_VARIABLE SDL2_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

    separate_arguments(SDL2_CFLAGS)
    separate_arguments(SDL2_LIBS)

    # Find SDL2_ttf and SDL2_mixer headers
    find_path(SDL2_INC_DIR SDL.h PATHS "${SDL2_PREFIX}/include/SDL2" "${SDL2_PREFIX}/include" NO_DEFAULT_PATH)
    find_path(SDL_TTF_INC SDL_ttf.h PATHS "${SDL2_PREFIX}/include/SDL2" "${SDL2_PREFIX}/include")
    find_path(SDL_MIXER_INC SDL_mixer.h PATHS "${SDL2_PREFIX}/include/SDL2" "${SDL2_PREFIX}/include")

    include_directories(${SDL2_INC_DIR} ${SDL_TTF_INC} ${SDL_MIXER_INC} "${CMAKE_SOURCE_DIR}/src")

    # Use -lSDL2_ttf -lSDL2_mixer
    set(ALL_SDL_LIBS ${SDL2_LIBS} -lSDL2_ttf -lSDL2_mixer)
    if(APPLE)
        set(ALL_SDL_LIBS ${ALL_SDL_LIBS} -framework Cocoa -framework IOKit -framework CoreFoundation)
    endif()
else()
    # Fallback: Windows vcpkg
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(SDL2_mixer CONFIG REQUIRED)
    include_directories("${CMAKE_SOURCE_DIR}/src")
    set(ALL_SDL_LIBS SDL2::SDL2 SDL2::SDL2_ttf SDL2::SDL2_mixer)
endif()

# ---- Source files ----
set(SOURCES src/world.cpp src/game.cpp src/renderer.cpp src/audio.cpp src/input.cpp)

# ---- Desktop ----
add_executable(SkyRealm_Desktop src/main_desktop.cpp ${SOURCES})
target_link_libraries(SkyRealm_Desktop PRIVATE ${ALL_SDL_LIBS})

# ---- Mobile ----
add_executable(SkyRealm_Mobile src/main_mobile.cpp ${SOURCES})
target_compile_definitions(SkyRealm_Mobile PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(SkyRealm_Mobile PRIVATE ${ALL_SDL_LIBS})

# Platform defines
if(WIN32)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_WINDOWS)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_WINDOWS PLATFORM_MOBILE)
elseif(APPLE)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_MACOS)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_IOS PLATFORM_MOBILE)
else()
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_LINUX)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_LINUX PLATFORM_MOBILE)
endif()