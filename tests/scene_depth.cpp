#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <vector>
static int remaining=-1;
static void *allocate(size_t n) {if(remaining==0)return nullptr;if(remaining>0)--remaining;return malloc(n);}
#define ENGINE_MEM_MALLOC allocate
#define ENGINE_MEM_FREE free
#include "../engine/scene_depth.hpp"
using T=MeshVisibility::Triangle;
static const int W=64,H=48;
int main() {
    for(int fail=0;fail<4;++fail) {remaining=fail;SceneDepth s;assert(!s.init(W,H));}
    remaining=-1;
    int order[4]={0,1,2,3};int checked=0;
    do {
        SceneDepth s;assert(s.init(W,H));s.clear();std::vector<uint16_t> screen(W*H,0);
        auto blit=[&](int x,int y,const uint16_t *p,int n) {assert(x>=0 && x+n<=W && y>=0 && y<H);std::copy(p,p+n,screen.begin()+y*W+x);};
        auto rect=[&](int l,int t,int r,int b,float z,uint16_t color,bool textured) {
            T a={{{float(l),float(t),1/z,0,0},{float(r),float(t),1/z,1,0},{float(r),float(b),1/z,1,1}},color,int16_t(t),int16_t(b-1)};
            T c={{{float(l),float(t),1/z,0,0},{float(r),float(b),1/z,1,1},{float(l),float(b),1/z,0,1}},color,int16_t(t),int16_t(b-1)};
            uint16_t white=0xffff;s.triangle(a,textured?&white:nullptr,1,1,blit);s.triangle(c,textured?&white:nullptr,1,1,blit);
        };
        const uint8_t pixels[4]={0x1c,0x1c,0x1c,0x1c},mask=0x90;
        for(int id:order) {
            if(id==0)rect(20,10,44,38,4,0x001f,false); // wall
            if(id==1)rect(16,20,48,30,6,0xf800,false); // partly hidden car
            if(id==2)rect(28,22,36,34,2,0xffff,true); // foreground textured mesh
            if(id==3)s.billboard(12,16,40,20,5,2,2,pixels,&mask,blit);
        }
        for(int y=0;y<H;++y)for(int x=0;x<W;++x) {
            uint16_t expected=0;
            if(x>=16 && x<48 && y>=20 && y<30)expected=0xf800;
            if(x>=12 && x<52 && y>=16 && y<36 && ((x<32)==(y<26)))expected=0x07e0;
            if(x>=20 && x<44 && y>=10 && y<38)expected=0x001f;
            if(x>=28 && x<36 && y>=22 && y<34)expected=0xffff;
            assert(screen[y*W+x]==expected);++checked;
        }
        s.clear();std::fill(screen.begin(),screen.end(),0);
        rect(20,10,44,38,8,0x1234,false);assert(screen[24*W+32]==0x1234);
    } while(std::next_permutation(order,order+4));
    // Two intersecting depth planes, with no pixel center on their intersection.
    // The expected winner comes from the planes, independently of depth encoding.
    for(int reverse=0;reverse<2;++reverse) {
        SceneDepth s;assert(s.init(W,H));s.clear();std::vector<uint16_t> screen(W*H,0);
        auto blit=[&](int x,int y,const uint16_t *p,int n) {std::copy(p,p+n,screen.begin()+y*W+x);};
        for(int j=0;j<2;++j) {
            const int id=j^reverse;
            const float l=id?.264f:.2f,r=id?.2f:.264f;
            const uint16_t color=id?0xf800:0x001f;
            T a={{{0,0,l,0,0},{W,0,r,0,0},{W,H,r,0,0}},color,0,H-1};
            T b={{{0,0,l,0,0},{W,H,r,0,0},{0,H,l,0,0}},color,0,H-1};
            s.triangle(a,nullptr,0,0,blit);s.triangle(b,nullptr,0,0,blit);
        }
        for(int y=0;y<H;++y)for(int x=0;x<W;++x)assert(screen[y*W+x]==(x<32?0xf800:0x001f));
    }
    printf("SCENE_DEPTH_PASS %d reference pixels, 24 draw orders, partial occlusion, front mesh, masked sprite, reset, allocation failures; 6144 crossing-plane pixels\n",checked);
}
