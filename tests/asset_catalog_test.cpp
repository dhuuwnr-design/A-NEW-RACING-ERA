#include "../app/src/main/cpp/asset_catalog.h"
#include <cassert>
#include <cstdio>
#include <string_view>
int main(){
 const auto* player=apex::findAssetSource("player_openwheel");
 assert(player && std::string_view(player->license)=="CC0-1.0");
 assert(player->expectedMaxBytes<=4u*1024u*1024u);
 const unsigned char valid[]={'g','l','T','F',2,0,0,0,12,0,0,0};
 assert(apex::isSupportedGlbHeader(valid,sizeof(valid)));
 const unsigned char invalid[]={'g','l','T','F',1,0,0,0,0,0,0,0};
 assert(!apex::isSupportedGlbHeader(invalid,sizeof(invalid)));
 assert(apex::findAssetSource("does_not_exist")==nullptr);
 std::printf("APEX_NEXT_ASSET_CATALOG_TEST PASS\n");
}
