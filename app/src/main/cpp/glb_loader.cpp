#include "glb_loader.h"
#include <cstring>
namespace apex {
namespace {
constexpr std::uint32_t kMagic = 0x46546C67u;
constexpr std::uint32_t kJson  = 0x4E4F534Au;
constexpr std::uint32_t kBin   = 0x004E4942u;
std::uint32_t u32(const unsigned char* p){return std::uint32_t(p[0])|(std::uint32_t(p[1])<<8)|(std::uint32_t(p[2])<<16)|(std::uint32_t(p[3])<<24);}
bool rangeOk(std::size_t o,std::size_t l,std::size_t t){return o<=t&&l<=t-o;}
}
bool parseGlb(const unsigned char* bytes,std::size_t size,GlbDocument& out){
 out={}; if(!bytes||size<12)return false; if(u32(bytes)!=kMagic)return false;
 const std::uint32_t version=u32(bytes+4),declaredLength=u32(bytes+8);
 if(version!=2||declaredLength<12||declaredLength>size)return false;
 out.version=version;out.totalLength=declaredLength;std::size_t cursor=12;bool jsonSeen=false;
 while(cursor<declaredLength){if(declaredLength-cursor<8)return false;const std::uint32_t chunkLength=u32(bytes+cursor),chunkType=u32(bytes+cursor+4);const std::size_t payload=cursor+8;if(!rangeOk(payload,chunkLength,declaredLength))return false;
  if(chunkType==kJson&&!jsonSeen){out.jsonOffset=payload;out.jsonLength=chunkLength;jsonSeen=true;}
  else if(chunkType==kBin&&out.binLength==0){out.binOffset=payload;out.binLength=chunkLength;}
  cursor=payload+chunkLength;}
 return cursor==declaredLength&&jsonSeen;
}
bool hasJsonChunk(const GlbDocument& doc){return doc.jsonLength!=0;}
bool hasBinChunk(const GlbDocument& doc){return doc.binLength!=0;}
}
