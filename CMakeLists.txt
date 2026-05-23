cmake_minimum_required(VERSION 3.14)
project(SkyRealmCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- SDL2 Setup (cross-platform) ----
if(APPLE)
    set(CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local" ${CMAKE_PREFIX_PATH})

    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND)
        set(SDL2_LIB SDL2::SDL2)
    endif()

    find_library(SDL2_TTF_LIB  SDL2_ttf  PATHS /opt/homebrew/lib /usr/local/lib)
    find_library(SDL2_MIXER_LIB SDL2_mixer PATHS /opt/homebrew/lib /usr/local/lib)
    find_path(SDL2_TTF_INC  SDL_ttf.h  PATHS /opt/homebrew/include/SDL2 /usr/local/include/SDL2 /opt/homebrew/include /usr/local/include)
    find_path(SDL2_MIXER_INC SDL_mixer.h PATHS /opt/homebrew/include/SDL2 /usr/local/include/SDL2 /opt/homebrew/include /usr/local/include)

    include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INC} ${SDL2_MIXER_INC} ${CMAKE_SOURCE_DIR}/src)

elseif(WIN32)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(SDL2_mixer CONFIG REQUIRED)
    include_directories(${CMAKE_SOURCE_DIR}/src)
    set(SDL2_LIB SDL2::SDL2)
    set(SDL2_TTF_LIB SDL2::SDL2_ttf)
    set(SDL2_MIXER_LIB SDL2::SDL2_mixer)

else()
    find_package(SDL2 REQUIRED)
    find_library(SDL2_TTF_LIB  SDL2_ttf)
    find_library(SDL2_MIXER_LIB SDL2_mixer)
    find_path(SDL2_TTF_INC  SDL_ttf.h  PATHS /usr/include/SDL2)
    find_path(SDL2_MIXER_INC SDL_mixer.h PATHS /usr/include/SDL2)
    include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INC} ${SDL2_MIXER_INC} ${CMAKE_SOURCE_DIR}/src)
    set(SDL2_LIB ${SDL2_LIBRARIES})
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
target_link_libraries(SkyRealm_Desktop PRIVATE ${SDL2_LIB} ${SDL2_TTF_LIB} ${SDL2_MIXER_LIB})

# ---- Mobile Target ----
add_executable(SkyRealm_Mobile src/main_mobile.cpp ${COMMON_SOURCES})
target_compile_definitions(SkyRealm_Mobile PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(SkyRealm_Mobile PRIVATE ${SDL2_LIB} ${SDL2_TTF_LIB} ${SDL2_MIXER_LIB})

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