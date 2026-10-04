#include "glb_mesh.h"
#include "glb_loader.h"
#include <android/asset_manager.h>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
namespace apex {
static bool I(const std::string&s,const char*k,size_t f,int&o){auto p=s.find(k,f);if(p==std::string::npos)return false;p=s.find(':',p);if(p==std::string::npos)return false;char*e=nullptr;long v=strtol(s.c_str()+p+1,&e,10);if(e==s.c_str()+p+1)return false;o=(int)v;return true;}
static bool A(const std::string&j,int n,int&v,int&c,int&ct){auto p=j.find("\"accessors\":[");if(p==std::string::npos)return false;for(int i=0;i<=n;i++){p=j.find('{',p);auto e=j.find('}',p);if(p==std::string::npos||e==std::string::npos)return false;if(i==n){auto o=j.substr(p,e-p);return I(o,"\"bufferView\"",0,v)&&I(o,"\"componentType\"",0,c)&&I(o,"\"count\"",0,ct);}p=e+1;}return false;}
static bool V(const std::string&j,int n,int&o){auto p=j.find("\"bufferViews\":[");if(p==std::string::npos)return false;for(int i=0;i<=n;i++){p=j.find('{',p);auto e=j.find('}',p);if(p==std::string::npos||e==std::string::npos)return false;if(i==n){auto q=j.substr(p,e-p);o=0;I(q,"\"byteOffset\"",0,o);return true;}p=e+1;}return false;}
bool loadGlbMesh(AAssetManager*m,const char*path,GlbMesh&o){o={};if(!m)return false;AAsset*a=AAssetManager_open(m,path,AASSET_MODE_BUFFER);if(!a)return false;auto n=(size_t)AAsset_getLength(a);auto*b=(const unsigned char*)AAsset_getBuffer(a);GlbDocument d;if(!b||!parseGlb(b,n,d)||!d.jsonLength||!d.binLength){AAsset_close(a);return false;}std::string j((const char*)b+d.jsonOffset,d.jsonLength);int pa,ia,pv,pc,pn,iv,ic,in,po,io;if(!I(j,"\"POSITION\"",0,pa)||!I(j,"\"indices\"",0,ia)||!A(j,pa,pv,pc,pn)||!A(j,ia,iv,ic,in)||pc!=5126||ic!=5123||!V(j,pv,po)||!V(j,iv,io)){AAsset_close(a);return false;}if((size_t)po+(size_t)pn*12>d.binLength||(size_t)io+(size_t)in*2>d.binLength){AAsset_close(a);return false;}auto*ps=b+d.binOffset+po;auto*is=b+d.binOffset+io;std::vector<float>v((size_t)pn*3);memcpy(v.data(),ps,v.size()*4);o.triangles.reserve((size_t)in*3);for(int k=0;k<in;k++){uint16_t q=uint16_t(is[k*2])|(uint16_t(is[k*2+1])<<8);if(q>=pn){o={};AAsset_close(a);return false;}o.triangles.insert(o.triangles.end(),{v[q*3],v[q*3+1],v[q*3+2]});}AAsset_close(a);return !o.triangles.empty();}
}
