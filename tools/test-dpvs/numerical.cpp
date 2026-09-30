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
#if defined(DPVS_X86_ASSEMBLY)
 const int assembly=1;unsigned cmov=DPVS::X86::getInstructionSets()&DPVS::X86::S_CMOV;
#else
 const int assembly=0;unsigned cmov=0;
#endif
 printf("META pointer=%u assembly=%d cmov=%u scalarControl=%d\n",unsigned(sizeof(void*)),assembly,cmov,
#if defined(PROBE_SCALAR_MATH)
 1
#else
 0
#endif
 );
 unsigned original=_controlfp(0,0),originalMx=_mm_getcsr();
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

 unsigned control=_controlfp(0,0),mx=_mm_getcsr();printf("MODE control=%08x mxcsr=%08x\n",control,mx);
 const int sizes[]={1,2,3,4,5,6,7,8,9,16,31,64};
 const unsigned cases[]={0x00000000,0x80000000,0x3f7fffff,0x3f800000,0x3f800001,0xbf7fffff,0xbf800000,0xbf800001,0x3effffff,0x3f000000,0x3f000001,0xbeffffff,0xbf000000,0xbf000001,0x437fffff,0x43800000,0x43800001,0xc37fffff,0xc3800000,0xc3800001,0x477fffff,0x47800000,0xc77fffff,0xc7800000,0x4affffff,0xcaffffff};
 for(int batch=0;batch<40;++batch)for(int si=0;si<12;++si){int n=sizes[si];float f[64];DPVS::INT32 out[64];DPVS::Vector2 v[64];DPVS::Vector2i fixed[64];DPVS::Vector3 v3[64];DPVS::Vector4 v4[64];float dots[64];
 for(int i=0;i<n;++i){unsigned u=cases[(batch+i)%26];if(batch>=26){u=(next()&0x807fffffu)|((118+next()%28)<<23);}f[i]=value(u);v[i]=DPVS::Vector2(f[i],-f[i]);v3[i]=DPVS::Vector3(f[i],-f[i],f[(i?i-1:0)]);v4[i]=DPVS::Vector4(f[i],-f[i],value(0x3f800001),1.0f);}
 DPVS::Math::intFloor(out,f,n);state("floor",control,mx);
 for(int i=0;i<n;++i){int expected=(int)floor((double)f[i]);if(out[i]!=expected)++failures;printf("F %d %d %d %08x %d %d\n",batch,n,i,bits(f[i]),out[i],expected);++rows;}
 // Positive scales, exact powers of two and nonbinary fractional boundaries.
 const float scales[]={1.0f,16.0f,0.1f,0.3f};
 for(int sc=0;sc<4;++sc){DPVS::Vector2 scale(scales[sc],scales[sc]);DPVS::Math::rasterToFixed(fixed,v,scale,n);state("raster",control,mx);for(int i=0;i<n;++i){int ex=(int)floor((double)v[i].x*(double)scale.x),ey=(int)floor((double)v[i].y*(double)scale.y);if(fixed[i][0]!=ex||fixed[i][1]!=ey)++failures;printf("R %d %d %d %d %08x %08x %d %d %d %d\n",batch,n,sc,i,bits(v[i].x),bits(scale.x),fixed[i][0],fixed[i][1],ex,ey);++rows;}}
 DPVS::Vector3 lo,hi;DPVS::Math::minMax(lo,hi,v3,n);state("minmax",control,mx);for(int c=0;c<3;++c){float a=v3[0][c],b=a;for(int i=1;i<n;++i){if(v3[i][c]<a)a=v3[i][c];if(v3[i][c]>b)b=v3[i][c];}if(lo[c]!=a||hi[c]!=b)++failures;printf("M %d %d %d %08x %08x\n",batch,n,c,bits(lo[c]),bits(hi[c]));++rows;}
 DPVS::Vector4 p(1.00000011920928955078125f,1.0f,-1.0f,1.0f);DPVS::Math::dot(dots,v4,p,n);state("dot",control,mx);for(int i=0;i<n;++i){double ref=0,mag=0;for(int c=0;c<4;++c){double term=(double)v4[i][c]*(double)p[c];ref+=term;mag+=fabs(term);}double tolerance=8.0*FLT_EPSILON*mag+1e-30;if(!_finite(dots[i])||fabs((double)dots[i]-ref)>tolerance)++failures;printf("D %d %d %d %08x %.17g %.17g\n",batch,n,i,bits(dots[i]),ref,tolerance);++rows;}
 }
 _controlfp(original,_MCW_RC
#if defined(_M_IX86)
 |_MCW_PC
#endif
 );_mm_setcsr(originalMx);printf("RESULT rows=%d failures=%d\n",rows,failures);return failures?3:0;
}
