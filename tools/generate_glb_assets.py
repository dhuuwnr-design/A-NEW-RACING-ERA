#!/usr/bin/env python3
import json,math,os,struct
R=os.path.join(os.path.dirname(__file__),"..","app","src","main","assets","glb");os.makedirs(R,exist_ok=True)
def G(V,I):
 p=b''.join(struct.pack("<3f",*v) for v in V);o=(len(p)+3)//4*4;b=p+b'\0'*(o-len(p))+b''.join(struct.pack("<H",i) for i in I);j={"asset":{"version":"2.0","generator":"Apex Engine Next"},"scene":0,"scenes":[{"nodes":[{"mesh":0}]}],"nodes":[{"mesh":0}],"meshes":[{"primitives":[{"attributes":{"POSITION":0},"indices":1,"mode":4}]}],"buffers":[{"byteLength":len(b)}],"bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":len(p)},{"buffer":0,"byteOffset":o,"byteLength":len(I)*2}],"accessors":[{"bufferView":0,"componentType":5126,"count":len(V),"type":"VEC3"},{"bufferView":1,"componentType":5123,"count":len(I),"type":"SCALAR"}]};q=json.dumps(j,separators=(",",":")).encode();q+=b' '*((-len(q))%4);return struct.pack("<4sII",b"glTF",2,12+8+len(q)+8+len(b))+struct.pack("<II",len(q),0x4E4F534A)+q+struct.pack("<II",len(b),0x4E4942)+b
def B(V,I,c,s):
 x,y,z=c;a,b,d=s;n=len(V)
 for X,Y,Z in [(-1,-1,-1),(1,-1,-1),(1,-1,1),(-1,-1,1),(-1,1,-1),(1,1,-1),(1,1,1),(-1,1,1)]:V.append((x+X*a,y+Y*b,z+Z*d))
 for f in [(0,1,2,0,2,3),(4,7,6,4,6,5),(0,4,5,0,5,1),(3,2,6,3,6,7),(1,5,6,1,6,2),(0,3,7,0,7,4)]:I.extend(n+k for k in f)
def C(V,I,c,r,h):
 x,y,z=c;n=len(V);N=10
 for k in range(N):
  a=2*math.pi*k/N;V.append((x+r*math.cos(a),y-h,z+r*math.sin(a)));V.append((x+r*math.cos(a),y+h,z+r*math.sin(a)))
 cb=len(V);V.append((x,y-h,z));ct=len(V);V.append((x,y+h,z))
 for k in range(N):
  q=(k+1)%N;I.extend([n+2*k,n+2*q,n+2*N if False else n+2*q+1,n+2*k,n+2*q+1,n+2*k+1,cb,n+2*q,n+2*k,ct,n+2*k+1,n+2*q+1])
def W(n,V,I):open(os.path.join(R,n),"wb").write(G(V,I))
V=[];I=[];B(V,I,(0,.28,0),(1,.22,1.7));B(V,I,(0,.48,-.15),(.38,.18,.65));B(V,I,(0,.34,.8),(.55,.06,.18));B(V,I,(0,.45,-1),(1,.06,.18));B(V,I,(-.62,.36,-.05),(.35,.16,.65));B(V,I,(.62,.36,-.05),(.35,.16,.65));B(V,I,(0,.18,1.18),(1.15,.05,.18));B(V,I,(0,.45,-1.18),(1.1,.05,.16));B(V,I,(0,.78,-.1),(.04,.04,.58));B(V,I,(-.35,.76,-.05),(.04,.04,.5));B(V,I,(.35,.76,-.05),(.04,.04,.5))
for x in (-.9,.9):
 for z in (-.62,.62):C(V,I,(x,.22,z),.28,.12)
W("player_openwheel.glb",V,I)
for n,fn in [("grandstand.glb",lambda V,I:[B(V,I,(i*2.2-4.4,1,0),(1,.9,.7)) for i in range(5)]),("pit_building.glb",lambda V,I:B(V,I,(0,1.5,0),(4,.1,2))),("barrier.glb",lambda V,I:[B(V,I,(i*1.5-2.25,.45,0),(.7,.45,.08)) for i in range(4)]),("signage.glb",lambda V,I:(B(V,I,(0,1.8,0),(1.2,.45,.06)),B(V,I,(0,.9,0),(.06,.9,.06)))),("vegetation.glb",lambda V,I:(C(V,I,(0,.8,0),.12,.8),C(V,I,(0,1.9,0),.7,.75))),("landmark.glb",lambda V,I:(B(V,I,(0,1.2,0),(2.5,1.2,1)),B(V,I,(0,2.8,0),(1,.4,.5))))]:
 V=[];I=[];fn(V,I);W(n,V,I)
