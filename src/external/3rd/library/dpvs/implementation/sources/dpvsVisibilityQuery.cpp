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
 * Description: 	VisibilityQuery code
 *
 * $Archive: /dpvs/implementation/sources/dpvsVisibilityQuery.cpp $
 * $Author: wili $ 
 * $Revision: #6 $
 * $Modtime: 4/24/02 17:09 $
 * $Date: 2003/08/24 $
 * 
 ******************************************************************************/

#include "dpvsVisibilityQuery.hpp"
#include "dpvsOcclusionBuffer.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsWriteQueue.hpp"
#include "dpvsImpCommander.hpp"
#include "dpvsRecursionSolver.hpp"
#include "dpvsSweepAndPrune.hpp"
#include "dpvsDebug.hpp"
#include "dpvsDataPasser.hpp"
#include "dpvsImpPhysicalPortal.hpp"
#include "dpvsImpVirtualPortal.hpp"
#include "dpvsRandom.hpp"
#include "dpvsRemoteDebugger.hpp"

#if defined(DPVS_OUTPUT_PORTAL_TRAVERSAL)
#pragma warning(push,1)
#	include <iostream>
#pragma warning(pop)
#endif //DPVS_OUTPUT_PORTAL_TRAVERSAL

//------------------------------------------------------------------------

VisibilityQuery::PortalTimeStamp	VisibilityQuery::s_portalTimeStamp;
VisibilityQuery::FrameTimeStamp		VisibilityQuery::s_frameTimeStamp;

#if defined (DPVS_EVALUTION)
static unsigned int eval6 = DPVS::FLEXLM::getCode(DPVS_FLEXLM_CODE2);
#endif
/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::PortalTimeStamp::wrap()
 *
 * Description:		
 *
  *
 *****************************************************************************/

