@echo off
REM ================================================
REM  SkyRealm - 一键构建所有平台
REM  双击此文件即可自动安装依赖并构建 EXE + APK
REM ================================================
setlocal enabledelayedexpansion
set ROOT=%~dp0
set ROOT=%ROOT:~0,-1%

echo.
echo  ==========================================
echo   SkyRealm 苍穹之境 - 一键构建工具
echo  ==========================================
echo.
echo  步骤 1: 检查基础工具
echo  ------------------------------------

:: ---- 检查 Git ----
where git >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [错误] 需要 Git，请从 https://git-scm.com/download/win 下载安装
    pause
    exit /b 1
)
echo [OK] Git 已安装

:: ---- 检查 CMake ----
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [安装] 正在安装 CMake...
    winget install Kitware.CMake --accept-package-agreements --disable-interactivity
    if %ERRORLEVEL% NEQ 0 (
        echo [错误] CMake 安装失败，请手动安装 https://cmake.org/download/
        pause
        exit /b 1
    )
)
echo [OK] CMake 已安装

:: ---- 设置 vcpkg ----
if not exist "C:\vcpkg\vcpkg.exe" (
    echo.
    echo  步骤 2: 安装 vcpkg
    echo  ------------------------------------
    echo [下载] 正在克隆 vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
    if %ERRORLEVEL% NEQ 0 (
        echo [错误] vcpkg 克隆失败，请检查网络连接
        echo [提示] 你可能需要关闭代理或使用 VPN
        pause
        exit /b 1
    )
    echo [构建] 正在编译 vcpkg...
    call C:\vcpkg\bootstrap-vcpkg.bat
    if %ERRORLEVEL% NEQ 0 (
        echo [错误] vcpkg 编译失败
        pause
        exit /b 1
    )
    echo [OK] vcpkg 安装完成
) else (
    echo [OK] vcpkg 已安装
)

:: ---- 安装 SDL2 ----
echo.
echo  步骤 3: 安装 SDL2 开发库
echo  ------------------------------------
C:\vcpkg\vcpkg.exe install sdl2:x64-windows sdl2-ttf:x64-windows sdl2-mixer:x64-windows
if %ERRORLEVEL% NEQ 0 (
    echo [错误] SDL2 安装失败
    pause
    exit /b 1
)
echo [OK] SDL2 安装完成

:: ---- 集成 vcpkg ----
C:\vcpkg\vcpkg.exe integrate install

:: ---- 构建 Windows 桌面版 ----
echo.
echo  步骤 4: 构建 Windows 桌面版 EXE
echo  ------------------------------------
cd /d "%ROOT%"
if not exist "build_win" mkdir build_win
cmake -B build_win -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% NEQ 0 (
    echo [提示] VS2022 不可用，尝试其他生成器...
    cmake -B build_win -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
)
cmake --build build_win --config Release
if %ERRORLEVEL% EQU 0 (
    echo [成功!] Windows EXE 已构建完成
    echo [位置] %ROOT%\build_win\Release\SkyRealm_Desktop.exe
) else (
    echo [错误] 构建失败，查看上方错误信息
)

:: ---- 检查 Android SDK ----
echo.
echo  步骤 5: 准备 Android APK 构建环境
echo  ------------------------------------

set ANDROID_STUDIO="C:\Program Files\Android\Android Studio"
if exist !ANDROID_STUDIO! (
    echo [OK] Android Studio 已安装
) else (
    echo [提示] 未找到 Android Studio，正在安装...
    winget install Google.AndroidStudio --accept-package-agreements --disable-interactivity
)

:: ---- 设置 Android SDK 环境变量 ----
set ANDROID_SDK_ROOT=%LOCALAPPDATA%\Android\Sdk

:: 尝试通过 Android Studio 的 SDK Manager 安装所需组件
echo.
echo  步骤 6: 安装 Android SDK 组件
echo  ------------------------------------
echo [提示] 首次安装需要下载约 2-4GB
echo [操作] 将打开 Android Studio SDK Manager
echo [操作] 请勾选以下组件并点击 Apply：
echo    - Android SDK Platform 34
echo    - Android SDK Build-Tools 34.0.0
echo    - NDK (Side by side) 26.3.11579264
echo    - CMake (从 SDK Tools 选项卡)
echo.
echo 按任意键打开 Android Studio SDK Manager...
pause >nul

:: 启动 Android Studio
start "" "C:\Program Files\Android\Android Studio\bin\studio64.exe"

echo.
echo  步骤 7: 安装完成后，运行以下命令构建 APK：
echo  ------------------------------------
echo   cd %ROOT%\build_tools\android
echo   gradlew assembleRelease
echo.
echo  或直接双击: build_tools\build_android_studio.bat
echo.
echo  ==========================================
echo   构建脚本执行完毕
echo  ==========================================
pause