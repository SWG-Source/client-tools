#ifndef __DPVSIMPOBJECT_HPP
#define __DPVSIMPOBJECT_HPP
/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description: 	Object implementation class
 *
 * $Archive: /dpvs/implementation/include/dpvsImpObject.hpp $
 * $Author: wili $ 
 * $Revision: #4 $
 * $Modtime: 2.10.02 13:31 $
 * $Date: 2003/06/19 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif
#if !defined (__DPVSOBJECT_HPP)
#	include "dpvsObject.hpp"
#endif
#if !defined (__DPVSAABB_HPP)
#	include "dpvsAABB.hpp"
#endif
#if !defined (__DPVSMATRIX_HPP)
#	include "dpvsMatrix.hpp"
#endif
#if !defined(__DPVSSTATISTICS_HPP)			// setStatus() call
#	include "dpvsStatistics.hpp"
#endif
#if !defined(__DPVSDATAPASSER_HPP)
#	include "dpvsDataPasser.hpp"
#endif
#if !defined(__DPVSSPHERE_HPP)
#	include "dpvsSphere.hpp"
#endif
#if !defined(__DPVSIMPMODEL_HPP)
#	include "dpvsImpModel.hpp"
#endif

namespace DPVS
{
template <class T> class Set;
class Ob;
class Cell;
class ImpCell;
class ImpCamera;
class FloatRectangle;
class ImpCellDirtyObjectList;
struct ObInstance;
struct VertexArray;

// a tiny hack so that we can set and get the reserved0 value :)
class HackUserObject : public Object
{
public:
	DPVS_FORCE_INLINE void		setReserved0	(UINT32 r)		{ m_reserved0 = r;		}
	DPVS_FORCE_INLINE UINT32	getReserved0	(void) const	{ return m_reserved0;	}
protected:
	 							HackUserObject	(void);								// shut up the compiler
private:
                        HackUserObject          (const HackUserObject&);            // not allowed
    HackUserObject&     operator=               (const HackUserObject&);            // not allowed
};

/******************************************************************************
 *
 * Class:			DPVS::ImpObject
 *
 * Description:		Implementation of Object
 *
 *****************************************************************************/

class ImpObject
{
friend class ImpCell;
public:

	// Note: if new portal types are added, please adjust the isPortal() function call
	// Note: this enumeration is currently packed into 3 bits. If there's more than 8 enums,
	// adjust the size of the m_type variable accordingly.
	enum ObjectType		
	{
		TYPE_BASE					= 0,
		TYPE_OBJECT					= 1,
		TYPE_REGION_OF_INFLUENCE	= 2,
		TYPE_PHYSICAL_PORTAL		= 3,
		TYPE_VIRTUAL_PORTAL			= 4
	};

	enum Status	// visibility status
	{
		HIDDEN						= 0,	// hidden
		VISIBLE						= 1		// visible
	};

	enum TestState
	{
		VF_DONE						= 0,	// VF culling has been done
		OC_DONE						= 1		// occlusion culling test has been done
	};

	void				setupQuickBounds			(AABB& aabb);		// called from "dpvsWrapper.cpp"
private:

	struct VisibilityRelation 
	{
		ImpObject*			m_parent;					// Pointer to parent
		ImpObject*			m_child;					// Pointer to child
		VisibilityRelation*	m_prev;						// Pointer to previous relation (of the same parent)
		VisibilityRelation*	m_next;						// Pointer to next relation (of the same parent)
	};

	struct Dynamic 
	{
		float				m_predictionLength;			// DYNAMIC number of seconds used for TBV size estimation
		float				m_lastLocationTime;			// DYNAMIC when was last location sample taken (motion prediction)
		Vector3				m_lastLocation;				// DYNAMIC last location sample (motion prediction)
		Vector3				m_velocity;					// DYNAMIC current velocity (motion prediction)
	};

						ImpObject					(const ImpObject&);	// not allowed
	ImpObject&			operator=					(const ImpObject&);	// not allowed
	void				checkEnableStatusRecursive	(void);
	void				enableInternal				(bool);
	bool				expectHidden				(void) const			{ return (DataPasser::getQueryTime() - m_lastVisibleTime) >= 0.2f; }
	void				linkToDirtyListHead			(void);
	void				linkToDirtyListTail			(void);
	void				setBoundsDirty				(void);
	void				unlinkFromDirtyList			(void);
	void				updateBounds				(void);
	void				setupExactBounds			(void);
	void				unlinkVisibilityChildren	(void);
	void				createNewTestVertex			(void);

