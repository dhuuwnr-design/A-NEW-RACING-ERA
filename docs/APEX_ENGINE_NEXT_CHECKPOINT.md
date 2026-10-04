

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


## P0 audit checkpoint — 2026-10-04

- Current source HEAD: `39c9fdee19e609d8c54bd6a29e81260b1d6a1f04`
- P0 items 1–5 implemented, with host-test-driven corrections.
- P0.1: race recovery now measures nearest centreline **segment**, interpolates progress/elevation along that segment, and only pulls the car when segment distance exceeds 7 m.
- P0.2: physics rewritten around body-frame `vx/vy`, bicycle-model slip angles, saturated lateral tyre forces, friction-circle longitudinal force, speed-sensitive/rate-limited steering, and integrated body/world velocity.
- P0.3: touch controls now expose explicit throttle/brake zones, support left/right-hand layouts, tilt steering, steering assist, and correctly handle `ACTION_POINTER_UP` so releasing one finger cannot leave another control stuck. Android's MotionEvent model uses pointer IDs and `ACTION_POINTER_UP` for this lifecycle.
- P0.4: track length/progress recomputation moved into `Race::rebuildTrackMetrics()` and called by construction and `nextTrack()`.
- P0.5: host CMake now builds tests without Android-native library targets; CI no longer hides host-test failures; physics regressions include >500 m travel in 15 s full throttle and <6 g high-speed full-lock.
- Debug overlay added with speed, throttle, brake, steering input, off-track distance, progress, FPS, lateral G, and control mode.
- Authoritative host-test/release run: GitHub Actions run `37227625152` succeeded on commit `3381cd6207b50e3f66dd93b05fe327f680841602`.
- Latest debug APK with real debug telemetry: workflow run `37227849137`, artifact `apex-engine-next-debug-apk`, SHA-256 `bae1fe2035b940fad65d4acd1d841230da79d6512806d8d952cfe540d6c398db`, size 1,648,948 bytes.
- **On-device status:** not yet tested after the P0 changes. The next action is user installation and real driving test. Do not claim P0 device-validated until that happens.
- Do not proceed to P1 until the user reports the on-device result from this build.
