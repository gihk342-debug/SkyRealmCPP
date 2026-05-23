cmake_minimum_required(VERSION 3.14)
project(SkyRealmCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- SDL2 Setup (cross-platform) ----
if(APPLE)
    # macOS: brew installs SDL2 with .pc files
    set(CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local" ${CMAKE_PREFIX_PATH})
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS}
        /opt/homebrew/include /usr/local/include
        ${CMAKE_SOURCE_DIR}/src)
elseif(WIN32)
    # Windows: vcpkg provides SDL2 easily
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(SDL2_mixer CONFIG REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
else()
    # Linux
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src)
endif()

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

# ---- Desktop Target ----
add_executable(SkyRealm_Desktop
    src/main_desktop.cpp
    ${COMMON_SOURCES}
)
target_link_libraries(SkyRealm_Desktop PRIVATE
    SDL2::SDL2 SDL2::SDL2_ttf SDL2::SDL2_mixer
)

# ---- Mobile Target ----
add_executable(SkyRealm_Mobile
    src/main_mobile.cpp
    ${COMMON_SOURCES}
)
target_compile_definitions(SkyRealm_Mobile PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(SkyRealm_Mobile PRIVATE
    SDL2::SDL2 SDL2::SDL2_ttf SDL2::SDL2_mixer
)

# ---- Platform-specific defines ----
if(WIN32)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_WINDOWS)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_WINDOWS PLATFORM_MOBILE)
elseif(APPLE)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_MACOS)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_IOS PLATFORM_MOBILE)
    # SDL2 needs main on macOS
    target_link_libraries(SkyRealm_Desktop PRIVATE SDL2::SDL2main)
    target_link_libraries(SkyRealm_Mobile  PRIVATE SDL2::SDL2main)
elseif(ANDROID)
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_LINUX)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_ANDROID PLATFORM_MOBILE)
else()
    target_compile_definitions(SkyRealm_Desktop PRIVATE PLATFORM_LINUX)
    target_compile_definitions(SkyRealm_Mobile  PRIVATE PLATFORM_LINUX PLATFORM_MOBILE)
endif()