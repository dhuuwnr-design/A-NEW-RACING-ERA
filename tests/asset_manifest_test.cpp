#include "../app/src/main/cpp/asset_manifest.h"
#include <cassert>
#include <cstdio>
int main() {
    assert(apex::kAssetManifest.size() == 7);
    for (const auto& a : apex::kAssetManifest) {
        assert(a.id && a.relativePath);
        assert(a.maxTriangles > 0 && a.maxTextureBytes > 0);
    }
    assert(!apex::kAssetManifest[0].optional);
    assert(apex::kAssetManifest[0].maxTriangles <= 12000);
    assert(apex::kAssetManifest[3].maxTriangles <= 1500);
    std::printf("APEX_NEXT_ASSET_MANIFEST_TEST PASS entries=%zu\\n",
                apex::kAssetManifest.size());
}
