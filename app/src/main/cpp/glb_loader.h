#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace apex {
struct GlbChunk { std::uint32_t type=0; std::size_t offset=0; std::size_t length=0; };
struct GlbDocument { std::uint32_t version=0; std::size_t totalLength=0; std::size_t jsonOffset=0; std::size_t jsonLength=0; std::size_t binOffset=0; std::size_t binLength=0; };
bool parseGlb(const unsigned char* bytes,std::size_t size,GlbDocument& out);
bool hasJsonChunk(const GlbDocument& doc);
bool hasBinChunk(const GlbDocument& doc);
}
