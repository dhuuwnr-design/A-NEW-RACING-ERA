

## Playable release verification — 2026-10-04

- Latest verified source HEAD: `1b97ca283f14b012ee99c0c817b68f1e949cdeb3`
- Android release workflow: `.github/workflows/android-release.yml`
- Verified GitHub Actions run: `37225561720` — **success**
- Playable artifact: `apex-engine-next-debug-apk`
- Artifact size: 1,642,029 bytes
- Artifact SHA-256: `1b1d9bdec63401d909f1c59fb646ccaad0f9ba701fc6295b3b1e1dada41c33cf`
- Host C++ test configure/build/run steps completed successfully in the successful release run.
- Android debug APK build completed successfully in the successful release run.
- Earlier release blockers found and fixed during this release pass:
  - Gradle/AGP toolchain mismatch; release workflow and Colab script aligned to Gradle 9.5.0.
  - Android HUD Java compile error: invalid `Path.stroke()`; corrected to `Canvas.drawPath()`.
  - Android CMake test-source paths corrected from `../../../tests` to `../../../../tests`.
  - Native renderer compile errors: duplicate malformed kerb emission removed and escaped literal `\\n` corruption in the AI rendering block restored to real C++ newlines.
- **Important limitation:** this is a CI-built **debug APK**, not yet device-installed/runtime-validated on a physical Android phone. Do not call it fully playable/device-validated until installation and a real race session are completed.
