#include "dpvsLibrary.hpp"
#include "dpvsCell.hpp"
#include "dpvsCamera.hpp"
#include "dpvsModel.hpp"
#include "dpvsObject.hpp"
#include "dpvsCommander.hpp"
#include <stdio.h>
#include <float.h>
class Capture:public DPVS::Commander {
public: unsigned mask;int begins,ends;
 Capture():mask(0),begins(0),ends(0){}
 void command(Command c) {if(c==QUERY_BEGIN)++begins;if(c==QUERY_END)++ends;if(c==INSTANCE_VISIBLE){int id=*static_cast<int*>(getInstance()->getObject()->getUserPointer());mask|=1u<<id;}}
};
int main(){
_controlfp(_RC_NEAR,_MCW_RC);
#if defined(_M_IX86)
_controlfp(_PC_24,_MCW_PC);
#endif

 DPVS::Library::init(DPVS::Library::COLUMN_MAJOR);DPVS::Cell* cell=DPVS::Cell::create();DPVS::Camera* cam=DPVS::Camera::create();cam->setCell(cell);
 DPVS::Frustum f;f.left=-1;f.right=1;f.bottom=-1;f.top=1;f.zNear=1;f.zFar=100;f.type=DPVS::Frustum::PERSPECTIVE;cam->setFrustum(f);
 int ids[9]={0,1,2,3,4,5,6,7,8};DPVS::Object* o[9];DPVS::Model* m[9];float z[8]={4,4.9f,4.99f,4.999f,5.001f,5.01f,5.1f,6};
 for(int i=0;i<8;++i){DPVS::Vector3 v={{(i-3.5f)*0.1f,0,z[i]}};m[i]=DPVS::SphereModel::create(v,0.05f);o[i]=DPVS::Object::create(m[i]);}
 DPVS::Vector3 verts[4]={{{-3,-3,5}},{{-3,3,5}},{{3,3,5}},{{3,-3,5}}};DPVS::Vector3i tris[2]={{0,1,2},{0,2,3}};
 m[8]=DPVS::MeshModel::create(verts,tris,4,2,true);o[8]=DPVS::Object::create(m[8]);o[8]->setWriteModel(m[8]);
 for(int i=0;i<9;++i){o[i]->setUserPointer(&ids[i]);o[i]->set(DPVS::Object::INFORM_VISIBLE,true);o[i]->set(DPVS::Object::CONTRIBUTION_CULLING,false);o[i]->setCell(cell);}
 o[7]->setCost(100000,200000,1.0f);
 int failures=0;float writes=0,totalWrites=0;int firstHidden=-1,reappeared=0;
 for(int mode=1;mode<2;++mode){unsigned flags=DPVS::Camera::VIEWFRUSTUM_CULLING;if(mode)flags|=DPVS::Camera::OCCLUSION_CULLING;cam->setParameters(640,480,flags);DPVS::Library::resetStatistics();
 for(int frame=0;frame<(mode?64:20);++frame){DPVS::Library::resetStatistics();Capture c;cam->resolveVisibility(&c,1);writes=DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEWRITESPERFORMED);printf("mode=%d frame=%d visible=%u begin=%d end=%d writes=%.0f\n",mode,frame,c.mask,c.begins,c.ends,writes);printf("counters mode=%d frame=%d",mode,frame);printf(" WRITEQUEUEWRITESREQUESTED=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEWRITESREQUESTED));printf(" WRITEQUEUEWRITESDISCARDED=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEWRITESDISCARDED));printf(" WRITEQUEUEWRITESPOSTPONED=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEWRITESPOSTPONED));printf(" WRITEQUEUEFLUSHES=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEFLUSHES));printf(" WRITEQUEUEBUCKETFLUSHWORK=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEBUCKETFLUSHWORK));printf(" MODELWRITESILHOUETTESQUERIED=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_MODELWRITESILHOUETTESQUERIED));printf(" DATABASEOBSOCCLUSIONTESTED=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_DATABASEOBSOCCLUSIONTESTED));printf(" DATABASEOBSOCCLUSIONCULLED=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_DATABASEOBSOCCLUSIONCULLED));printf(" OCCLUSIONBUFFEREDGESRASTERIZED=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_OCCLUSIONBUFFEREDGESRASTERIZED));printf(" SKIPPED=%.0f VPTFAILED=%.0f POINT=%.0f OBJECT=%.0f",DPVS::Library::getStatistic(DPVS::Library::STAT_DATABASEOBSOCCLUSIONSKIPPED),DPVS::Library::getStatistic(DPVS::Library::STAT_OBJECTVPTFAILED),DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEPOINTQUERIES),DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEOBJECTQUERIES));puts("");totalWrites+=writes;if(c.begins!=1||c.ends!=1)++failures;if(!mode&&c.mask!=511)++failures;if(mode){if((c.mask&128)||DPVS::Library::getStatistic(DPVS::Library::STAT_WRITEQUEUEWRITESREQUESTED)<1)++failures;if(!(c.mask&1))++failures;if(!(c.mask&128)){if(firstHidden<0)firstHidden=frame;}else if(firstHidden>=0){++reappeared;}}}}
 if(totalWrites<=0||firstHidden<0||reappeared>0)++failures;printf("SUMMARY firstHidden=%d reappeared=%d totalWrites=%.0f failures=%d\n",firstHidden,reappeared,totalWrites,failures);
 for(int i=8;i>=0;--i){o[i]->setCell(0);o[i]->release();m[i]->release();}cam->setCell(0);cam->release();cell->release();DPVS::Library::exit();return failures?3:0;
}
