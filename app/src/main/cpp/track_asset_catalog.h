#pragma once
#include <array>
#include <string_view>
namespace apex {
struct TrackAsset {
 const char* id;
 const char* localPath;
 const char* sourceUrl;
 int triangles;
 float width;
 float length;
};
const TrackAsset* findTrackAsset(std::string_view id);
struct TrackAssetPlacement {
 const char* assetId;
 float distance;
 float lateral;
 float yawOffset;
 float scale;
};
constexpr std::array<TrackAssetPlacement,6> kTrackAssetPlacements{{
 {"start_finish_straight",0.0f,0.0f,0.0f,1.0f},
 {"banked_corner",120.0f,0.0f,0.0f,1.0f},
 {"banked_corner",155.0f,0.0f,1.5707963f,1.0f},
 {"start_finish_straight",190.0f,0.0f,0.0f,1.0f},
 {"banked_corner",300.0f,0.0f,3.1415926f,1.0f},
 {"banked_corner",335.0f,0.0f,4.7123890f,1.0f}
}};
}
