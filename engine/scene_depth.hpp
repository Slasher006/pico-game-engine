#pragma once
#include "mesh_visibility.hpp"
#ifdef ENGINE_USE_PSRAM
#include "triangle_psram.hpp"
#endif

// Opt-in opaque scene depth. Compact inverse depth in PSRAM on supported boards;
// only scanline scratch stays in the internal heap. All world draws share it.
class SceneDepth {
    uint16_t width=0, height=0;
    uint16_t *depth=nullptr, *scratch=nullptr, *colors=nullptr;
    uint8_t *covered=nullptr;
#ifdef ENGINE_USE_PSRAM
    uint32_t address=TrianglePSRAM::INVALID;
#endif
    void read(int y,int x,int n) {
#ifdef ENGINE_USE_PSRAM
        ENGINE_PSRAM_READ(address+2*(y*width+x),scratch+x,2*n);
#else
        memcpy(scratch+x,depth+y*width+x,2*n);
#endif
    }
    void write(int y,int x,int n) {
#ifdef ENGINE_USE_PSRAM
        ENGINE_PSRAM_WRITE(address+2*(y*width+x),scratch+x,2*n);
#else
        memcpy(depth+y*width+x,scratch+x,2*n);
#endif
    }
    // Positive float ordering is preserved with 11 mantissa bits and five
    // exponent bits. Avoid a reciprocal and two libm clamps for every pixel.
    // Normal game depths (near plane 0.1 through 256 units) fit without clamping.
    static uint16_t encodeInverse(float inverse) {
        if (!(inverse > 0)) return 0;
        uint32_t bits;
        static_assert(sizeof(bits) == sizeof(inverse), "32-bit float required");
        memcpy(&bits, &inverse, sizeof(bits));
        const uint32_t compact = bits >> 12, bias = 110u << 11;
        if (compact <= bias) return 1;
        if (compact >= bias + 65535) return 65535;
        return uint16_t(compact - bias);
    }
    template<class Blit> void finish(int y,int left,int right,Blit blit) {
        write(y,left,right-left+1);
        for(int x=left;x<=right;) {
            while(x<=right && !covered[x]) ++x;
            const int start=x;
            while(x<=right && covered[x]) ++x;
            if(x>start) blit(start,y,colors+start,x-start);
        }
    }
public:
    ~SceneDepth() {
        ENGINE_MEM_FREE(depth); ENGINE_MEM_FREE(scratch); ENGINE_MEM_FREE(colors); ENGINE_MEM_FREE(covered);
#ifdef ENGINE_USE_PSRAM
        if(address!=TrianglePSRAM::INVALID) TrianglePSRAM::release();
#endif
    }
    SceneDepth() = default;
    SceneDepth(const SceneDepth &)=delete;
    SceneDepth &operator=(const SceneDepth &)=delete;
    bool init(uint16_t w,uint16_t h) {
        if(!w || !h || w>2048 || h>2048 || width) return false;
        scratch=static_cast<uint16_t *>(ENGINE_MEM_MALLOC(w*2));
        colors=static_cast<uint16_t *>(ENGINE_MEM_MALLOC(w*2));
        covered=static_cast<uint8_t *>(ENGINE_MEM_MALLOC(w));
        if(!scratch || !colors || !covered) return false;
#ifdef ENGINE_USE_PSRAM
        address=TrianglePSRAM::reserveBytes(uint32_t(w)*h*2);
        if(address==TrianglePSRAM::INVALID) return false;
        TrianglePSRAM::retain();
#else
        depth=static_cast<uint16_t *>(ENGINE_MEM_MALLOC(size_t(w)*h*2));
        if(!depth) return false;
#endif
        width=w;height=h;return true;
    }
    void clear() {
        memset(scratch,0,width*2);
        for(int y=0;y<height;++y) write(y,0,width);
    }
    template<class Blit>
    void triangle(const MeshVisibility::Triangle &t,const uint16_t *texture,
                  uint16_t tw,uint16_t th,Blit blit) {
        for(int y=t.first;y<=t.last;++y) {
            MeshVisibility::Vertex ends[2]; int found=0;const float sy=y+.5f;
            for(int j=0;j<3;++j) {
                const auto &a=t.v[j],&b=t.v[(j+1)%3];
                if((sy>=a.y && sy<b.y)||(sy>=b.y && sy<a.y)) {
                    const float f=(sy-a.y)/(b.y-a.y);
                    ends[found++]={a.x+f*(b.x-a.x),sy,a.z+f*(b.z-a.z),a.u+f*(b.u-a.u),a.v+f*(b.v-a.v)};
                }
            }
            if(found!=2) continue;
            if(ends[0].x>ends[1].x) {auto swap=ends[0];ends[0]=ends[1];ends[1]=swap;}
            const float span=ends[1].x-ends[0].x;
            if(span<=0) continue;
            const int left=int(ceilf(fmaxf(0,fminf(width,ends[0].x-.5f))));
            const int right=int(ceilf(fmaxf(0,fminf(width,ends[1].x-.5f))))-1;
            if(right<left) continue;
            read(y,left,right-left+1);memset(covered+left,0,right-left+1);
            bool changed=false;
            const float inverseStep=(ends[1].z-ends[0].z)/span;
            float inverse=ends[0].z+(left+.5f-ends[0].x)*inverseStep;
            for(int x=left;x<=right;++x,inverse+=inverseStep) {
                const uint16_t z=encodeInverse(inverse);
                if(z<=scratch[x]) continue;
                uint16_t color=t.tint;
                if(texture && tw && th) {
                    const float f=(x+.5f-ends[0].x)/span;
                    const float u=ends[0].u+f*(ends[1].u-ends[0].u),v=ends[0].v+f*(ends[1].v-ends[0].v);
                    const int tx=int(fmaxf(0,fminf(tw-1,u*tw))),ty=int(fmaxf(0,fminf(th-1,v*th)));
                    const uint16_t texel=texture[ty*tw+tx];
                    const unsigned r=(texel>>11)&31,g=(texel>>5)&63,b=texel&31;
                    color=uint16_t(((r*((t.tint>>11)&31)/31)<<11)|((g*((t.tint>>5)&63)/63)<<5)|(b*(t.tint&31)/31));
                }
                colors[x]=color;scratch[x]=z;covered[x]=1;changed=true;
            }
            if(changed) finish(y,left,right,blit);
        }
    }
    template<class Blit>
    void billboard(int bx,int by,int bw,int bh,float distance,int sw,int sh,
                   const uint8_t *pixels,const uint8_t *mask,Blit blit) {
        if(bw<=0 || bh<=0 || sw<=0 || sh<=0 || !isfinite(distance) || distance<=0) return;
        const int left=bx<0?0:bx,right=bx+bw>width?width:bx+bw;
        const int top=by<0?0:by,bottom=by+bh>height?height:by+bh;
        if(left>=right || top>=bottom) return;
        const uint16_t z=encodeInverse(1/distance);
        for(int y=top;y<bottom;++y) {
            read(y,left,right-left);memset(covered+left,0,right-left);bool changed=false;
            const int sy=(y-by)*sh/bh;
            for(int x=left;x<right;++x) {
                const int i=sy*sw+(x-bx)*sw/bw;
                if(!(mask[i>>3]&(0x80>>(i&7))) || z<=scratch[x]) continue;
                const unsigned p=pixels[i];
                colors[x]=uint16_t((((p>>5)*31/7)<<11)|((((p>>2)&7)*63/7)<<5)|((p&3)*31/3));
                scratch[x]=z;covered[x]=1;changed=true;
            }
            if(changed) finish(y,left,right-1,blit);
        }
    }
};
