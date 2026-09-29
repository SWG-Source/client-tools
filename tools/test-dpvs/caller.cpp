// Compile this TU instead of dpvsMath.obj; implementation is included unchanged.
#include "dpvsPrivateDefs.hpp"
#if defined(PROBE_SCALAR_MATH)
#undef DPVS_X86_ASSEMBLY
#endif
#include "dpvsMath.cpp"
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
static unsigned bits(float f){unsigned u;memcpy(&u,&f,4);return u;}
static float value(unsigned u){float f;memcpy(&f,&u,4);return f;}
static unsigned rng=0x12345678u;
static unsigned next(){rng=rng*1664525u+1013904223u;return rng;}
static int failures=0,rows=0;
static void state(const char* op,unsigned before,unsigned mx){unsigned after=_controlfp(0,0);unsigned ma=_mm_getcsr();if((before&(_MCW_RC|_MCW_PC))!=(after&(_MCW_RC|_MCW_PC)) || (mx&0xffc0)!=(ma&0xffc0)){++failures;printf("STATE_FAIL %s\n",op);}}
int main(){
_controlfp(_RC_NEAR,_MCW_RC);
#if defined(_M_IX86)
_controlfp(_PC_64,_MCW_PC);
#endif
_MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
#if defined(_M_IX86)
 unsigned short rawcw=0; __asm fnstcw rawcw
 printf("RAW_X87 cw=%04x\n",unsigned(rawcw));
 if((rawcw & 0x0f00)!=0x0300) return 91;
#else
 printf("RAW_X87 not-applicable-x64\n");
#endif
#if defined(DPVS_X86_ASSEMBLY)
 printf("DISPATCH assembly=1 cmov=%u\n",unsigned(DPVS::X86::getInstructionSets()&DPVS::X86::S_CMOV));
#else
 printf("DISPATCH assembly=0\n");
#endif

int errors=0;float scales[2]={8.0f,16.0f*0.45f};
for(int sc=0;sc<2;++sc)for(int k=-100;k<=4096;k+=3){float center=float(k)/scales[sc];for(int delta=-1;delta<=1;++delta){unsigned u=bits(center);if(u==0&&delta<0)continue;float x=value(u+delta);DPVS::Vector2 v(x,x);DPVS::Vector2i out;DPVS::Vector2 scale(scales[sc],scales[sc]);DPVS::Math::rasterToFixed(&out,&v,scale,1);int ref=int(floor(double(x)*double(scales[sc])));printf("R %d %d %d %08x %d %d\n",sc,k,delta,bits(x),out[0],ref);if(out[0]!=ref)++errors;}}
const float eps[7]={-0.0001f,-0.000001f,-0.0000001f,0,0.0000001f,0.000001f,0.0001f};
for(int i=0;i<512;++i){DPVS::Vector3 a(float(i%17)*0.13f,float(i%23)*0.17f,2.0f+float(i%29)*0.11f);DPVS::Vector3 b(a.x+1.1f,a.y+0.3f,a.z+0.7f);DPVS::Vector3 c(a.x-0.2f,a.y+1.3f,a.z+float(i%11)*0.09f);DPVS::Vector4 plane=DPVS::Math::getNormalizedPlaneEquation(a,b,c);for(int e=0;e<7;++e){DPVS::Vector4 cam(a.x+plane.x*eps[e],a.y+plane.y*eps[e],a.z+plane.z*eps[e],1);float out;DPVS::Math::dot(&out,&plane,cam,1);printf("C %d %d %08x %d %08x %08x %08x %08x %08x %08x %08x\n",i,e,bits(out),(out>0)-(out<0),bits(plane.x),bits(plane.y),bits(plane.z),bits(plane.w),bits(cam.x),bits(cam.y),bits(cam.z));}}
printf("RESULT rasterExactFailures=%d\n",errors);return errors?3:0;}