	void				updateOcclusionWritable		(void);
	// DEBUG DEBUG - temporarily changed public!
public:
	float				estimateBenefit				(void) const;
	float				estimateWriteCost			(void) const { return m_writeCost; }
	void				recalculateWriteCost		(float previousCost);

private:
	void				validateTBV					(void);

#if defined (DPVS_VFT_IN_END)							// apply padding on platforms that don't have VFT in beginning!
	UINT32				m_padding;
#endif
	// so far we have 20 bytes (16 bytes from ReferenceCount + 4 from the VFT or the padding)

	//--------------------------------------------------------------------
	// Volatile stuff that is accessed often
	//--------------------------------------------------------------------

	UINT32				m_lastStatus;					// ALL 32-bit status of last query (i.e. supports up to 32 different cameras)
	UINT32				m_clipMask;						// REMOVE!!! current clip mask, required only for visible objects

	//--------------------------------------------------------------------
	// Data that is read every frame / often and modified infrequently
	//--------------------------------------------------------------------

 	UINT32				m_hasVisibilityParent	: 1;	// ALL object has visibility parent?
	UINT32				m_forceSelect			: 1;	// VISIBLE?
	UINT32				m_dirtyTestVertex		: 1;	// REMOVE? is the test vertex dirty?
	UINT32				m_inDirtyList			: 1;	// ALL object is in dirty bounds list?
	UINT32				m_firstBounds			: 1;	// ALL is this the first time bounds are calculated?
	UINT32				m_static				: 1;	// ALL predicted by the system (assume static) -> used by TBV generator
	UINT32				m_enabled				: 1;	// ALL object enabled?
	UINT32				m_selfEnabled			: 1;	// ALL is object "personally enabled" (m_enabled is computed from this)
	UINT32				m_exactDatabaseFit		: 1;	// ALL should object be fitted exactly into the database?
	UINT32				m_contributionCulling	: 1;	// ALL is contribution culling enabled?
	UINT32				m_informVisible			: 1;	// ALL inform user that object was visible?
	UINT32				m_floatingPortal		: 1;	// ALL is the "portal" a floating portal
	UINT32				m_informPortalEnter		: 1;	// ALL send "portal enter" message to Commander
	UINT32				m_informPortalExit		: 1;	// ALL send "portal exit" message to Commander
	UINT32				m_informPortalPreExit	: 1;	// ALL send "portal pre exit" message to Commander
	UINT32				m_abortable				: 1;	// ALL abort visibility query if object visible?
	UINT32				m_objectToCellUniform	: 1;	// ALL is object->cell matrix uniform?
	UINT32				m_occlusionWritable		: 1;	// ALL can object be used as an occluder? 
	UINT32				m_userOccluder			: 1;	// ALL user-side occlusion enable/disable (this is applied to m_occlusionWritable)
	UINT32				m_unbounded				: 1;	// ALL object regarded to be unbounded

	UINT32				m_hasDynamic			: 1;	// ALL has non-NULL "Dynamic" pointer
	UINT32				m_reportImmediately		: 1;	// ALL report visibility immediately?
	UINT32				m_portal				: 1;	// ALL is object a portal?
	UINT32				m_type					: 3;	// (really ObjectType enumeration)
	UINT32				m_averageCount			: 5;	// Average counter
	UINT32				m_testState				: 1;	// ALL which part of testing has currently been done? (0 = VF cull, 1 = occ cull)
// ---------------- cache line 1 ends -------------------

	// HMM.. maybe we should pack the render and write costs? (UINT16 or even UINT8 each..)

	ImpModel*			m_testModel;					// ALL internally used test model ptr
	float				m_renderCost;					// ALL estimated render cost for cost-benefit analysis
	float				m_writeCost;					// ALL (REMOVE?) estimated render cost for cost-benefit analysis (TEMP)
	ImpModel*			m_writeModel;					// ALL internally used write model ptr
	float				m_objectToCellMatrix[4*3];		// ALL object->cell matrix (ALIGNMENT HANDLED INTERNALLY!)

// ---------------- cache line 3 ends -------------------

	AABB				m_TBV;							// ALL temporal bounding volume AABB (or exact if object is static)

