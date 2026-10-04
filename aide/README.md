# Apex Engine Next — AIDE build target

This is an **on-device AIDE build target** for Apex Engine Next. It does not replace the canonical Android/Gradle project in `app/` and does not change the C++ engine architecture.

## Why this exists

The canonical project currently uses Android Gradle Plugin 8.7.3 + CMake. AIDE's official documentation says it can develop NDK C/C++ apps and open basic Android Studio projects, but its Android Studio interoperability page also says the full Gradle build system is not supported. Therefore this target uses the older `ndk-build`/Android.mk route for the same native C++ sources.

The native engine remains in:

`app/src/main/cpp/`

The AIDE target only supplies a mobile-friendly packaging/build wrapper around those sources.

## Open in AIDE

Open the `aide/` directory as an Android project.

The target contains:
- `AndroidManifest.xml`
- `src/com/apexenginenext/MainActivity.java`
- `res/values/styles.xml`
- `jni/Android.mk`
- `jni/Application.mk`
- `project.properties`

The Android.mk points back to the canonical C++ sources under `../app/src/main/cpp/`, so there is no second copy of the engine.

## Assets

The canonical runtime assets live under `app/src/main/assets/`. This AIDE target starts with procedural fallback scenery if GLB assets are not copied into `aide/assets/`.

For a full visual build, copy the staged runtime GLBs into the matching `aide/assets/cars/` and `aide/assets/environment/` paths before running.

## Important

This target is a build compatibility layer only. The main Gradle/CMake project remains the source of truth for normal/CI/Colab builds.
