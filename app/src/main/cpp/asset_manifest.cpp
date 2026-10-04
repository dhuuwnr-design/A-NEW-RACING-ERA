#include "asset_manifest.h"
namespace apex {
static_assert(kAssetManifest[0].maxTriangles <= 45000, "player asset budget exceeded");
static_assert(kAssetManifest[0].maxTextureBytes <= 4*1024*1024, "player texture budget exceeded");
}
