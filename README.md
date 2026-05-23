# SkyRealmCPP - Cross-Platform Build Instructions

## Prerequisites

### Windows (Desktop)
1. Install **Visual Studio 2022** (Community) with C++ workload
2. Install **vcpkg**:
   ```
   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg && bootstrap-vcpkg.bat
   vcpkg install sdl2:x64-windows sdl2-ttf:x64-windows sdl2-mixer:x64-windows
   vcpkg integrate install
   ```
3. Run: `build_tools\build_windows.bat`

### macOS (Desktop)
```
brew install cmake sdl2 sdl2_ttf sdl2_mixer
cd ~/Desktop/SkyRealmCPP
mkdir -p build && cd build
cmake .. && make
./SkyRealm_Desktop
```

### Linux (Desktop)
```
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev cmake
cd ~/Desktop/SkyRealmCPP
mkdir -p build && cd build
cmake .. && make
./SkyRealm_Desktop
```

### Android (APK)
1. Install Android Studio + NDK
2. Use SDL2's android-project template
3. Copy src/ into app/src/main/cpp/
4. Configure CMakeLists.txt
5. `./gradlew assembleRelease`

### iOS (IPA)
1. On macOS with Xcode
2. `brew install sdl2 sdl2_ttf sdl2_mixer`
3. Create iOS app target, link frameworks
4. Archive via Xcode

## Controls

| Key | Action |
|-----|--------|
| WASD / Arrows | Move |
| Space | Attack |
| E | Interact |
| 1/2/3 | Skills |
| I | Inventory |
| M | Map |
| Esc | Close panels |
| H | Use potion |
| G | Use mana gem |

## Project Structure

```
SkyRealmCPP/
├── CMakeLists.txt          # Build config
├── src/
│   ├── types.h            # All data structures
│   ├── random.h           # PRNG (xorshift)
│   ├── world.h/cpp        # World generation
│   ├── game.h/cpp         # Game logic & mechanics
│   ├── renderer.h/cpp     # SDL2 rendering
│   ├── audio.h/cpp        # Sound effects
│   ├── input.h/cpp        # Input handling
│   ├── main_desktop.cpp   # Desktop entry point
│   └── main_mobile.cpp    # Mobile entry point (with joystick)
├── assets/                 # (future: sprites, fonts)
├── mobile/                 # Mobile-specific resources
└── build_tools/
    ├── build_windows.bat   # Windows build script
    ├── build_android.bat   # Android APK guide
    └── build_ios.sh        # iOS IPA guide
```