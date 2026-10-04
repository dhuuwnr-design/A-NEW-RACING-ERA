#pragma once
#include <vector>
#include <cmath>
#include "physics.h"

namespace apex {

struct TrackPoint {
    float x,z;
    float tx,tz;
    float progress;
    float elevation=0;
    float width=5.2f;
};

struct AI {
    float progress=0,laps=0,speed=0,x=0,z=0,yaw=0,elevation=0,lineOffset=0;
    float previousProgress=0;
    float lapTime=0,lastLapTime=0,bestLapTime=-1;
    bool finished=false;
};

struct Race {
    int trackIndex=0;
    std::vector<TrackPoint> points;
    std::vector<AI> ai;
    float trackLength=1,playerProgress=0,lastProgress=0,playerElevation=0;
    float raceTime=0,finishTime=-1,startTimer=3.0f,goTimer=0,offTrackDistance=0;
    float gapToLeader=0,intervalToAhead=0;
    int completedLaps=0;
    int lap=1,totalLaps=5,position=1,previousPosition=1,positionDelta=0;
    int playerPoint=0;
    bool started=false,finished=false;
    Race();
    void rebuildTrackMetrics();
    void reset(Car& car);
    void update(Car& car,float dt);
    void nextTrack(Car& car);
    const char* trackName() const;
};

}
