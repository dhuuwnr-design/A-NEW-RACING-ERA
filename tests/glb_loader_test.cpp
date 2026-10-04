#include "../app/src/main/cpp/glb_loader.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static void put32(std::vector<unsigned char>& b, std::uint32_t v) {
    b.push_back((unsigned char)(v & 255u));
    b.push_back((unsigned char)((v >> 8) & 255u));
    b.push_back((unsigned char)((v >> 16) & 255u));
    b.push_back((unsigned char)((v >> 24) & 255u));
}

static std::vector<unsigned char> makeGlb() {
    const std::string json = "{\"asset\":{\"version\":\"2.0\"}}";
    const std::size_t paddedJson = (json.size() + 3u) & ~std::size_t(3u);
    const std::size_t binSize = 4;
    const std::size_t total = 12 + 8 + paddedJson + 8 + binSize;
    std::vector<unsigned char> b;
    b.reserve(total);
    put32(b, 0x46546C67u);
    put32(b, 2);
    put32(b, (std::uint32_t)total);
    put32(b, (std::uint32_t)paddedJson);
    put32(b, 0x4E4F534Au);
    b.insert(b.end(), json.begin(), json.end());
    while (b.size() < 12 + 8 + paddedJson) b.push_back(' ');
    put32(b, (std::uint32_t)binSize);
    put32(b, 0x004E4942u);
    b.insert(b.end(), {0,1,2,3});
    return b;
}

int main() {
    const auto bytes = makeGlb();
    apex::GlbDocument doc;
    assert(apex::parseGlb(bytes.data(), bytes.size(), doc));
    assert(doc.version == 2);
    assert(doc.totalLength == bytes.size());
    assert(apex::hasJsonChunk(doc));
    assert(apex::hasBinChunk(doc));
    assert(doc.binLength == 4);

    auto bad = bytes;
    bad[4] = 1;
    assert(!apex::parseGlb(bad.data(), bad.size(), doc));

    bad = bytes;
    bad[8] = 0;
    assert(!apex::parseGlb(bad.data(), bad.size(), doc));

    std::printf("APEX_NEXT_GLB_LOADER_TEST PASS\n");
}
