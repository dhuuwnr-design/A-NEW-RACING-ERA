#pragma once
#include <android/asset_manager.h>
#include <vector>
namespace apex { struct GlbMesh { std::vector<float> triangles; }; bool loadGlbMesh(AAssetManager*,const char*,GlbMesh&); }