	VisibilityRelation*	m_visibilityParent;				// ALL pointer to visibility relation (may be null)
	VisibilityRelation*	m_firstVisibilityChild;			// ALL (REMOVE?) pointer to visibility relation (may be null)

// ---------------- cache line 4 ends -------------------

	UINT32				m_hiddenTimeStamp;				// REMOVE when this object was hidden (do we need this?)
	float				m_lastVisibleTime;				// VISIBLE time when object was last visible (actual time in seconds, not timestamped)
	float				m_lastFrameBenefit;				// DEBUG DEBUG DEBUG
	float				m_currentBenefit;				// VISIBLE benefit for cost-benefit analysis
	float				m_average;						// VISIBLE average benefit
	UINT32				m_benefitTimeStamp;				// VISIBLE benefit time stamp (when was benefit last computed)
	UINT32				m_goodOccluderTimeStamp;		// VISIBLE when was goodOccluder() last called?
	UINT32				m_lastOccluderUsedTimeStamp;	// VISIBLE when was the object last used as an occluder?

// ---------------- cache line 5 ends -------------------

	Vector3				m_testVertex;					// VISIBLE test vertex used by the test vertex algorithms TODO: COMPRESS!
	Cell*				m_cell;							// ALL pointer back to cell
	ImpObject*			m_cellPrev;						// ALL previous in same cell
	ImpObject*			m_cellNext;						// ALL next in same cell
	ImpObject*			m_prev;							// ALL previous in world-linked list
	ImpObject*			m_next;							// ALL next in world-linked list

// ---------------- cache line 6 ends -------------------
	
	ObInstance*			m_firstInstance;				// ALL first ObInstance of the object (head of linked list)
	ImpObject*			m_prevDirty;					// DYNAMIC next object in dirty list
	ImpObject*			m_nextDirty;					// DYNAMIC prev object in dirty list
	float				m_dirtyTimeStamp;				// DYNAMIC dirty check timestamp
	Dynamic*			m_dynamic;						// data for dynamic objects (null if not dynamic -- always check this ptr first please!)

#if !defined (DPVS_VFT_IN_END)							// apply padding on platforms that have VFT in the beginning!
	UINT32				m_padding;
#endif

	// sizeof(Object) should be 216 (i.e. 224 bytes - 8 bytes)
	// here we leave (7 cache lines - 8 bytes) (the mem allocator uses the 8 bytes)


	//--------------------------------------------------------------------
	// Shared data between objects
	//--------------------------------------------------------------------

	static bool			s_forceSelectAll;				// select all objects as occluders during this frame?
	static int			s_dynamicToStaticWork;			// amount of dynamic->static work during this frame	
	static ImpObject*	s_head;							// head of linked list of objects

protected:
	
	void				setType						(ObjectType t);

public:

						ImpObject					(Model* testModel);
	virtual				~ImpObject					(void);

	static void			resetObjectTimeStamps		(void);
	static void			suggestGarbageCollect		(Commander* cmd, float);
	static void			checkConsistencyAll			(void);					// check consistency of all objects..

#if defined (DPVS_DEBUG)
	void				checkConsistency			(void) const;
#else
	void				checkConsistency			(void) const			{}
#endif

	void				addBenefit					(float b)								{ m_currentBenefit += b; }
	bool				backFaceCull				(class ImpCamera* c) const;				
	void				selfEnable					(bool v);
	void				forceSelect					(void)					{ m_forceSelect = true; }	// used to force select object as an occluder during this frame

	UINT32				getBenefitTimeStamp			(void) const			{ return m_benefitTimeStamp; }
	Cell*				getCell						(void) const			{ return m_cell; }
	const AABB&			getCellSpaceAABB			(void) const			{ return m_TBV;	}
	Sphere				getCellSpaceSphere			(void) const;	// NOT CHEAP!
	UINT32				getHiddenTimeStamp			(void) const			{ return m_hiddenTimeStamp;				}
	UINT32				getLastOccluderUsedTimeStamp(void) const			{ return m_lastOccluderUsedTimeStamp;	}
	
