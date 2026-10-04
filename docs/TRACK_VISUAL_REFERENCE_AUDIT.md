# Apex Engine Next — Circuit Visual Reference Audit

This document records the external visual references used while building the seven current circuits.

## Reference principles

- The centerline/width geometry is sourced from the open TUMFTM racetrack database already imported into `real_tracks.h`.
- Gameplay references are used for *visual and spatial cues* (corner sequence, kerbs, runoff, barriers, landmark placement), not copied game textures or proprietary models.
- The renderer uses circuit-specific procedural materials and terrain profiles so the seven venues do not share one generic environment.
- Published circuit lengths are used as the final geometric calibration target.

## Circuit references

### Spa-Francorchamps
- Formula 1 onboard: https://www.formula1.com/en/video/track-view-onboard-with-oscar-piastri-around-spa-francorchamps.1870865393591520415
- F1 24 recreation comparison: https://www.overtake.gg/news/f1-24-gameplay-showcases-new-spa-and-silverstone-recreations.2062/
- Visual cues: Ardennes forest, large elevation variation, red/white kerbs, broad modern runoff, Raidillon/Eau Rouge climb.

### Silverstone
- F1 25 gameplay/race reference: https://www.youtube.com/watch?v=yJzRfaDDuTY
- F1 24 hotlap reference: https://www.youtube.com/watch?v=clPjF4RwOJk
- Visual cues: open British parkland, very fast flowing corners, green surroundings, modern kerbs/runoff and dense spectator infrastructure.

### Monza
- F1 onboard: https://www.formula1.com/en/video/onboard-a-lap-of-monza.6060946204001
- F1 25 track guide: https://www.youtube.com/watch?v=QiNm5RIEQhk
- Visual cues: long straights, heavy braking chicanes, red/white kerbs, park/woodland setting and gravel traps.

### Suzuka
- F1 circuit guide: https://www.formula1.com/en/latest/article/circuit-guide-everything-you-need-to-know-about-the-suzuka-circuit.2BbgsRdkeux78UBGbmYiZV
- F1 23 gameplay reference: https://www.chaptercheats.com/cheat/pc/601174/f1-23/video-walkthrough/386689
- Visual cues: dense Japanese woodland, flowing S Curves, Degner, Spoon, 130R, and the figure-eight flyover.
- The current engine explicitly models the flyover as a 3-D crossing rather than a flat 2-D intersection.

### Bahrain / Sakhir
- Mercedes hot-lap guide: https://www.mercedesamgf1.com/news/watch-your-guide-to-a-hot-lap-of-the-bahrain-f1-track
- Visual cues: desert surroundings, wide paved runoff, floodlight/night-race character and sparse vegetation.

### Interlagos / Sao Paulo
- F1technical onboard guide: https://www.f1technical.net/news/10906
- F1 25 track guide: https://www.youtube.com/watch?v=3YPlkrEm6-8
- Visual cues: dramatic elevation, compressed technical middle sector, Senna S, kerbs and dense green hillside surroundings.

### Circuit of the Americas / Austin
- EA SPORTS F1 24 COTA guide: https://www.youtube.com/watch?v=MZHLXp5CF6o
- EA SPORTS F1 22 COTA guide: https://www.youtube.com/watch?v=tGe5eB88YwI
- Visual cues: large Turn 1 climb/blind apex, fast opening sector, long back straight, tower/fan-zone landmarks and prairie-like surroundings.

## Current implementation status

Implemented in the engine:
1. 320 real centerline samples per circuit.
2. Measured source widths retained per sample.
3. Closed-polyline length recalculation before official-length normalization.
4. Circuit-specific terrain/elevation envelopes.
5. Circuit-specific grass, road, runoff and gravel materials.
6. Circuit-specific tree density/environment profiles.
7. Suzuka overpass elevation separation and support structures.
8. Circuit-specific procedural landmark/scenery placement pass for all seven venues.
8. Automated seven-circuit geometry validation.

Still being developed:
- Exact telemetry-derived altitude profiles.
- Higher-fidelity GLB grandstands, pit buildings, fencing and marshal posts.
- Final authored landmark meshes and texture/material variants.
- Higher fidelity kerb segmentation and runoff paint patterns.
- Additional public/CC0 3-D assets.
