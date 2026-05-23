cmake_minimum_required(VERSION 3.14)
project(SkyRealmCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- SDL2 Setup ----
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)

# ---- Common Source Files ----
set(COMMON_SOURCES
    src/types.h
    src/random.h
    src/world.h       src/world.cpp
    src/game.h        src/game.cpp
    src/renderer.h    src/renderer.cpp
    src/audio.h       src/audio.cpp
    src/input.h       src/input.cpp
)

# ---- External Libraries ----
include(FetchContent)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# ---- Desktop Target (main_desktop.cpp has its own main()) ----
add_executable(SkyRealm_Desktop
    src/main_desktop.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(SkyRealm_Desktop
    ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# ---- Mobile Target (defined via SDL_main, so undefine main for SDL) ----
add_executable(SkyRealm_Mobile
    src/main_mobile.cpp
    ${COMMON_SOURCES}
)
target_compile_definitions(SkyRealm_Mobile PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(SkyRealm_Mobile
    ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${SDL2_MIXER_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# ---- Platform-specific settings ----
if(WIN32)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_WINDOWS)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_WINDOWS PLATFORM_MOBILE)
elseif(APPLE)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_MACOS)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_IOS PLATFORM_MOBILE)
elseif(ANDROID)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_LINUX)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_ANDROID PLATFORM_MOBILE)
else()
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_LINUX)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_LINUX PLATFORM_MOBILE)
endif()