@echo off
REM ================================================
REM  SkyRealm Android APK 一键构建
REM ================================================
setlocal

set ROOT=%~dp0
set ANDROID_SDK=%LOCALAPPDATA%\Android\Sdk
set NDK_VERSION=26.3.11579264

echo ================================================
echo  SkyRealm Android APK 构建脚本
echo ================================================
echo.

REM ---- Check Android SDK ----
if not exist "%ANDROID_SDK%" (
    echo [错误] Android SDK 未找到: %ANDROID_SDK%
    echo [操作] 请先运行 Android Studio 并通过 SDK Manager 安装:
    echo   - Android SDK Platform 34
    echo   - Build-Tools 34.0.0
    echo   - NDK 26.3.11579264
    pause
    exit /b 1
)

echo [OK] Android SDK: %ANDROID_SDK%

REM ---- Set environment ----
set ANDROID_HOME=%ANDROID_SDK%
set ANDROID_SDK_ROOT=%ANDROID_SDK%
set ANDROID_NDK_HOME=%ANDROID_SDK%\ndk\%NDK_VERSION%

if not exist "%ANDROID_NDK_HOME%" (
    echo [错误] NDK %NDK_VERSION% 未安装
    echo [路径] %ANDROID_NDK_HOME%
    echo [操作] 请在 Android Studio SDK Manager 中安装 NDK
    pause
    exit /b 1
)

echo [OK] NDK: %ANDROID_NDK_HOME%

REM ---- Download SDL2 Android binaries (if not present) ----
set SDL_DIR=%ROOT%SDL
if not exist "%SDL_DIR%" (
    echo.
    echo [下载] 正在获取 SDL2 Android 预编译库...
    echo [提示] 需要从 SDL 官网下载或通过 GitHub Releases 获取
    echo [操作] 请访问 https://github.com/libsdl-org/SDL/releases
    echo [操作] 下载 SDL2-2.30.x.zip 并解压到此目录
    echo [操作] Android 项目模板在 SDL2-2.30.x/android-project/
    echo.
    echo [文件结构]:
    echo   %SDL_DIR%\include\SDL.h
    echo   %SDL_DIR%\lib\arm64-v8a\libSDL2.so
    echo   %SDL_DIR%\lib\armeabi-v7a\libSDL2.so
    echo.
    echo 按任意键继续（将跳过 SDL 下载）...
    pause >nul
)

REM ---- Build ----
echo.
echo [构建] 正在编译 APK...

cd /d "%ROOT%"
call gradlew assembleRelease

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ================================================
    echo  构建成功!
    echo  APK 文件: %ROOT%app\build\outputs\apk\release\app-release-unsigned.apk
    echo ================================================
) else (
    echo [错误] 构建失败
)

pause