	float				getLastVisibleTime			(void) const			{ return m_lastVisibleTime;	}
	const OBB&			getModelOBB					(void)					{ return m_testModel->getOBB(); }
	const Matrix4x3&	getObjectToCameraMatrix		(ImpCamera* c) const;
	const Matrix4x3&	getObjectToCell				(void) const			{ return (const Matrix4x3&)(m_objectToCellMatrix[0]); }
	float				getRenderCost				(void) const			{ return m_renderCost + m_writeCost * 0.30f;		}
	void				getTestMesh					(VertexArray& v) const	{ m_testModel->getTestMesh(v); }
	const Mesh*			getTestMesh					(void) const			{ return m_testModel->getTestMesh(); }
	ImpModel*			getTestModel				(void) const			{ return m_testModel; }
	const Vector3&		getTestVertex				(void) 					{ if (m_dirtyTestVertex) createNewTestVertex(); return m_testVertex; }
	ObjectType			getType						(void) const			{ return (ObjectType)m_type;		}
	UINT32				getUserClipMask				(void) const			{ return m_clipMask >> 1; }		//skip ZERO plane
	HackUserObject*		getUserObject				(void) const  			{ return ((HackUserObject*)(const_cast<ImpObject*>(this)))-1;/*DPVS_ASSERT(m_userObject && m_userObject->getImplementation()==this); return m_userObject;*/	}
	Model*				getUserTestModel			(void) const			{ return m_testModel->getUserModel();	}
	Model*				getUserWriteModel			(void) const			{ return m_writeModel ? m_writeModel->getUserModel() : null;	}
	ImpObject*			getVisibilityParent			(void) const			{ return m_visibilityParent ? m_visibilityParent->m_parent : null;	}

	bool				getTestRectangle			(FloatRectangle &r);
	bool				getTestSilhouette			(EdgeSilhouette &s)		{ return m_testModel->getTestSilhouette (s); }
	TestState			getTestState				(void) const			{ return (TestState)(m_testState); }
	ImpModel*			getWriteModel				(void) const			{ return m_writeModel; }
	bool				getWriteRectangle			(FloatRectangle &r);
	bool				getWriteSilhouette			(EdgeSilhouette &s, Range<float>&, ImpCamera *c);

	bool				goodOccluder				(bool enableForceSelect);				

	bool				hasVisibilityParent			(void) const			{ return m_hasVisibilityParent; }
	bool				informPortalEnter			(void) const			{ return m_informPortalEnter; }
	bool				informPortalExit			(void) const			{ return m_informPortalExit; }
	bool				informPortalPreExit			(void) const			{ return m_informPortalPreExit; }
	bool				informVisible				(void) const			{ return m_informVisible; }
	void				initBenefit					(UINT32 time);
	bool				intersectCellSpaceAABB		(const AABB&); // not const as may trigger some recalcs

	bool				isAbortable					(void) const			{ return m_abortable; }
	bool				isBackFaceCullable			(void) const			{ return m_testModel->isBackFaceCullable(); }
	bool				isContributionCullingEnabled(void) const			{ return m_contributionCulling; }
	bool				isDirty						(void) const			{ return m_inDirtyList; }
	bool				isEnabled					(void) const			{ return m_enabled;		}
	bool				isFloatingPortal			(void) const			{ return m_floatingPortal; }
	bool				isObjectToCellUniform		(void) const			{ return m_objectToCellUniform; }
	bool				isOcclusionWritable			(void) const			{ return m_occlusionWritable; }
	bool				isPortal					(void) const			{ return m_portal; }
	bool				isRegionOfInfluence			(void) const			{ return (getType() == TYPE_REGION_OF_INFLUENCE); }
	bool				isSelfEnabled				(void) const			{ return m_selfEnabled; }
	bool				isStatic					(void) const			{ return m_exactDatabaseFit; }	// NOTE THAT WE USE M_EXACTDATABASEFIT VARIABLE!!
	bool				isUnbounded					(void) const			{ return m_unbounded;		}
	bool				isUserOccluder				(void) const			{ return m_userOccluder; }
	bool				isVisibilityParent			(void) const			{ return m_firstVisibilityChild != null; } // is the object a visibility parent?

	bool				reportImmediately			(void) const			{ return m_reportImmediately; }
	void				resetBenefitTimeStamp		(void)					{ m_benefitTimeStamp = 0; m_goodOccluderTimeStamp = (UINT32)(0); }

