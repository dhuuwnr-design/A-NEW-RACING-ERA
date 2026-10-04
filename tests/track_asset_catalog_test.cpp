#include "../app/src/main/cpp/track_asset_catalog.h"
#include <cassert>
#include <cstdio>
int main(){
 auto* s=apex::findTrackAsset("start_finish_straight");
 auto* c=apex::findTrackAsset("banked_corner");
 assert(s&&c&&s->triangles<1000&&c->triangles<2000);
 assert(apex::kTrackAssetPlacements.size()==6);
 std::printf("APEX_NEXT_TRACK_ASSET_CATALOG_TEST PASS\n");
}