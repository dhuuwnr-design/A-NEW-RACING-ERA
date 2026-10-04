# Apex Engine Next — Durable Project Checkpoint

Checkpoint created: 2026-10-04
Repository: dhuuwnr-design/A-NEW-RACING-ERA
Branch: main
Verified HEAD before this checkpoint: 313a8d8a0fd56ce9fad4a186cd9ea8420cfd24c8

## Resume rule

When the user says “Continue”, “Continue Apex Engine Next”, “work on this project”, or equivalent, start from this repository checkpoint immediately.
Do not restart from a generic plan. Do not invent repository state. First inspect this checkpoint, then inspect the current GitHub tree/HEAD and the files relevant to the next task. Treat the repository as the source of truth.

## Project identity

Apex Engine Next is the user's serious Android-first F1-style racing game project.
- Custom C++ engine path; do not switch to Unity or Godot unless the user explicitly changes direction.
- Android is the target platform; APK packaging is secondary to building the actual game.
- The quality bar is a serious racing game, not a toy/prototype.
- Visual/gameplay decisions should be research-driven and checked against original F1/racing gameplay references when appropriate.
- Track work is a major priority: real-world-inspired circuit layout, environment, elevation, curvature, turns, kerbs/barriers and overall visual character.
- Use free/open assets where licensing permits and record provenance/licensing before admitting assets.
- Never claim a track is exact or 99% accurate without evidence and visual/geometry validation.



## Beyond-expectations north star

Apex Engine Next is not being optimized merely to resemble existing mobile racing games. The target is an original racing experience whose combination of driving feel, circuit fidelity, presentation, simulation depth and mobile performance feels unusually ambitious for its platform.

The project should continuously pursue:
- believable vehicle dynamics that communicate grip, load transfer, tyre state and aero rather than arcade steering;
- circuits that feel geographically and visually specific instead of interchangeable procedural tracks;
- a living race presentation: meaningful camera language, timing/position information, start/restart/race-finish drama, and future broadcast-grade presentation;
- environmental depth that changes the driver's perception of speed and place without wasting mobile GPU budget;
- scalable systems so future weather, time-of-day, damage, strategy, telemetry and richer AI can be added without replacing the engine foundation;
- original design ideas that go beyond copying the feature checklist of another F1 game.

Every ambitious feature still needs the same rule: implement it, measure it, test it, and keep only what improves the actual game.

## Latest verified work

Commit 726fadab49327c4ce34fe0d4bc0f71600f7e214d:
- changed circuit kerbs from an unconditional full-lap stripe to a geometry-derived corner-only feature;
- added circuit-aware atmospheric clear palettes for distinct venue identity;
- preserved the existing rendering path and avoided extra geometry/draw-call overhead for the atmosphere change.

Verification status for this work:
- GitHub commit succeeded.
- Repository file update succeeded.
- Full Android runtime/build verification has NOT yet been performed in this session.
- Therefore this change must not be described as runtime-validated until a real build/test/device check succeeds.

## Verified repository state at checkpoint

The GitHub repository was inspected through Composio on 2026-10-04.

Current branch landscape:
- main exists and is unprotected.
- No other branch was returned by the branch listing.

Recent verified commits:
- 313a8d8 — Sync AIDE MainActivity exactly with Apex Engine Next source
- b41ff16 — Complete Apex Engine Next source transfer and correct AIDE entry point
- Earlier history exists; inspect it before reverting or deleting anything.

CI status:
- GitHub Actions workflow listing currently returned zero workflows/runs.
- A .gitlab-ci.yml file is present. Do not assume GitHub Actions is configured merely because the repository contains Android build files.

Important verified source files:
- app/src/main/cpp/main.cpp (~34 KB)
- app/src/main/cpp/physics.cpp (~4.3 KB)
- app/src/main/cpp/race.cpp (~5.6 KB)
- app/src/main/cpp/real_tracks.h (~60.9 KB)
- app/src/main/cpp/track_profiles.h
- app/src/main/cpp/track_asset_catalog.cpp/.h
- app/src/main/cpp/asset_catalog.cpp/.h
- app/src/main/cpp/asset_manifest.cpp/.h
- app/src/main/cpp/glb_loader.cpp/.h
- app/src/main/cpp/glb_mesh.cpp/.h
- app/src/main/cpp/CMakeLists.txt
- app/src/main/java/com/apexenginenext/MainActivity.java
- aide/src/com/apexenginenext/MainActivity.java
- app/build.gradle, root build.gradle, settings.gradle, gradle.properties
- tests/ contains asset, GLB, physics and track-related C++ tests.
- tools/ contains colab_build.sh, fetch_cc0_assets.sh, and generate_glb_assets.py.
- docs/ contains asset/reference planning and track visual audit documents.

Important project documents already present:
- docs/TRACK_VISUAL_REFERENCE_AUDIT.md
- docs/ASSET_INTEGRATION_PLAN.md
- docs/ASSET_IMPORT_SOURCES.md
- docs/GAMEPLAY_AND_FREE_ASSET_REFERENCE.md

## Current technical direction

The repository contains a native Android/C++ rendering and racing foundation with OpenGL ES/JNI-era Android structure, native physics/race code, GLB asset loading, track data and tests.

The presence of these files is verified. Their actual runtime quality must still be validated before declaring the game functional or visually accurate.

The current codebase also contains both app/ and aide/ Android entry-point structures. Treat this as an integration point to audit rather than assuming they are identical or both production paths.

## Highest-priority next work

1. Inspect the current HEAD and checkpoint before modifying anything.
2. Establish a reproducible build/test path from the repository itself.
3. Audit the native renderer/game loop and Android lifecycle for real runtime correctness.
4. Audit physics and race logic for regressions, especially steering direction, start-engine behavior, track boundaries, lap/checkpoint progression, AI positions and race completion.
5. Validate current track geometry and track selection logic against the actual data in real_tracks.h and track_profiles.h.
6. Continue serious track production: geometry first, then environment/assets, then visual validation. Fix layout bugs rather than merely adjusting code cosmetically.
7. Integrate free 3D assets only after license/provenance and runtime compatibility are checked.
8. Add or strengthen automated regression tests for every corrected subsystem.
9. Use GitHub commits as durable checkpoints after meaningful verified milestones.

## Research and quality rules

- Prefer verified repository contents over memory or assumptions.
- For external references, use current public sources and distinguish reference material from implemented features.
- Search GitHub/public sources for useful open-source techniques and free assets when they materially improve the game.
- Use available plugins/tools when they materially help; Composio is an approved route for GitHub and multi-service workflows.
- Do not repeatedly stop after trivial code corrections. Make substantive progress, then verify.
- Do not claim complete, fixed, tested, or working without evidence from the relevant build/test/runtime check.
- Preserve working systems unless a change is justified by evidence.
- Avoid destructive cleanup of old project files until their role is verified from the current tree/history.

## User's current strategic priority

Build the actual Apex Engine Next game first. APK packaging/testing can follow after the game systems and visuals are materially developed.

Track quality and believable F1-style racing presentation are major priorities. The goal is a polished, serious mobile racing experience, not a small demo.

## Checkpoint update protocol

After a meaningful work session:
- record the new HEAD commit;
- list what was actually changed;
- list tests/builds actually run and their results;
- list known failures/open issues;
- list the exact next highest-priority task;
- update this checkpoint file in the same repository.

Never overwrite a checkpoint with unsupported claims.