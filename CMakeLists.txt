cmake_minimum_required(VERSION 3.14)
project(SkyRealmCPP LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# ---- SDL2 configuration ----
if(WIN32)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(SDL2_mixer CONFIG REQUIRED)
    include_directories("${CMAKE_SOURCE_DIR}/src")
    # vcpkg uses these exact target names
    set(SDL2_TARGETS SDL2::SDL2 SDL2::SDL2main SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
elseif(APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2    REQUIRED sdl2)
    pkg_check_modules(SDL_TTF  REQUIRED SDL2_ttf)
    pkg_check_modules(SDL_MIX REQUIRED SDL2_mixer)
    include_directories(${SDL2_INCLUDE_DIRS} ${SDL_TTF_INCLUDE_DIRS} ${SDL_MIX_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/src")
    link_directories(${SDL2_LIBRARY_DIRS} ${SDL_TTF_LIBRARY_DIRS} ${SDL_MIX_LIBRARY_DIRS})
    set(SDL2_TARGETS ${SDL2_LIBRARIES} ${SDL_TTF_LIBRARIES} ${SDL_MIX_LIBRARIES})
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2    REQUIRED sdl2)
    pkg_check_modules(SDL_TTF  REQUIRED SDL2_ttf)
    pkg_check_modules(SDL_MIX REQUIRED SDL2_mixer)
    include_directories(${SDL2_INCLUDE_DIRS} ${SDL_TTF_INCLUDE_DIRS} ${SDL_MIX_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/src")
    set(SDL2_TARGETS ${SDL2_LIBRARIES} ${SDL_TTF_LIBRARIES} ${SDL_MIX_LIBRARIES})
endif()

# ---- Source files ----
set(SOURCES src/world.cpp src/game.cpp src/renderer.cpp src/audio.cpp src/input.cpp)

# ---- Desktop ----
add_executable(SkyRealm_Desktop src/main_desktop.cpp ${SOURCES})
target_link_libraries(SkyRealm_Desktop PRIVATE ${SDL2_TARGETS})

# ---- Mobile ----
add_executable(SkyRealm_Mobile src/main_mobile.cpp ${SOURCES})
target_compile_definitions(SkyRealm_Mobile PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(SkyRealm_Mobile PRIVATE ${SDL2_TARGETS})

# ---- Platform defines ----
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