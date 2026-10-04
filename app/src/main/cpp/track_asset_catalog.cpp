#include "track_asset_catalog.h"
namespace apex {
static constexpr TrackAsset kAssets[]={
 {"start_finish_straight","assets/tracks/start_finish_straight.glb","https://cdn.3dassets.dev/assets/15190/v1/model.glb",652,12.0f,20.1f},
 {"banked_corner","assets/tracks/banked_corner.glb","https://cdn.3dassets.dev/assets/15189/v1/model.glb",1160,12.0f,33.4f}
};
const TrackAsset* findTrackAsset(std::string_view id){for(const auto&a:kAssets)if(id==a.id)return &a;return nullptr;}
}
