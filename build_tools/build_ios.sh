#!/bin/bash
# ================================================
#  SkyRealm Build Script - iOS IPA
#  Requires: macOS, Xcode, SDL2 frameworks
# ================================================

echo "================================================"
echo " SkyRealm iOS IPA Builder"
echo "================================================"
echo ""
echo "[IMPORTANT] This requires:"
echo "  - macOS with Xcode 14+"
echo "  - SDL2, SDL2_ttf, SDL2_mixer frameworks"
echo ""
echo "Step 1: Install SDL2 frameworks"
echo "  brew install sdl2 sdl2_ttf sdl2_mixer"
echo ""
echo "Step 2: Create Xcode project"
echo "  Copy src/ files into an iOS app target"
echo "  Link frameworks: SDL2.framework, SDL2_ttf.framework, SDL2_mixer.framework"
echo ""
echo "Step 3: Build IPA"
echo "  xcodebuild -project SkyRealm.xcodeproj -scheme SkyRealm_iOS \\"
echo "    -configuration Release -archivePath build/SkyRealm.xcarchive archive"
echo "  xcodebuild -exportArchive -archivePath build/SkyRealm.xcarchive \\"
echo "    -exportPath build/ -exportOptionsPlist ExportOptions.plist"
echo ""
echo "================================================"

echo ""
echo "[INFO] Generating ExportOptions.plist..."
cat > build_tools/ExportOptions.plist << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>method</key>
    <string>development</string>
    <key>teamID</key>
    <string>YOUR_TEAM_ID</string>
</dict>
</plist>
EOF
echo "[INFO] ExportOptions.plist created. Edit with your Team ID."