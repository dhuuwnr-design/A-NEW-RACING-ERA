# Apex Engine Next

Android-first custom C++ racing game foundation targeting the uploaded F1-style reference experience.

## Current checkpoint: 2.6

The current branch is a native C++/OpenGL ES Android foundation with a procedural elevated circuit, four-wheel physics, seven AI cars, five-lap race state, touch controls, cockpit/chase cameras and a broadcast-style timing tower.

### 2.1 work completed

- fixed-step 120 Hz physics loop retained for deterministic vehicle updates
- renderer hot path optimized by deriving triangle normals in the fragment shader instead of rebuilding a CPU normal buffer every frame
- procedural 240-point elevated circuit with variable width, kerbs, edge lines, barriers and vegetation
- four-wheel load/slip telemetry, combined grip, longitudinal/lateral load transfer, downforce/drag and chassis pitch/roll
- seven AI cars with curvature-aware braking, predictive corner speed and staggered racing-line offsets
- native 3-2-1-GO race start, five-lap completion, position, gap and interval telemetry
- progressive Android steering input, throttle/brake touch zones and cockpit/chase camera toggle
- research log for F1 gameplay references and CC0/public-domain asset candidates

### Important verification rule

GitLab is the project source of truth. Continuation follows: inspect -> research -> implement -> verify -> commit -> read back.

A GitLab pipeline marked failed is not treated as an APK/runtime test. The latest pipelines are currently failing before usable jobs are exposed, so Android runtime validation is still pending. Host-side test sources remain in tests/.

## Next implementation phase

1. native glTF/GLB asset ingestion test
2. one CC0 open-wheel car asset integrated alongside the procedural car
3. material/texture path with mobile-friendly texture limits
4. stronger lighting, shadows and environment depth
5. richer cockpit/HUD telemetry
6. track-specific data and multiple circuits
7. menus, setup, results and settings
8. tyres, ERS, fuel, weather, damage, pit stops and progression


### 2.6 work completed
- fixed two native renderer build blockers in the 2.4 renderer path: runoff/gravel buffers are declared and the JNI race-time symbol now matches the Java native declaration
- added a dependency-free native GLB 2.0 container parser with bounds checking for JSON and BIN chunks
- added a synthetic GLB regression test and included it in the host verification set
- added a phone-friendly `tools/colab_build.sh` that installs the same Android SDK/NDK/CMake versions used by CI, runs host tests, and builds the debug APK
- expanded CI host verification to run the physics, track, asset-manifest, asset-catalog, track-asset-catalog and GLB loader tests
- added a circuit-specific landmark scenery pass for Spa, Silverstone, Monza, Suzuka, Bahrain, Interlagos and COTA, including venue-specific structure/sign silhouettes and desert floodlight dressing

### Verification status
The source-level changes have been committed and read back from GitLab. The GitLab runner is still blocked by account identity verification, so these changes are **not yet claimed as runtime/build verified** until the Colab build or another runnable Android environment completes successfully.
