#pragma once
#include <cmath>

namespace apex {

struct TrackVisualProfile {
    const char* environment;
    float grass[3];
    float road[3];
    float runoff[3];
    float gravel[3];
    float kerbA[3];
    float kerbB[3];
    float runoffOuter;
    float gravelOuter;
    float treeSpacing;
    float elevationRange;
    bool bridge;
};

static constexpr TrackVisualProfile TRACK_PROFILES[7] = {
    {"Ardennes forest", {0.035f,0.16f,0.055f}, {0.045f,0.050f,0.055f}, {0.12f,0.13f,0.15f}, {0.34f,0.29f,0.22f}, {0.92f,0.92f,0.92f}, {0.78f,0.05f,0.05f}, 7.0f, 12.0f, 15.0f, 102.2f, false},
    {"British parkland", {0.045f,0.19f,0.065f}, {0.045f,0.050f,0.055f}, {0.16f,0.18f,0.20f}, {0.30f,0.26f,0.20f}, {0.92f,0.92f,0.92f}, {0.78f,0.05f,0.05f}, 7.0f, 12.0f, 17.0f, 11.3f, false},
    {"Monza park", {0.035f,0.15f,0.045f}, {0.050f,0.052f,0.055f}, {0.14f,0.16f,0.18f}, {0.34f,0.29f,0.20f}, {0.92f,0.92f,0.92f}, {0.78f,0.05f,0.05f}, 7.0f, 12.0f, 14.0f, 12.8f, false},
    {"Suzuka woodland", {0.035f,0.17f,0.055f}, {0.045f,0.048f,0.052f}, {0.13f,0.15f,0.17f}, {0.36f,0.30f,0.22f}, {0.92f,0.92f,0.92f}, {0.78f,0.05f,0.05f}, 6.5f, 11.5f, 14.0f, 40.4f, true},
    {"Bahrain desert", {0.23f,0.18f,0.10f}, {0.050f,0.052f,0.055f}, {0.12f,0.15f,0.19f}, {0.55f,0.42f,0.24f}, {0.92f,0.92f,0.92f}, {0.78f,0.05f,0.05f}, 7.5f, 12.5f, 32.0f, 16.9f, false},
    {"Interlagos hills", {0.025f,0.14f,0.045f}, {0.045f,0.050f,0.055f}, {0.13f,0.15f,0.17f}, {0.34f,0.27f,0.19f}, {0.92f,0.92f,0.92f}, {0.78f,0.05f,0.05f}, 6.5f, 11.5f, 16.0f, 43.0f, false},
    {"Austin prairie", {0.055f,0.16f,0.060f}, {0.045f,0.050f,0.055f}, {0.13f,0.15f,0.17f}, {0.38f,0.31f,0.22f}, {0.92f,0.92f,0.92f}, {0.78f,0.05f,0.05f}, 7.0f, 12.0f, 20.0f, 30.0f, false}
};

inline float gauss(float p, float center, float width) {
    float d=p-center;
    return std::exp(-(d*d)/(2.0f*width*width));
}

inline float trackElevation(int trackIndex, float p) {
    p -= std::floor(p);
    switch(trackIndex % 7) {
        case 0: // Spa: major climb through Eau Rouge/Raidillon, then long descent.
            return 51.1f*gauss(p,.075f,.045f) - 28.0f*gauss(p,.40f,.11f) + 16.0f*gauss(p,.62f,.12f);
        case 1: // Silverstone: shallow, rolling airfield terrain.
            return 5.65f*std::sin(6.2831853f*(p*.92f+.08f)) + 1.4f*gauss(p,.72f,.08f);
        case 2: // Monza: mostly flat with gentle parkland undulation.
            return 6.4f*std::sin(6.2831853f*(p*1.08f+.15f)) + 1.0f*gauss(p,.55f,.10f);
        case 3: { // Suzuka: rolling circuit plus the real figure-eight flyover.
            float h=20.2f*std::sin(6.2831853f*(p*.92f+.10f)) + 4.0f*gauss(p,.31f,.07f);
            // The return/back straight passes over Degner 2/130R. Raise only the
            // upper branch so the 2-D crossing becomes a real 3-D overpass.
            h += 18.0f*gauss(p,.847f,.025f);
            return h;
        }
        case 4: // Bahrain: low-relief desert terrain.
            return 8.45f*std::sin(6.2831853f*(p*.72f+.17f)) + 1.2f*gauss(p,.18f,.08f);
        case 5: // Interlagos: pronounced rolling elevation.
            return 21.5f*std::sin(6.2831853f*(p*.82f+.05f)) + 5.0f*gauss(p,.12f,.055f) - 3.0f*gauss(p,.66f,.08f);
        default: // COTA: strong Turn-1 rise followed by rolling terrain.
            return 16.0f*gauss(p,.025f,.035f) - 7.0f*gauss(p,.18f,.09f) + 9.0f*std::sin(6.2831853f*(p*.78f+.25f));
    }
}

}
