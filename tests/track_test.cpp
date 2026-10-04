#include "../app/src/main/cpp/race.h"
#include "../app/src/main/cpp/real_tracks.h"
#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
    apex::Race race;
    assert(race.points.size() == 320);
    assert(race.ai.size() == 7);

    const float official[apex::REAL_TRACK_COUNT] = {
        7004.0f,5891.0f,5793.0f,5807.0f,5412.0f,4309.0f,5513.0f
    };

    apex::Car car{};
    for (int track=0; track<apex::REAL_TRACK_COUNT; ++track) {
        if (track > 0) race.nextTrack(car);
        assert(race.trackIndex == track);
        assert(std::fabs(race.trackLength-official[track]) < 1.5f);

        float minY=1e9f,maxY=-1e9f;
        for (const auto& p : race.points) {
            assert(std::isfinite(p.x) && std::isfinite(p.z));
            assert(std::isfinite(p.tx) && std::isfinite(p.tz));
            assert(std::isfinite(p.progress) && p.progress >= 0.0f && p.progress < 1.0f);
            assert(p.width > 3.0f && p.width < 15.0f);
            float tangentLen=std::sqrt(p.tx*p.tx+p.tz*p.tz);
            assert(tangentLen > 0.8f && tangentLen < 1.2f);
            minY=std::min(minY,p.elevation);
            maxY=std::max(maxY,p.elevation);
        }
        assert(maxY-minY > 2.0f);
        assert(std::fabs(race.points.front().progress) < 0.001f);
        assert(race.points.back().progress < 1.0f);
    }

    // Suzuka must retain its distinctive 3-D crossover: the return branch is elevated.
    apex::Race suzuka;
    apex::Car suzukaCar{};
    suzuka.nextTrack(suzukaCar);
    suzuka.nextTrack(suzukaCar);
    suzuka.nextTrack(suzukaCar);
    float lower=1e9f, upper=-1e9f;
    for (const auto& p : suzuka.points) {
        if (p.progress > .42f && p.progress < .46f) lower=std::min(lower,p.elevation);
        if (p.progress > .83f && p.progress < .87f) upper=std::max(upper,p.elevation);
    }
    assert(upper-lower > 4.0f);

    std::printf("APEX_NEXT_TRACK_TEST PASS: seven calibrated circuits, 320 samples each, Suzuka crossover elevation validated\n");
    return 0;
}