void VisibilityQuery::PortalTimeStamp::wrap()
{
	ImpObject::resetObjectTimeStamps();
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::FrameTimeStamp::wrap()
 *
 * Description:		
 *
 * Notes:			
 *
 *****************************************************************************/

void VisibilityQuery::FrameTimeStamp::wrap(void)
{
	// nada
}

using namespace DPVS;

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::VisibilityQuery()
 *
 * Description:		Common constructor
 *
 *****************************************************************************/

VisibilityQuery::VisibilityQuery() :
	m_cameraList(),
	m_portalList(),
	m_portalChain(),
	m_stencilModelList(),
	m_stencilModelCount(0),
	m_activeStencilCount(0),
	m_clipMaskList(),
	m_objectList(),
	m_ROIList(),
	m_stencilSilhouette(),
	m_recursionDepth(1),
	m_importanceThreshold(0.f),
	m_sweepAndPrune(NEW<SweepAndPrune>()),
	m_recursionSolver(NEW<RecursionSolver>()),
	m_currentCamera(null),
	m_currentCommander(null),
	m_oldFPUMask(0),
	m_properties(Camera::VIEWFRUSTUM_CULLING|Camera::OCCLUSION_CULLING),
	m_rasterWidth(0),
	m_rasterHeight(0),
	m_imageSpaceScalingX(1.f),
	m_imageSpaceScalingY(1.f),
	m_previousFrameCost(0.f),
	m_currentQueryObjectCount(0),
	m_previousQueryObjectCount(0),
	m_queriesPerSec(60.0f),
	m_FPSCounter(0),
	m_lastFPSTime(Debug::getServices()->getTime()),
	m_occlusionWriteQueue(null),
	m_occlusionBuffer(null),
	m_minimumCoverage(0.f,0.f,1.f),
	m_contributionCulling(false),
	m_screenSizeCulling(false),
	m_valid(false),
	m_preparedForRESEND(false),
	m_globalFlash(false)
{
	init();
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::~VisibilityQuery()
 *
 * Description:		
 *
 *****************************************************************************/

VisibilityQuery::~VisibilityQuery()	
{
	clean();

	DELETE (m_occlusionWriteQueue);
	DELETE (m_occlusionBuffer);
	DELETE (m_recursionSolver);
	DELETE (m_sweepAndPrune);		
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::init()
 *
 * Description:		VisibilityQuery initialization routine
 *
 *****************************************************************************/

void VisibilityQuery::init (void)
{
	m_clipMaskList.reset();
	m_objectList.reset();
	m_ROIList.reset();
	m_cameraList.reset();
	m_portalList.reset();
	m_recursionSolver->clear();
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::clean()
 *
 * Description:		
 *
 *****************************************************************************/

void VisibilityQuery::clean (void)
{
	if(m_cameraList.getCount() > 1)
	{
		m_cameraList.setCurrentIndex(1);

		while(!m_cameraList.isLast())	// note: camera 0 is user's original camera so don't destroy it
		{
			DELETE(m_cameraList.getCurrent());
			m_cameraList.traverse();
		}
	}

	m_cameraList.reset();
	m_portalList.reset();
}


/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::ROIvsObject()
 *
 * Description:		Perform ROI vs. Object testing
 *
 * Parameters:		firstObject = index to first object
 *					lastObject  = index to one past last object (i.e. exclusive value)
 *					firstROI	= index to first ROI
 *					lastROI		= index to one past last ROI (i.e. exclusive value)
 *
 *****************************************************************************/

// only used in the same .cpp file
DPVS_FORCE_INLINE void VisibilityQuery::ROIvsObject(const ImpObject* const * objectList,const ImpObject* const * ROIList,int objectCount,int ROICount,const Vector3& primarySweepDirection) const
{
	m_sweepAndPrune->process (DataPasser::getCommander(), objectList, ROIList, objectCount, ROICount, primarySweepDirection);
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::informUser()
 *
 * Description:		Informs user about results of the visibility query
 *
 *****************************************************************************/

void VisibilityQuery::informUser (void) const
{
	ImpCommander* ic = DataPasser::getImpCommander();

	DPVS_ASSERT (!ic->getViewer());							// WHAT?

	m_cameraList.setCurrentIndex(0);
	m_portalList.setCurrentIndex(0);

	//--------------------------------------------------------------------
	// Initialize portal chain
	//--------------------------------------------------------------------

	int	portalChainPosition = 0;
	CameraPortalTestValue& p = m_portalChain.getElement(portalChainPosition);
	p.c = m_cameraList.getCurrent();
	p.p = m_portalList.getCurrent();
	p.testValue = -1;							// CANNOT occur
	portalChainPosition++;

	//--------------------------------------------------------------------
	// Set initial view port
	//--------------------------------------------------------------------

	ImpCamera* currentCamera = m_cameraList.getCurrent();			// current camera (user's camera)

	if(!currentCamera)
		return;

#if defined DPVS_USE_REMOTE_DEBUGGER
	RemoteDebuggerWrapper::setScreenSize(currentCamera->getRasterWidth(),currentCamera->getRasterHeight());
#endif // DPVS_USE_REMOTE_DEBUGGER
	ic->setViewer(&currentCamera->getViewer());						// set correct viewer

	ic->command(Commander::VIEW_PARAMETERS_CHANGED);				// report new viewport

	//--------------------------------------------------------------------
	// Keep looping...
	//--------------------------------------------------------------------

	int		stencilTestValue  = -1;									// disable test
	int		stencilWriteValue = -1;									// disable write
	int		cellIndex = 0;
	float	objectImportance = 1.0f;

	float	framePrice = 0.f;				// DEBUG DEBUG

	while(m_objectList.hasContainer(cellIndex))
	{
		//--------------------------------------------------------------------
		// Find lists of objects and ROIs in current cell
		//--------------------------------------------------------------------

		ImpObject* const * objectList;
		ImpObject* const * ROIList;
		
		const int objectCount = m_objectList.getContainer(cellIndex,objectList);
		const int ROICount	  = m_ROIList.getContainer(cellIndex,ROIList);

		//--------------------------------------------------------------------
		// Assign clip masks to objects
		//--------------------------------------------------------------------

		const UINT32*	clipMaskList;
		m_clipMaskList.getContainer(cellIndex,clipMaskList);

		for(int ci=0;ci<objectCount;ci++)
		{
			framePrice += objectList[ci]->getRenderCost();	// DEBUG DEBUG
			objectList[ci]->setClipMask( clipMaskList[ci] );
		}

		//--------------------------------------------------------------------
		// Perform ROI vs. object sorting + matching
		//--------------------------------------------------------------------

		DataPasser::setObjectImportance(objectImportance);			// set importance for LOD level selection
		DataPasser::setViewerCamera(currentCamera);					// to track user's matrix queries

		ROIvsObject(objectList,ROIList, objectCount,ROICount,currentCamera->getCameraToCell().getDof());		// resolve ROI vs Object and report to user

		//---------------------------------------------
		// Report view change to user
		//---------------------------------------------

		m_cameraList.traverse();									// get next camera
		if(m_cameraList.isLast())
			break;

		//-------------------------------------------------------
		// get previous & current portal caps
		//
		// levelsTraversed=1: new levels in depth, stencils OK
		// levelsTraversed=0: same level, removed 1 and added 1 new
		// levelsTraversed<0: backwards N levels and added 1 new
		//-------------------------------------------------------

		const int levelsTraversed = m_portalList.traverse();		// get next portal

		bool currentStencil,  currentFloating;
		objectImportance = calculateImportance();					// update cumulative importance
		getCurrentPortalCaps(currentStencil,currentFloating);

		//-------------------------------------------------------
		// Send PORTAL_ENTER and PORTAL_EXIT callbacks
		//-------------------------------------------------------

		// levelsTraversed=1: new levels in depth, stencils OK
		// levelsTraversed=0: same level, removed 1 and added 1 new
		// levelsTraversed<0: backwards N levels and added 1 new

		if(levelsTraversed<=0)
		{
			int cnt = (-levelsTraversed) + 1;	// # of portals exitted

			for(int i=0;i<cnt;i++)				// EXIT portals in reverse order
			{
				const int index = portalChainPosition-1-i;
				reportPortalExit(ic,index);
			}
			portalChainPosition -= cnt;
		}

		CameraPortalTestValue& p = m_portalChain.getElement(portalChainPosition); 

		p.c = m_cameraList.getCurrent();
		p.p = m_portalList.getCurrent();
		p.testValue = stencilTestValue;
		portalChainPosition++;

		if(m_portalChain[portalChainPosition-1].p->informPortalEnter())
		{
			int index = portalChainPosition-1;

			reportViewport(m_portalChain[index-1].c);					// PARENT camera
			ic->setInstanceObject	(m_portalChain[index].p);			// current portal
			ic->command				(Commander::PORTAL_ENTER);
			ic->setInstanceObject	(null);								// current portal
		}

		//-------------------------------------------------------
		// update stencil mask hierarchy
		//-------------------------------------------------------

		currentCamera = m_cameraList.getCurrent();
		DPVS_ASSERT(currentCamera);


		if(m_activeStencilCount>0)
		{
			//-----------------------------------------------
			// Stencil decrement operations
			//-----------------------------------------------

			const int decCount = (-levelsTraversed) + 1;

			for(int id=0;id<decCount;id++)
			{
				int index = (m_stencilModelCount-1) - id;							// NOTE: reverse order

				if(m_stencilModelList[index].m)
				{
					//-----------------------------------------------
					// Set stencil values
					//-----------------------------------------------

					--stencilTestValue;
					--stencilWriteValue;

					ic->setStencilValues(stencilWriteValue,stencilTestValue);		// decrement

					//-----------------------------------------------
					// Set stencil model (knows the object->camera transformation)
					//-----------------------------------------------

					ImpCamera* dc = m_stencilModelList[index-1].c;					// PARENT camera
					dc->allowVirtualPlane(false);
					reportViewport(dc);							
					dc->allowVirtualPlane(true);

					reportStencilMask(m_stencilModelList[index].p);					// CURRENT portal
				}
			}

			ic->setStencilValues(stencilTestValue,-1);								// new test, disable write

		} //if(m_activeStencilCount>0)


		updateActiveStencilList(m_activeStencilCount);


		if(m_activeStencilCount==0)													// disable stencil buffer
		{
			ic->setStencilValues(-1,-1);											// disable test, disable write
			stencilTestValue	= 0;
			stencilWriteValue	= 1;
		}
		else
		{
			//-----------------------------------------------
			// Stencil increment
			//-----------------------------------------------

			if(currentStencil)
			{
				//-----------------------------------------------
				// the firtst stencil
				//-----------------------------------------------

				if(m_activeStencilCount==1)
				{
					ic->setStencilValues(-1,1);
					stencilTestValue	= 0;
					stencilWriteValue	= 1;
				}

				//-----------------------------------------------
				// increment stencil mask
				//-----------------------------------------------

				const int index = m_stencilModelCount-1;

				ic->setStencilValues(stencilTestValue,stencilWriteValue);			// increment

				m_stencilModelList[index-1].c->allowVirtualPlane(false);
				reportViewport(m_stencilModelList[index-1].c);						// PARENT camera
				m_stencilModelList[index-1].c->allowVirtualPlane(true);

				reportStencilMask(m_stencilModelList[index].p);						// CURRENT portal

				stencilTestValue++;
				stencilWriteValue++;
				ic->setStencilValues(stencilTestValue,-1);							// new test, disable write
			}

		} // if(m_activeStencilCount==0) - else													// disable stencil buffer

		reportViewport(currentCamera);		// report current camera in any case

		p.testValue = stencilTestValue;		// MAY have been changed
		cellIndex++;
	}


	//-------------------------------------------------------------------------
	// Some PORTAL_EXIT commands may not have been sent
	//-------------------------------------------------------------------------

	if(portalChainPosition>=2)
	{
		// levelsTraversed<0: backwards N levels
		const int levelsTraversed = -m_portalList.traverse();

		for(int i=0;i<levelsTraversed;i++)			// EXIT portals in reverse order
		{
			const int index = portalChainPosition-i-1;
			reportPortalExit(ic,index);
		}
	}

	//---------------------------------------------
	// DEBUG DEBUG output
	//---------------------------------------------

	m_previousFrameCost = framePrice;

	//---------------------------------------------
	// OK, we're done
	//---------------------------------------------

	ic->setViewer(null);
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::reportPortalExit()
 *
 * Description:		Send "portal exit" -related callbacks
 *
 *****************************************************************************/

void VisibilityQuery::reportPortalExit		(ImpCommander* ic,int index) const
{
	// Avoid unnecessary callbacks
	if( (!m_portalChain[index].p->informPortalPreExit()) &&
		(!m_portalChain[index].p->informPortalExit()))
		return;

	// PORTAL_PRE_EXIT
	if(m_portalChain[index].p->informPortalPreExit())			// send pre exit
	{
		ic->setInstanceObject	(m_portalChain[index].p);		// current portal (ONLY for knowing _which_ portal)
		ic->command				(Commander::PORTAL_PRE_EXIT);
		ic->setInstanceObject	(null);
	}

	// VPC
	reportViewport(m_portalChain[index-1].c);					// PARENT camera

	// PORTAL_EXIT
	if(m_portalChain[index].p->informPortalExit())
	{
		ic->setInstanceObject	(m_portalChain[index].p);		// current portal
		ic->setStencilValues	(m_portalChain[index].testValue,-1);
		ic->command				(Commander::PORTAL_EXIT);
		ic->setInstanceObject	(null);
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::reportStencilMask()
 *
 * Description:		
 *
 *****************************************************************************/

void VisibilityQuery::reportStencilMask	(ImpPhysicalPortal*p) const
{
	ImpCommander* ic = DataPasser::getImpCommander();
	ic->setInstanceObject	(p);
	ic->command				(Commander::STENCIL_MASK);
	ic->setInstanceObject	(null);							// SAFETY
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::reportViewport()
 *
 * Description:		
 *
 *****************************************************************************/

void VisibilityQuery::reportViewport (ImpCamera* c) const
{
	//-----------------------------------------------------
	// to track user's matrix queries
	//-----------------------------------------------------
	DataPasser::setViewerCamera(c);

#if defined DPVS_USE_REMOTE_DEBUGGER
	RemoteDebuggerWrapper::setScreenSize(c->getRasterWidth(),c->getRasterHeight());
#endif // DPVS_USE_REMOTE_DEBUGGER

	//-----------------------------------------------------
	// report viewport
	//-----------------------------------------------------

	ImpCommander* ic = DataPasser::getImpCommander();

	ic->setViewer(&c->getViewer());
	ic->command(Commander::VIEW_PARAMETERS_CHANGED);

	if(!(getProperties() & Camera::RESEND))
	if(Debug::getLineDrawFlags() & Library::LINE_PORTAL_RECTANGLES)
	{
		const FloatRectangle&	r		= c->getRasterViewport();
		Library::LineType		type	= Library::LINE_PORTAL_RECTANGLES;

		Debug::drawLine(type, Vector2(r.x0, r.y0), Vector2(r.x1, r.y0), Vector4(0.6f, 0.6f, 0.6f, 0.4f));
		Debug::drawLine(type, Vector2(r.x1, r.y0), Vector2(r.x1, r.y1), Vector4(0.6f, 0.6f, 0.6f, 0.4f));
		Debug::drawLine(type, Vector2(r.x1, r.y1), Vector2(r.x0, r.y1), Vector4(0.6f, 0.6f, 0.6f, 0.4f));
		Debug::drawLine(type, Vector2(r.x0, r.y1), Vector2(r.x0, r.y0), Vector4(0.6f, 0.6f, 0.6f, 0.4f));
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::updateActiveStencilList()
 *
 * Description:		
 *
 * Notes:			This function traverses the portal chain in backwards order.
 *
 *****************************************************************************/

void VisibilityQuery::updateActiveStencilList(int& activeCount) const
{
	const int current = m_portalList.getCurrentIndex();
	int	pos = current;
	m_stencilModelCount	= 0;
	activeCount			= 0;

	DPVS_ASSERT(current < m_portalList.getAppendIndex());

	//-----------------------------------------------------
	// count entries starting from current entry all the
	// way to the root node (user's camera)
	//-----------------------------------------------------

	for(pos=current;pos>=0;)
	{
		m_portalList.setCurrentIndex(pos);
		m_stencilModelCount++;
		pos=m_portalList.getParentIndex();
	}

	//-----------------------------------------------------
	// build list in forward order
	//-----------------------------------------------------

	int	i;
	for(pos=current,i=m_stencilModelCount-1;i>=0;i--)
	{
		m_portalList.setCurrentIndex(pos);
		m_cameraList.setCurrentIndex(pos);

		ImpPhysicalPortal *p = m_portalList.getCurrent();
		ImpCamera *c		 = m_cameraList.getCurrent();

		DPVS_ASSERT(p && c);

		Model* sm = pos>0 ? p->getStencilModel() : null;		// the first portal is INVALID pointer

		CameraPortalStencil& s = m_stencilModelList.getElement(i);	// DEBUG DEBUG here we _know_ the entry exists so we wouldn't need the resize code
		s.c = c;
		s.p = p;
		s.m = sm ? sm->getImplementation() : null;

		if(sm)
			activeCount++;

		pos = m_portalList.getParentIndex();
	}

	m_portalList.setCurrentIndex(current);
	m_cameraList.setCurrentIndex(current);
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::calculateImportance()
 *
 * Description:		Calculates cumulative object importance value
 *
 * Notes:			This function traverses the portal chain in backwards order.
 *					All changes to *this are temporary
 *
 *****************************************************************************/

float VisibilityQuery::calculateImportance() const
{
	const int current = m_portalList.getCurrentIndex();
	DPVS_ASSERT(current < m_portalList.getAppendIndex());

	float importance = 1.f;
	int	pos = current;									// current is valid

	while(pos>0)										// position zero is invalid
	{
		m_portalList.setCurrentIndex(pos);
		ImpPhysicalPortal *p = m_portalList.getCurrent();
		if(p!=getInvalidPointer())
			importance *= p->getImportanceDecay();		// multiply importance together
		pos = m_portalList.getParentIndex();
	}

	m_portalList.setCurrentIndex(current);
	return importance;
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::getCurrentPortalCaps()
 *
 * Description:		Calculates cumulative object importance value
 *
 * Notes:			This function traverses the portal chain in backwards order.
 *
 *****************************************************************************/

void VisibilityQuery::getCurrentPortalCaps(bool &stencilMask,bool &floating) const
{
	stencilMask = false;
	floating	= false;

	const int index = m_portalList.getCurrentIndex();
	if(index<=0)
		return;

	ImpPhysicalPortal *p = m_portalList.getCurrent();
	if(p==getInvalidPointer())
		return;

	if(p->getStencilModel())
		stencilMask = true;

	if(p->isFloatingPortal())
		floating = true;
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::updateQueriesPerSec()
 *
 * Description:		Update queries/sec counter so that the helper
 *					can keep track of approximate FPS value
 *
 *****************************************************************************/

void VisibilityQuery::updateQueriesPerSec	(void)
{
	m_FPSCounter++;

	//--------------------------------------------------------------------
	// update real FPS counter every 0.5 seconds
	//--------------------------------------------------------------------
	
	const float t = Debug::getServices()->getTime();
	if ((t-m_lastFPSTime)>0.5f)						
	{
		m_queriesPerSec = (t-m_lastFPSTime) / m_FPSCounter; // queries/time elapsed
		m_FPSCounter	= 0;
		m_lastFPSTime	= t;
	}
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::setParameters()
 *
 * Description:		Defines camera viewing parameters
 *
 * Parameters:		screenWidth
 *					screenHeight
 *					properties
 *					imageSpaceScalingX
 *					imageSpaceScalingY
 *
 * Returns:			true if raster space dimensions remained the same
 *
 *****************************************************************************/

bool VisibilityQuery::setParameters (int screenWidth,int screenHeight,UINT32 properties,float imageSpaceScalingX,float imageSpaceScalingY)
{

	// evaluation copy check
#if defined (DPVS_EVALUATION)
	if ((eval6^1122075)/7 != (DPVS_FLEXLM_ANSWER2^1122075)/7)
	{
		properties &=~ Camera::OCCLUSION_CULLING;
		properties &=~ Camera::VIEWFRUSTUM_CULLING;
	}
#endif

	//--------------------------------------------------------------------
	// Object minimum coverage...
	//--------------------------------------------------------------------

	setObjectMinimumCoverage(m_minimumCoverage[0],m_minimumCoverage[1],m_minimumCoverage[2]);

	//--------------------------------------------------------------------
	// If occlusion culling is enabled, automatically turn on view
	// frustum culling as well.
	//--------------------------------------------------------------------

	if (properties & Camera::OCCLUSION_CULLING)
		properties |= Camera::VIEWFRUSTUM_CULLING;

/*/----------------------------------------------------------------
	// Perform a CRC check here (see dpvsCopyProtection.hpp for
	// details). If CRC check fails, disable occlusion and
	// view frustum culling.
	//----------------------------------------------------------------

	if (!compareCRC())	
	{
		properties &= ~Camera::OCCLUSION_CULLING;
		properties &= ~Camera::VIEWFRUSTUM_CULLING;
	}
*/
	//--------------------------------------------------------------------
	// If someone is dummy enough to resubmit these parameters every
	// frame, let's not punish him (cause we'll take the blame from
	// the performance hit anyway...) =)
	//--------------------------------------------------------------------

	const bool sameRaster		= (m_rasterWidth==screenWidth && m_rasterHeight==screenHeight);
	const bool sameImageSpace = (sameRaster && 
						  m_imageSpaceScalingX == imageSpaceScalingX &&
						  m_imageSpaceScalingY == imageSpaceScalingY);

	if(sameImageSpace && m_properties==properties)	// nothing changed
		return sameRaster;

	const bool sameOcclusionBuffer = sameImageSpace && !((m_properties^properties)&Camera::OCCLUSION_CULLING);

	//--------------------------------------------------------------------
	// Copy new values
	//--------------------------------------------------------------------

	m_rasterWidth			= screenWidth;
	m_rasterHeight			= screenHeight;
	m_properties			= properties;
	m_imageSpaceScalingX	= imageSpaceScalingX;
	m_imageSpaceScalingY	= imageSpaceScalingY;

	if(!sameOcclusionBuffer)
	{
		//--------------------------------------------------------------------
		// Free old write queue and occlusion buffer
		//--------------------------------------------------------------------

		DELETE(m_occlusionWriteQueue);
		DELETE(m_occlusionBuffer);
		m_occlusionWriteQueue	= null;		// SAFETY
		m_occlusionBuffer		= null;		// SAFETY

		//--------------------------------------------------------------------
		// If occlusion culling is enabled, create write queue and
		// occlusion buffer
		//--------------------------------------------------------------------

		if((m_properties&Camera::OCCLUSION_CULLING) && screenWidth>0 && screenHeight>0)
		{
			m_occlusionBuffer		= new (MALLOC(sizeof(OcclusionBuffer))) OcclusionBuffer (m_rasterWidth,m_rasterHeight,m_imageSpaceScalingX,m_imageSpaceScalingY);
			m_occlusionWriteQueue	= new (MALLOC(sizeof(WriteQueue)))      WriteQueue		 (m_occlusionBuffer);
		}
	}

	m_valid = true;
	return sameRaster;
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::setObjectMinimumCoverage()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void VisibilityQuery::setObjectMinimumCoverage (float w,float h,float opacity)				
{ 
	m_contributionCulling	= false;
	m_screenSizeCulling		= false;

	m_minimumCoverage.make(w,h,opacity);

	if(m_occlusionBuffer)
		m_occlusionBuffer->setCoverageThreshold(opacity);

	//--------------------------------------------------------------------
	// Disable contribution culling if evaluation copy check fails
	//--------------------------------------------------------------------

#if defined (DPVS_EVALUATION)
	if ((eval6-3389813)*233939 != (DPVS_FLEXLM_ANSWER2-3389813)*233939)
#endif
	{
		m_screenSizeCulling   = ((w!=0.0f) && (h!=0.0f));
		m_contributionCulling = opacity < 1.f;
	}
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::setStaticCoverageMask()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void VisibilityQuery::setStaticCoverageMask	(const unsigned char* buffer,int w, int h,int pitch)
{
	if(m_occlusionBuffer)
		m_occlusionBuffer->setStaticCoverageBuffer(buffer,w,h,pitch);
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::setStaticZBuffer()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void VisibilityQuery::setStaticZBuffer		(const float* buffer,int w, int h,int pitch,float farValue)
{
	if(m_occlusionBuffer)
		m_occlusionBuffer->setStaticZBuffer(buffer,w,h,pitch,farValue);
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::grabZBuffer()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

float* VisibilityQuery::grabZBuffer			(int&w,int&h)
{
	if(m_occlusionBuffer)
		return m_occlusionBuffer->grabZBuffer(w,h);
	return null;
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::displayOcclusionBuffer()
 *
 * Description:		Copy contents of the coverage/z-buffer to user surface
 *
 * Notes:			Calls Commander::command, from where grabBuffer() is probably called.
 *					Passes the information of which buffer to grab in icom->m_bufferType.
 *
 *****************************************************************************/

void VisibilityQuery::displayOcclusionBuffer(void) const
{
	//--------------------------------------------------------------------
	// no buffers available if occlusion culling is disabled
	//--------------------------------------------------------------------

	if(!(getProperties()&Camera::OCCLUSION_CULLING) || !m_occlusionBuffer)
		return;
	
	//--------------------------------------------------------------------
	// report buffers
	//--------------------------------------------------------------------

	const int bcnt = 4;
	Library::BufferType	btype[4] = {Library::BUFFER_COVERAGE,
									Library::BUFFER_DEPTH,
									Library::BUFFER_FULLBLOCKS,
									Library::BUFFER_STENCIL};

	const UINT32 bflags	= Debug::getBufferFlags();
	ImpCommander* ic	= DataPasser::getImpCommander();

	ic->setOcclusionBuffer(m_occlusionBuffer);			// for USER communication

	for(int i=0;i<bcnt;i++)								// all buffer types
	{
		if(!(bflags & btype[i]))
			continue;

		ic->setBufferType(btype[i]);
		ic->command(Commander::DRAW_BUFFER);			// report that there is a buffer to be drawn
	}

	ic->setOcclusionBuffer(null);						// SAFETY
}


/*****************************************************************************
 *
 * Function:		VisibilityQuery::minimizeMemoryUsage()
 *
 * Description:		Minimizes memory footprint of the camera
 *
 * Parameters:
 *
 *****************************************************************************/

void VisibilityQuery::minimizeMemoryUsage	(void)
{
	if(m_occlusionBuffer)
		m_occlusionBuffer->minimizeMemoryUsage();

	// TODO: are there other subsystems?
}

/*****************************************************************************
 *
 * Function:		VisibilityQuery::setVisible(Object*,UINT32)
 *
 * Description:		Marks objects as "visible"
 *
 * Parameters:		clipMask	= 32-bit clip mask
 * 
 *****************************************************************************/

void VisibilityQuery::setVisible	(UINT32 clipMask)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpObject* o = VQData::get().getObject();
	DPVS_ASSERT(o);

	//--------------------------------------------------------------------
	// Update object's "last visible" time DEBUG DEBUG CAN WE GET RID OF
	// THIS?
	//--------------------------------------------------------------------

	o->setVisibleTime();				

	//--------------------------------------------------------------------
	// Perform object-type-specific update. We cannot use virtual
	// functions here easily (that's why we have an ugly switch-case).
	//--------------------------------------------------------------------

	switch(o->getType())
	{
		case ImpObject::TYPE_OBJECT:

			if (o->informVisible())
			{
				// DEBUG DEBUG combine these two lists into a struct
				m_objectList.append(o);							
				m_clipMaskList.append(clipMask);
			}

			m_currentQueryObjectCount++;

			if(!(m_properties & Camera::OCCLUSION_CULLING))
				return;

			if((m_currentQueryObjectCount < 20) ||
				(m_currentQueryObjectCount < (3*m_previousQueryObjectCount)>>1))	// less than (3/2 == 1.5) increase?
				return;

			if(m_globalFlash)									// already a global flash frame
				return;

			if(m_cameraList.getCurrentIndex() > 0)				// portals - the following code doesn't handle stencils -> exit
				return;

			// DEBUG DEBUG
//			Debug::print("Yo! %d\n",m_previousQueryObjectCount);
			applyOccluderSelectionPatch();						// d'oh -- we must apply the occluder selection patch!

		break;

		case ImpObject::TYPE_REGION_OF_INFLUENCE:
			m_ROIList.append(o);
			break;

		case ImpObject::TYPE_PHYSICAL_PORTAL:					// fall thru!
		case ImpObject::TYPE_VIRTUAL_PORTAL:
			setPortalVisible(clipMask);
			break;

		default:
			DPVS_ASSERT(!"Unknown object type");				// unknown object type
			break;
	}
}


//------------------------------------------------------------------------
