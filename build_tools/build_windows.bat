@echo off
REM ================================================
REM  SkyRealm Build Script - Windows (Desktop)
REM ================================================

REM ---- Configuration ----
set BUILD_DIR=build_windows
set EXE_NAME=SkyRealm_Desktop.exe
set SDL2_DIR=C:\SDL2

REM ---- Find SDL2 ----
if not exist "%SDL2_DIR%" (
    echo [INFO] SDL2 not found at %SDL2_DIR%, trying vcpkg...
    where vcpkg >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        echo [INFO] Installing SDL2 via vcpkg...
        vcpkg install sdl2 sdl2-ttf sdl2-mixer --triplet x64-windows
    )
)

REM ---- Build with CMake ----
echo [INFO] Configuring CMake...
cmake -B %BUILD_DIR% -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed.
    echo [HINT] Make sure SDL2, SDL2_ttf, SDL2_mixer are installed.
    echo [HINT] Install via vcpkg: vcpkg install sdl2 sdl2-ttf sdl2-mixer --triplet x64-windows
    pause
    exit /b 1
)

echo [INFO] Building...
cmake --build %BUILD_DIR% --config Release

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build complete!
    echo [INFO] Executable: %BUILD_DIR%\Release\%EXE_NAME%
) else (
    echo [ERROR] Build failed.
)

pause