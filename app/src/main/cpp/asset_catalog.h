#pragma once
#include <cstddef>
#include <string_view>
namespace apex {
struct AssetSource { const char* id; const char* localPath; const char* sourceUrl; const char* license; std::size_t expectedMaxBytes; };
const AssetSource* findAssetSource(std::string_view id);
bool isSupportedGlbHeader(const unsigned char* bytes, std::size_t size);
}
