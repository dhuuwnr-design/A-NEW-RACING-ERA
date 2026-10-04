# Apex Engine Next — 2.2 free 3D asset integration

## Selected production vehicle
The first asset target is the CC0 open-wheel racer from 3DAssets.dev:
- 8,028 triangles
- 348.8 KB GLB
- wheel-roll animation
- CC0 1.0
- source: https://3dassets.dev/assets/transport-collection-hd-open-wheel-racer-aa0e5cef
- GLB: https://cdn.3dassets.dev/assets/18685/v1/model.glb

Kenney Racing Kit is the trackside source library: CC0, 110 3D assets.
https://kenney.nl/assets/racing-kit

## Implemented now
- Asset IDs map to local Android asset paths and public CC0 provenance URLs.
- Player vehicle stays inside the 12k-triangle / 4 MB mobile budget.
- A GLB v2 header gate prevents unsupported blobs reaching the renderer.
- CMake includes the asset catalog and native test.
- Procedural vehicle/track rendering remains the safe fallback.

## Important status
The actual GLB binary is NOT yet committed to GitLab. The available GitLab connector in this session does not provide a binary download/upload path from the CC0 CDN. The source URL is provenance metadata, not a claim that the model is already inside the APK.

## Next gate
1. Vendor a lightweight GLB/glTF decoder.
2. Import player.glb into Android assets.
3. Decode POSITION/NORMAL/UV and materials.
4. Drive wheel animation and steering from physics.
5. Add asset-backed car rendering with procedural fallback.
6. Instance CC0 gantry, barriers, signs and vegetation.


## 2.8 CC0 asset upgrade research

### Approved source candidates
- **Kenney Racing Kit** — CC0; 70+ optimized low-poly racing objects; includes glTF 2.0 plus roads, borders, tents, pitstop, flags, textured billboards, fences and bridges. Source: https://opengameart.org/content/racing-kit and official asset page https://kenney.nl/assets/racing-kit
- **Modular racing pack by Marina_sunny_girl** — CC0 1.0; 40+ models with GLB/Blend/OBJ/FBX/DAE; lightweight materials/textures. Source: https://marina-sunny-girl.itch.io/modular-racing-pack

### Integration rule
Do not replace the current engine's track geometry with generic CC0 track tiles: the seven circuit layouts remain Apex-specific. Import CC0 assets only as scene dressing/infrastructure where they improve visual fidelity. The generated placeholders remain fallback assets.

### Car rule
The CC0 Racing Kit is not treated as the final open-wheel/F1 car. The open-wheel car gets a separate higher-fidelity GLB source/model pipeline so environment upgrades do not lower vehicle fidelity.

### Mobile rule
Prefer GLB, low draw-call meshes, small textures, and reused materials. Assets must pass the existing GLB bounds/parser checks before runtime use.
