#include "dpvsLibrary.hpp"
#include "dpvsCell.hpp"
#include "dpvsCamera.hpp"
#include "dpvsModel.hpp"
#include "dpvsObject.hpp"
#include "dpvsCommander.hpp"
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
class ProbeServices:public DPVS::Library::Services{public:void error(const char* msg){fprintf(stderr,"DPVS_ASSERTION %s\n",msg);fflush(stderr);exit(86);}};
const int N=257;
class Capture:public DPVS::Commander {public:int counts[N],begins,ends,invalid;Capture():begins(0),ends(0),invalid(0){for(int i=0;i<N;++i)counts[i]=0;}void command(Command c){if(c==QUERY_BEGIN)++begins;if(c==QUERY_END)++ends;if(c==INSTANCE_VISIBLE){int i=*static_cast<int*>(getInstance()->getObject()->getUserPointer());if(i<0||i>=N)++invalid;else ++counts[i];}}};
int main(){
_controlfp(_RC_NEAR,_MCW_RC);
#if defined(_M_IX86)
_controlfp(_PC_24,_MCW_PC);
#endif
ProbeServices services;DPVS::Library::init(DPVS::Library::COLUMN_MAJOR,&services);DPVS::Cell* cell=DPVS::Cell::create();DPVS::Camera* cam=DPVS::Camera::create();cam->setCell(cell);DPVS::Frustum f;f.left=-1;f.right=1;f.bottom=-1;f.top=1;f.zNear=1;f.zFar=100;f.type=DPVS::Frustum::PERSPECTIVE;cam->setFrustum(f);cam->setParameters(640,480,DPVS::Camera::VIEWFRUSTUM_CULLING|DPVS::Camera::OCCLUSION_CULLING,0.5f,0.5f);cam->setObjectMinimumCoverage(8,8,1);
int ids[N];DPVS::Object* ob[N];DPVS::Model* models[N];
for(int i=0;i<256;++i){DPVS::Vector3 v={{0,0,2}};if(i>0&&i<128){v.v[0]=float(i%16-8)*0.3f;v.v[1]=float(i/16-4)*0.3f;v.v[2]=10+float(i%3);}else if(i>=128){v.v[0]=100+float(i%16)*3;v.v[1]=float(i/16)*3;v.v[2]=10;}models[i]=DPVS::SphereModel::create(v,0.3f);ob[i]=DPVS::Object::create(models[i]);}
DPVS::Vector3 verts[4]={{{-4,-4,5}},{{-4,4,5}},{{4,4,5}},{{4,-4,5}}};DPVS::Vector3i tris[2]={{0,1,2},{0,2,3}};models[256]=DPVS::MeshModel::create(verts,tris,4,2,true);ob[256]=DPVS::Object::create(models[256]);ob[256]->setWriteModel(models[256]);for(int i=0;i<N;++i){ids[i]=i;ob[i]->setUserPointer(&ids[i]);ob[i]->setCell(cell);}
int fail=0;for(int q=0;q<192;++q){if(q==64){for(int i=128;i<256;i+=4)ob[i]->setCell(0);}Capture c;cam->resolveVisibility(&c,8,0.0f);DPVS::Library::checkConsistency();if(c.begins!=1||c.ends!=1||c.invalid||c.counts[0]!=1)++fail;for(int i=0;i<N;++i)if(c.counts[i]>1)++fail;printf("QUERY q=%d sentinel=%d begin=%d end=%d invalid=%d visible=",q,c.counts[0],c.begins,c.ends,c.invalid);for(int i=0;i<N;++i)if(c.counts[i])printf("%d,",i);puts("");fflush(stdout);}for(int i=N-1;i>=0;--i){ob[i]->setCell(0);ob[i]->release();models[i]->release();}cam->setCell(0);cam->release();cell->release();DPVS::Library::exit();printf("RESULT failures=%d\n",fail);return fail?3:0;}
