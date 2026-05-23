@echo off
REM ================================================
REM  SkyRealm Android APK - 一键构建脚本
REM ================================================
echo ================================================
echo  SkyRealm Android APK 构建指南
echo ================================================
echo.
echo [前提] 需要安装:
echo   1. Android Studio (已安装: C:\Program Files\Android\Android Studio)
echo   2. Android SDK + NDK (通过 SDK Manager 安装)
echo.
echo [步骤 1] 打开 Android Studio SDK Manager:
echo   File ^> Settings ^> Languages ^> Android SDK ^> SDK Tools
echo   勾选安装:
echo     - NDK (Side by side) 26.3.11579264
echo     - CMake
echo     - Android SDK Platform 34
echo     - Build-Tools 34.0.0
echo.
echo [步骤 2] 下载 SDL2 Android 库:
echo   从 https://github.com/libsdl-org/SDL/releases 下载 SDL2-2.30.x.zip
echo   解压到 build_tools\android\SDL\
echo   复制 android-project\app\src\main\java\org\libsdl\app\SDLActivity.java
echo.
echo [步骤 3] 构建:
echo   cd build_tools\android
echo   gradlew assembleRelease
echo.
echo [APK 位置]:
echo   build_tools\android\app\build\outputs\apk\release\app-release-unsigned.apk
echo ================================================
echo.
echo 按任意键打开 Android Studio...
pause >nul

if exist "C:\Program Files\Android\Android Studio\bin\studio64.exe" (
    start "" "C:\Program Files\Android\Android Studio\bin\studio64.exe"
) else (
    echo Android Studio 未找到，请检查安装路径
)
pause