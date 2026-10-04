#pragma once
#include <array>
namespace apex {
enum class AssetKind { Vehicle, Trackside, Environment };
struct AssetManifestEntry {
    const char* id;
    const char* relativePath;
    AssetKind kind;
    int maxTriangles;
    int maxTextureBytes;
    bool optional;
};
constexpr std::array<AssetManifestEntry,7> kAssetManifest{{
    {"player_openwheel","assets/cars/player.glb",AssetKind::Vehicle,12000,4*1024*1024,false},
    {"ai_openwheel","assets/cars/ai.glb",AssetKind::Vehicle,12000,3*1024*1024,true},
    {"pit_grandstand","assets/environment/pit_grandstand.glb",AssetKind::Environment,12000,2*1024*1024,true},
    {"start_finish_gantry","assets/environment/start_finish_gantry.glb",AssetKind::Trackside,1500,512*1024,true},
    {"trackside_barrier","assets/environment/barrier.glb",AssetKind::Trackside,4000,512*1024,true},
    {"trackside_signs","assets/environment/signs.glb",AssetKind::Trackside,2500,512*1024,true},
    {"vegetation_cluster","assets/environment/vegetation.glb",AssetKind::Environment,6000,1024*1024,true}
}};
}
