#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

python3 tools/generate_glb_assets.py

export ANDROID_HOME="${ANDROID_HOME:-/content/android-sdk}"
export ANDROID_SDK_ROOT="$ANDROID_HOME"
export PATH="$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools:$PATH"

sudo_cmd=""
if command -v sudo >/dev/null 2>&1; then sudo_cmd=sudo; fi

$sudo_cmd apt-get update -qq
$sudo_cmd apt-get install -y wget unzip openjdk-17-jdk >/dev/null

mkdir -p "$ANDROID_HOME/cmdline-tools"
if [ ! -x "$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager" ]; then
  wget -q https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip -O /tmp/android-cmdline.zip
  rm -rf "$ANDROID_HOME/cmdline-tools/latest"
  unzip -q /tmp/android-cmdline.zip -d "$ANDROID_HOME/cmdline-tools"
  mv "$ANDROID_HOME/cmdline-tools/cmdline-tools" "$ANDROID_HOME/cmdline-tools/latest"
fi

yes | sdkmanager --licenses >/dev/null 2>&1 || true
sdkmanager "platform-tools" "platforms;android-35" "build-tools;36.0.0" "ndk;27.2.12479018" "cmake;3.22.1" >/dev/null

if [ ! -d /content/gradle-9.5.0 ]; then
  wget -q https://services.gradle.org/distributions/gradle-9.5.0-bin.zip -O /tmp/gradle.zip
  unzip -q /tmp/gradle.zip -d /content
fi
export PATH="/content/gradle-9.5.0/bin:$PATH"

echo "== Host tests =="
rm -rf build-host
cmake -S app/src/main/cpp -B build-host
cmake --build build-host --target apex_physics_test apex_track_test apex_asset_manifest_test apex_asset_catalog_test apex_track_asset_catalog_test apex_glb_loader_test
./build-host/apex_physics_test
./build-host/apex_track_test
./build-host/apex_asset_manifest_test
./build-host/apex_asset_catalog_test
./build-host/apex_track_asset_catalog_test
./build-host/apex_glb_loader_test

echo "== Runtime assets =="
./tools/fetch_cc0_assets.sh

echo "== Android APK =="
gradle --no-daemon :app:assembleDebug

APK="$ROOT/app/build/outputs/apk/debug/app-debug.apk"
test -s "$APK"
echo "APK: $APK"
ls -lh "$APK"
sha256sum "$APK"
