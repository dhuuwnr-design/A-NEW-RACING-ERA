#include "asset_catalog.h"
#include <cstdint>
#include <cstring>
namespace apex {
static constexpr AssetSource kSources[] = {
 {"player_openwheel","assets/cars/player.glb","https://cdn.3dassets.dev/assets/18685/v1/model.glb","CC0-1.0",4u*1024u*1024u},
 {"ai_openwheel","assets/cars/ai.glb","https://cdn.3dassets.dev/assets/18685/v1/model.glb","CC0-1.0",3u*1024u*1024u},
 {"pit_grandstand","assets/environment/pit_grandstand.glb","https://3dassets.dev/packs/go-kart-track-and-pit-lane","CC0-1.0",2u*1024u*1024u},
 {"start_finish_gantry","assets/environment/start_finish_gantry.glb","https://3dassets.dev/","CC0-1.0",512u*1024u}
};
const AssetSource* findAssetSource(std::string_view id){for(const auto& s:kSources)if(id==s.id)return &s;return nullptr;}
bool isSupportedGlbHeader(const unsigned char* b,std::size_t n){if(!b||n<12||std::memcmp(b,"glTF",4)!=0)return false;std::uint32_t v=std::uint32_t(b[4])|(std::uint32_t(b[5])<<8)|(std::uint32_t(b[6])<<16)|(std::uint32_t(b[7])<<24);return v==2;}
}