	void				setBenefitTimeStamp			(UINT32 t)				{ m_benefitTimeStamp = t;	}
	void				setCell						(Cell* c);
	void				setClipMask					(UINT32 msk)			{ if (msk != m_clipMask) m_clipMask = msk; }
	void				setAbortable				(bool v)				{ m_abortable = v; }
	void				setContributionCulling		(bool v)				{ m_contributionCulling = v; }
	void				setFloatingPortal			(bool v)				{ m_floatingPortal = v; }
	void				setHiddenTimeStamp			(UINT32 t)				{ m_hiddenTimeStamp=t; m_forceSelect = true;		}
	void				setInformPortalEnter		(bool v)				{ m_informPortalEnter = v; }
	void				setInformPortalExit			(bool v)				{ m_informPortalExit = v; }
	void				setInformPortalPreExit		(bool v)				{ m_informPortalPreExit = v; }
	void				setInformVisible			(bool v)				{ m_informVisible = v; }
	void				setLastOccluderUsedTimeStamp(UINT32 t)				{ m_lastOccluderUsedTimeStamp = t;	}
	void				setReportImmediately		(bool v)				{ m_reportImmediately = v; }
	void				setObjectToCell				(const Matrix4x3 &mtx);
	void				setRenderCost				(float cost);
	void				setRenderCost				(int vnum,int tnum,float complexity);
	void				setTestModel				(Model *m);
	void 				setTestState				(TestState testState) 	{ m_testState = testState;}
	void				setTestVertex				(const Vector3& tv)		{ m_dirtyTestVertex = false; m_testVertex = tv; }
	void				setUnbounded				(bool v);
	void				setUserOccluder				(bool v)				{ m_userOccluder = v; updateOcclusionWritable(); }
	void				setVisibilityParent			(ImpObject* parent);
	void				setVisibleTime				(void)					{ m_lastVisibleTime = DataPasser::getQueryTime(); }
	void				setWriteModel				(Model *m);

	void				testVertexFailed			(void)					{ m_dirtyTestVertex = true;	}

	// DATABASE API
	bool				assumeVisible				(UINT32 cameraIDMask) const { return (m_lastStatus & cameraIDMask)!=0;							}
	ObInstance*			getFirstInstance			(void) const				{ return m_firstInstance;											}
	unsigned int		getTimeStamp				(void) const				{ return getUserObject()->getReserved0();	 /* !!!!! */			}
	void				setFirstInstance			(ObInstance* inst)			{ m_firstInstance = inst;											}
	void				setTimeStamp				(unsigned int newTime)		{ getUserObject()->setReserved0(newTime);							}		
	void				setStatus					(Status s, UINT32 mask);		
	
	static void			forceSelectAll				(bool b)								{ s_forceSelectAll = b; }
	static void			updateAllDirtyBounds		(ImpCellDirtyObjectList&);

	// COST-RELATED
	enum	// all costs are P3 CPU cycles (very approximate)
	{
		RENDER_BASE_COST			= 15000 + 8000,		// 15000 = dPVS processing cost, 8000 = GPU processing cost
		NODE_BASE_COST				= 15000*2,			// ????
		PORTAL_BASE_COST			= 10000000,			// Very Large Number
		RENDER_TRIANGLE_COST		= 100,				// 100 cycles (measured on GEForce 2, 866Mhz P3)
		RENDER_PIXEL_COST			= 8,				// cycles per pixel
		FLASH_BUDGET				= 2500000,			// how many cycles can we use for flashing?

		WRITE_COST_PER_OBJECT		= 2500,				// base cost per object
		WRITE_COST_PER_BUCKET_EDGE	= 400,				// cost for each edge processed in a bucket
		WRITE_COST_PER_SCANLINE		= 65,				// cost per rasterized scanline
		WRITE_COST_PER_ZCHANGE		= 200				// cost per z-change processed
		
	};

	static float		calculateRenderCost			(int vnum,int tnum,float complexity);
};

DPVS_FORCE_INLINE void ImpObject::setStatus	(Status s, UINT32 mask)		
{ 
	UINT32 newMask = m_lastStatus;
	if (s == HIDDEN)
		newMask &=~mask;
	else
		newMask |= mask;

	if (m_lastStatus != newMask)  
	{  
		DPVS_PROFILE(Statistics::incStatistic(Library::STAT_DATABASEOBSTATUSCHANGES,1));  
		m_lastStatus = newMask;  
	}
}  

//------------------------------------------------------------------------
// Wrapper code
//------------------------------------------------------------------------

DPVS_FORCE_INLINE ImpObject* Object::getImplementation	(void) const				
{ 
	return reinterpret_cast<ImpObject*>(const_cast<Object*>(this)+1);
}

} //namespace DPVS

//------------------------------------------------------------------------
#endif //__DPVSIMPOBJECT_HPP
