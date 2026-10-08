# Apex Engine Next — Release Gates

This file is the release contract for Apex Engine Next.

## Current product gate: PLAYABLE VERTICAL SLICE

The first public APK is **blocked** until all of these are green:
- Android orientation is correct and remains landscape.
- Loading screen completes without a stall.
- Main menu has no dead/false actions.
- Only implemented features are actionable; unfinished features remain visibly LOCKED.
- Quick Race launches reliably.
- Conventional touch controls work simultaneously: left/right + throttle/brake.
- Camera toggle works and never flips the world/upside-down.
- Car can accelerate, steer, brake, and remain controllable after repeated input.
- HUD values remain sane during a race.
- Race completion works.
- Host physics/track/asset tests pass.
- Android debug build succeeds from a clean checkout.
- No known P0/P1 gameplay blocker remains.

## Feature unlock policy

Features are intentionally locked until their underlying systems are actually implemented and tested.

| Feature | State | Unlock condition |
|---|---|---|
| Quick Race | OPEN | Core playable race passes |
| Settings | OPEN | Current basic settings screen works |
| Career | LOCKED | Progression, saves, race flow and rewards implemented |
| Multiplayer | LOCKED | Networking, synchronization, lobby/session flow and disconnect handling implemented |
| Garage | LOCKED | Vehicle setup/customization data and persistence implemented |
| Pit/strategy | LOCKED | Pit lane, stops, tyre/fuel/strategy systems implemented |
| Weather | LOCKED | Visual + grip/wet-line/spray simulation verified |
| Advanced AI | LOCKED | Overtaking, defending, traffic and race strategy verified |
| Replay/broadcast | LOCKED | Deterministic replay/state capture and camera system verified |
| Damage | LOCKED | Collision/damage model and safe recovery verified |

## Release rule

**Do not distribute an APK merely because CI can build it.**

An APK becomes a release candidate only after the current major milestone is complete, automated tests are green, and the Android acceptance checklist has been run from a clean install.

If a critical glitch is discovered, release status returns to **BLOCKED** until fixed and retested.

## Quality target

A non-F1 player should understand the menu and controls within seconds.
An F1 fan should recognize the racing presentation, information hierarchy and seriousness of the product.

No fake buttons, placeholder features presented as finished, or knowingly broken systems are allowed in a release build.
