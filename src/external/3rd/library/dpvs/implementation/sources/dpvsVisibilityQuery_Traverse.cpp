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
 * Description: 	VisibilityQuery's traversal functions
 *
 * $Archive: /dpvs/implementation/sources/dpvsVisibilityQuery_Traverse.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 8.01.02 13:32 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsVisibilityQuery.hpp"
#include "dpvsOcclusionBuffer.hpp"
#include "dpvsWriteQueue.hpp"
#include "dpvsImpCamera.hpp"
#include "dpvsImpCell.hpp"
#include "dpvsRecursionSolver.hpp"
#include "dpvsDebug.hpp"
#include "dpvsImpCommander.hpp"
#include "dpvsDatabase.hpp"
#include "dpvsImpPhysicalPortal.hpp"
#include "dpvsImpVirtualPortal.hpp"

#if defined(DPVS_OUTPUT_PORTAL_TRAVERSAL)
#pragma warning(push,1)
#	include <iostream>
#pragma warning(pop)
#endif //DPVS_OUTPUT_PORTAL_TRAVERSAL


/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::launchTravelers()
 *
 * Description:		
 *
 *****************************************************************************/

void VisibilityQuery::launchTravelers (float importanceThreshold)
{
	DPVS_OUTPUT( std::cout << std::endl );
//	Debug::print("%d\n",sizeof(VQData));

	VQData::get().newVisibilityQuery(this);

	//------------------------------------------------------------------------
	// Set initial data for traversal
	// NOTE: CAME to root node thru "invalid" portal, which was REAL
	//------------------------------------------------------------------------

	ImpPhysicalPortal* invalidPortal = (ImpPhysicalPortal*)const_cast<void*>(getInvalidPointer());
	addCameraAndPortal	(m_currentCamera,invalidPortal,0);
	addTransition		(m_currentCamera->getCell()->getImplementation(),invalidPortal,invalidPortal,RecursionSolver::REAL_TRANSITION);
	terminateLevel		();

	//------------------------------------------------------------------------
	//------------------------------------------------------------------------

	m_importanceThreshold = importanceThreshold;
	bool occlusionCulling = (getProperties() & Camera::OCCLUSION_CULLING)!=0;

	CameraPortalStencil& stencilModel0 = m_stencilModelList.getElement(0);
	stencilModel0.c = m_cameraList.getCurrent();
	stencilModel0.p = null;
	stencilModel0.m = null;
	m_activeStencilCount= 0;
	m_stencilModelCount = -1;

	float objectImportance = 1.0f;

	while(!m_cameraList.isLast())
	{
		ImpCamera *c		 = m_cameraList.getCurrent();
		ImpPhysicalPortal* p = m_portalList.getCurrent();

		m_currentCamera = c;					// camera used in elsewhere

		//----------------------------------------------------------------
		// Start new containers for current cell
		//----------------------------------------------------------------

		m_clipMaskList.newContainer();
		m_objectList.newContainer();
		m_ROIList.newContainer();

		//----------------------------------------------------------------
		// Draw camera axii
		//----------------------------------------------------------------

		if(Debug::getLineDrawFlags() & Library::LINE_VIRTUAL_CAMERA_AXII)
		{
			Library::LineType type = Library::LINE_VIRTUAL_CAMERA_AXII;

			Matrix4x3 m = c->getCameraToCell();
			m.productFromLeft(c->getCell()->getImplementation()->getCellToWorld());

			const float scale = 5.f;
			Vector3 loc = m.getTranslation();
			Vector3 xo = m.getRight() * scale;
			Vector3 yo = m.getUp() * scale;
			Vector3 zo = m.getDof() * scale;
			Debug::drawLine(type, loc, loc+xo, Vector4(1.0f,0.0f,0.0f,1.f));
			Debug::drawLine(type, loc, loc+yo, Vector4(0.0f,1.0f,0.0f,1.f));
			Debug::drawLine(type, loc, loc+zo, Vector4(0.0f,0.0f,1.0f,1.f));
		}

		//----------------------------------------------------------------
		// Find out if there is need for additional clipping plane
		//----------------------------------------------------------------

		if(p!=invalidPortal && p->getType()==ImpObject::TYPE_VIRTUAL_PORTAL && p->isFloatingPortal())
		{
			// pleq in object-space (that's where the warp matrix is defined)
			ImpVirtualPortal* vp = (ImpVirtualPortal*)p;		//SAFE

			const Matrix4x3& warp = vp->getWarpMatrix();
			Vector3 normal	(warp.getDof());
			Vector3 pip		(warp.getTranslation());
			Vector4 pleq	(normal.x,normal.y,normal.z,-dot(normal, pip));

			// transform to cell-space	(DPVS internal)
			Vector4	tmp;
		
			Math::transformPlanes (&tmp, &pleq, p->getObjectToCell(), 1);
			VQData::get().setFrustumPlane(7,tmp);	// 0==zero, 1-6==VF, 7==virtualPlane

			Math::transformPlanes (&tmp, &pleq, p->getObjectToCameraMatrix(c), 1);
			c->setClipPlane(7,tmp);				// 0==zero, 1-6==VF, 7==virtualPlane
			c->setClipPlaneCount(8);
			VQData::get().enableProperties(VQData::PERFORM_VIEWFRUSTUM_CULLING);		// NOTE: Force VF culling of floating portals.
		}
		else
		{
			c->setClipPlaneCount(7);			// 0==zero, 1-6==VF
		}

		//----------------------------------------------------------------
		// Setup VQData::get()
		//----------------------------------------------------------------

		VQData::get().newCamera(c);					// validate visibility query data

		//----------------------------------------------------------------
		// Set importance and viewer camera to DataPasser
		//----------------------------------------------------------------

		DataPasser::setObjectImportance	(objectImportance);
		DataPasser::setViewerCamera		(c);					// to track user's matrix queries


		//----------------------------------------------------------------
		// Support Cell::REPORT_IMMEDIATELY (if requested)
		//----------------------------------------------------------------

		DPVS_ASSERT (c->getCell());
		DPVS_ASSERT (m_currentCommander);

		ImpCell* impCell = c->getCell()->getImplementation();

		if (impCell->reportImmediately())
		{
			ImpCommander*	impCommander	= m_currentCommander->getImplementation();
//			Debug::print ("CELL IMMEDIATE REPORT for %p\n",c->getCell());
			impCommander->setCell(c->getCell());						// current cell
			impCommander->command(Commander::CELL_IMMEDIATE_REPORT);	// user is allowed to modify the target database!
			impCommander->setCell(null);								// return to NULL
		}
		
		//----------------------------------------------------------------
		// Make sure the database has been updated properly (i.e. all
		// modified object bounds have been checked etc.. -- note that
		// the database may have _just_ become modified)
		//----------------------------------------------------------------

		ImpObject::updateAllDirtyBounds(*impCell);

		//----------------------------------------------------------------
		// Call the database's traversal routine
		//----------------------------------------------------------------

		IDatabase* database = impCell->getDatabase();

		DPVS_ASSERT(database);
		bool abortion = !database->traverse();					// calls addObject(), addROI(), addCamera(), addPortal()

		//----------------------------------------------------------------
		// If some object requested global abortion, signal this to the
		// commander and bail out immediately.
		//----------------------------------------------------------------

		if(abortion)
		{
			ImpObject* io = m_objectList.getPreviouslyAppended();
			DPVS_ASSERT(io);

			ImpCommander* ic = DataPasser::getImpCommander();
			ic->setInstanceObject(io);
			ic->command (Commander::QUERY_ABORT);
			ic->setInstanceObject(null);				//SAFETY

			clean();		// release cameras etc
			init();			// reset lists
			
			break;
		}

		//----------------------------------------------------------------
		// Terminate camera & portal lists created by current camera
		//----------------------------------------------------------------

		terminateLevel();

		//-------------------------------------------------------
		// Get previous portal caps
		//-------------------------------------------------------

		bool previousStencil;
		bool previousFloating;
		getCurrentPortalCaps(previousStencil,previousFloating);

		m_cameraList.traverse();
		if(m_cameraList.isLast())
			break;

//		Debug::print("New camera\n");

		int levelsTraversed = m_portalList.traverse();
		objectImportance = calculateImportance();					// update cumulative importance

		// levelsTraversed=1: new levels in depth, stencils OK
		// levelsTraversed=0: same level, removed 1 and added 1 new
		// levelsTraversed<0: backwards N levels and added 1 new

		if(occlusionCulling)
		{
			DPVS_OUTPUT( std::cout << "-----------------------\n" );
			DPVS_OUTPUT( std::cout << "Levels traversed: " << levelsTraversed << std::endl );

			//-------------------------------------------------------
			// get current portal caps
			//-------------------------------------------------------

			bool currentStencil;
			bool currentFloating;
			getCurrentPortalCaps(currentStencil,currentFloating);

			//-------------------------------------------------------
			// stencil masks have become invalid if
			//	- traversing backwards more than 1 level
			//	- traversing backwards 1 level and previous had a stencil mask (needs to be removed)
			//	- same level and both previous and current have a stencil mask
			//-------------------------------------------------------

			bool stencilsValid = true;

			if( (levelsTraversed < -1)					 ||
				(levelsTraversed==-1 && previousStencil) ||
				(levelsTraversed== 0 && currentStencil && previousStencil))
			{
				stencilsValid = false;
			}

			if(stencilsValid)	DPVS_OUTPUT( std::cout << "Stencils OK" << std::endl );
			else				DPVS_OUTPUT( std::cout << "Stencils INVALID" << std::endl );

			//-------------------------------------------------------
			// remove outdated entries from Write Queue
			//-------------------------------------------------------

			launch_removeFromWriteQueue	(levelsTraversed);

			//-------------------------------------------------------
			// update stencil mask hierarchy (INC-REPLACE policy)
			//-------------------------------------------------------

			launch_updateStencilMasks	(stencilsValid, currentStencil, currentFloating);
		}

		m_recursionSolver->traverse();
	}

	//--------------------------------------------------------------------
	// Switch off stencils
	//--------------------------------------------------------------------

	if(occlusionCulling)
		m_occlusionBuffer->initStencil(null);		//disable stencils

	VQData::get().invalidate();
}


/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::launch_removeFromWriteQueue()
 *
 * Description:		
 *
 *****************************************************************************/

void VisibilityQuery::launch_removeFromWriteQueue(int levelsTraversed)
{
	if(levelsTraversed>=1)
		return;

	//--------------------------------------------------------------------
	// Determine the number of cells where the entries in
	// the write queue are not needed anymore
	//--------------------------------------------------------------------

	int outdatedCellCount;
	
	if(levelsTraversed==0)	
		outdatedCellCount = 1;						// sister cell
	else					
		outdatedCellCount = (-levelsTraversed)+1;	// consecutive children

#ifndef DPVS_OUTPUT_PORTAL_TRAVERSAL

	for(int i=m_stencilModelCount-outdatedCellCount;i<m_stencilModelCount;i++)	// remove entries of # last cameras
	{
		m_occlusionWriteQueue->freeEntriesUsing( m_stencilModelList[i].c );
	}

#else

	DPVS_OUTPUT( std::cout << "Write Queue Removal (#cell): " << outdatedCellCount << std::endl );
	for(int i=m_stencilModelCount-outdatedCellCount;i<m_stencilModelCount;i++)	// remove entries of # last cameras
	{
		ImpCamera* sc = m_stencilModelList[i].c;
		const int cnt = m_occlusionWriteQueue->freeEntriesUsing(sc);
		std::cout << "Removed (" << sc << "): " << cnt << std::endl;
	}

#endif //DPVS_OUTPUT_PORTAL_TRAVERSAL
}


/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::launch_updateStencilMasks()
 *
 * Description:		
 *
 *****************************************************************************/

void VisibilityQuery::launch_updateStencilMasks	(bool stencilsValid, bool currentStencil, bool currentFloating)
{
	updateActiveStencilList(m_activeStencilCount);

	if(m_activeStencilCount==0)
	{
		m_occlusionBuffer->initStencil(null);			// disable stencil buffer
		DPVS_OUTPUT( std::cout << "Disable stencil\n" );
	}
	else
	{
		ImpCamera* sc			= null;
		ImpPhysicalPortal* sp	= null;

		Range<float> zrange;

		if(stencilsValid)
		{
			if(currentStencil)
			{
				const int index = m_stencilModelCount-1;

				//---------------------------------------------------------
				// Force flush for objects potentially affected by WRONG stencil stencil mask
				//---------------------------------------------------------

				sc = m_stencilModelList[index].c;						// the last (has stenil)
				m_occlusionWriteQueue->forceFlush(sc->getRasterViewport());

				//---------------------------------------------------------
				// Add a new stencil mask to stencil buffer
				//---------------------------------------------------------

				if(m_activeStencilCount==1)
				{
					DPVS_OUTPUT( std::cout << "Init stencil\n" );
					sc = m_stencilModelList[index].c;					// the first ==the last
					m_occlusionBuffer->initStencil(sc);					// init stencil buffer
				}

				sc = m_stencilModelList[index-1].c;						// PARENT camera transforms current portal(!!)
				sp = m_stencilModelList[index].p;						// CURRENT portal
				performStencilOP(sc,sp,zrange);
			}
			else
			{
				DPVS_OUTPUT( std::cout << "No Stencil\n" );
			}
		}
		else
		{
			//---------------------------------------------------------
			// Force flush for objects potentially affected by WRONG stencil stencil mask
			//---------------------------------------------------------

			m_occlusionWriteQueue->forceFlush(m_stencilModelList[m_stencilModelCount-1].c->getRasterViewport());

			//NOTE: this could lead to problems later?

			//---------------------------------------------------------
			// Init stencil buffer stencil buffer
			//---------------------------------------------------------

			sc = m_stencilModelList[m_stencilModelCount-1].c;			// We're not sure if the last has stencil or not
			m_occlusionBuffer->initStencil(sc);							// init stencil buffer

			//---------------------------------------------------------
			// Add all stencil masks to stencil buffer (INC-REPLACE)
			//---------------------------------------------------------

			for(int i=1;i<m_stencilModelCount;i++)
			{
				if(m_stencilModelList[i].m)
				{
					sc = m_stencilModelList[i-1].c;						// PARENT camera transforms current portal(!!)
					sp = m_stencilModelList[i].p;						// current portal
					performStencilOP(sc,sp,zrange);
				}
			}
		}

		//---------------------------------------------------
		// Clear depth buffer according to stencil mask
		//---------------------------------------------------

		if(currentFloating)
		{
			//---------------------------------------------------
			// Clears depth buffer from places the stencil is set
			//---------------------------------------------------

/*			m_occlusionBuffer->setZGradients	(ZGradient::TEST, 
												 m_stencilSilhouette.getPlaneVertices(), 
												 m_stencilSilhouette.getPlaneCount(),
												 zrange.getMin(),	
												 zrange.getMax());
*/
			m_occlusionBuffer->setZConstant(ZGradient::TEST,zrange.getMin());	// DEBUG DEBUG DEBUG
			m_occlusionBuffer->clearAccordingToStencil	();

			DPVS_OUTPUT( std::cout << "Clear Z-Buffer\n" );
		} //if(currentFloating)
	} // if(m_activeStencilCount==0) - else
}


/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::performStencilOP()
 *
 * Description:		
 *
 * Returns:			true is succesful
 *
 * Notes:			This function traverses the portal chain in backwards order.
 *
 *****************************************************************************/

bool VisibilityQuery::performStencilOP(ImpCamera* sc, ImpPhysicalPortal* sp,Range<float>& zrange)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	m_stencilSilhouette.clear();

	VQData::get().setProperties(VQData::FLOW_ACTIVE,false);

	if(sp->getStencilSilhouette(m_stencilSilhouette,zrange,sc))
	{
		DPVS_OUTPUT( std::cout << "Stencil OP\n" );
		m_occlusionBuffer->setScissor(sc);
		VQData::get().setProperties(VQData::FLOW_ACTIVE,true);
		return m_occlusionBuffer->stencilOp(m_stencilSilhouette);	// set stencil mask
	}
	else
	{
		DPVS_OUTPUT( Debug::print("Front clipping stencil OP\n"));
	}

	VQData::get().setProperties(VQData::FLOW_ACTIVE,true);
	return true;
}


/*****************************************************************************
 *
 * Function:		VisibilityQuery::traversePortal()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

bool VisibilityQuery::traversePortal(ImpCamera* targetCamera, Cell* userTargetCell, UINT32 clipMask)
{
	DPVS_ASSERT(VQData::get().testProperties(VQData::FLOW_ACTIVE));
	ImpCamera* c = VQData::get().getCamera();
	ImpPhysicalPortal *p = static_cast<ImpPhysicalPortal*>(VQData::get().getObject());
	DPVS_ASSERT(c == m_currentCamera);
	DPVS_ASSERT(p);

	ImpPhysicalPortal *s,*d;
	ImpCell* targetCell	= userTargetCell->getImplementation();
	UINT32 transitionFlags = 0;

	//---------------------------------------------------------------
	// transition flags for recursion solver
	//---------------------------------------------------------------

	if(p->getType() == ImpObject::TYPE_PHYSICAL_PORTAL)		transitionFlags |= RecursionSolver::REAL_TRANSITION;
	if(p->getStencilModel())								transitionFlags |= RecursionSolver::USES_STENCIL;

	//---------------------------------------------------------------
	// calculate matrix after the transition
	//---------------------------------------------------------------
	Matrix4x3 m(NO_CONSTRUCTOR);

	if(!p->calculateTransition(s,d, m,targetCell))
		return false;

	//---------------------------------------------------------------
	// test transition using recursion solver
	//---------------------------------------------------------------

	if(!testTransition(s,d,targetCell,transitionFlags))
		return false;

	//---------------------------------------------------------------
	// set data to target camera
	//---------------------------------------------------------------

	targetCamera->setCameraToCell(m);
	targetCamera->setCellNoReference(userTargetCell);

	//-----------------------------------------------------
	// calculate new scissor & frustum
	//-----------------------------------------------------

	FloatRectangle	portalRectangle;
	if(!p->getTestRectangle(portalRectangle))
		return false;

	Frustum frustum;
	if(!targetCamera->createFrustumFromRectangle(frustum,portalRectangle))		// create a new view frustum
		return false;

	targetCamera->setFrustumPlanesAndMatrix	(frustum);

	//-----------------------------------------------------
	// set misc data to targetCamera
	//-----------------------------------------------------

	targetCamera->setID						(c->getID() ^ p->getHashValue());	// give rather random ID to created camera
	targetCamera->setTimeStamp				(++s_portalTimeStamp);

	//-----------------------------------------------------
	// information for recursion solver
	//-----------------------------------------------------

	addCameraAndPortal		(targetCamera,p,clipMask);
	addTransition			(targetCamera->getCell()->getImplementation(),s,d,transitionFlags);

	DPVS_OUTPUT( std::cout << " Accept\n" );
	return true;
}


/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::addTransition()
 *
 * Description:		
 *
 * Parameters:		targetCell	= the cell we arrived into
 *					s			= source portal of the transition
 *					d			= target portal of the transition
 *					flags		= REAL/VIRTUAL, STENCIL etc
 *
 *****************************************************************************/

void VisibilityQuery::addTransition	(ImpCell* targetCell,ImpPhysicalPortal* s,ImpPhysicalPortal *d,UINT32 flags)
{ 
	m_recursionSolver->append(s,d,targetCell,flags); 
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::testTransition()
 *
 * Description:		
 *
 * Parameters:
 *
 * Returns:
 *
 *****************************************************************************/

bool VisibilityQuery::testTransition	 (ImpPhysicalPortal* s,ImpPhysicalPortal *d,ImpCell* targetCell,UINT32 flags)
{
	//-----------------------------------------------------
	// test if recursion solver accepts transition
	//-----------------------------------------------------

	if(!m_recursionSolver->testRecursionDepth(s,d))
	{
		//DEBUG DEBUG
//		std::cout << "VisibilityQuery terminated by recursion depth" << std::endl;
		return false;
	}

	if(!m_recursionSolver->testReTraversal(targetCell,flags))
	{
		//DEBUG DEBUG
//		std::cout << "VisibilityQuery terminated by re-traversal" << std::endl;
		return false;
	}

	//-----------------------------------------------------
	// test if importance exceeds threshold
	//-----------------------------------------------------

	if(m_importanceThreshold==0.0f)
		return true;

	float importance = s->getImportanceDecay() * calculateImportance();

	if(importance < m_importanceThreshold)
	{
//		std::cout << "VisibilityQuery terminated importance threshold" << std::endl;
		return false;
	}

	return true;
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::setRecursionDepth()
 *
 * Description:		
 *
 * Parameters:
 *
 *****************************************************************************/

void VisibilityQuery::setRecursionDepth	(int d)			
{ 
	m_recursionDepth = d; 
	m_recursionSolver->setMaximumDepth(d); 
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::addCameraAndPortal()
 *
 * Description:		
 *
 ****************************************************************************/

void VisibilityQuery::addCameraAndPortal(ImpCamera* c,ImpPhysicalPortal* p, UINT32 /*clipMask*/)
{ 
	m_cameraList.append(c);
	m_portalList.append(p);
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::terminateLevel()
 *
 * Description:		
 *
 ****************************************************************************/

void VisibilityQuery::terminateLevel(void)
{
	addCameraAndPortal(null,null,0);
	m_recursionSolver->appendNull();
}

/*****************************************************************************
 *
 * Function:		DPVS::VisibilityQuery::applyOccluderSelectionPatch()
 *
 * Description:		
 *
 ****************************************************************************/

void VisibilityQuery::applyOccluderSelectionPatch(void)
{
	//-------------------------------------------------------
	// push data
	//-------------------------------------------------------

	ImpObject* s_o = VQData::get().getObject();
	ImpCamera* s_c = VQData::get().getCamera();
	int	s_ci = m_cameraList.getCurrentIndex();

	//-------------------------------------------------------
	// disable occluder selection
	//-------------------------------------------------------

	m_globalFlash = true;
	ImpObject::forceSelectAll(true);

	//-------------------------------------------------------
	// clear write queue and occlusion buffer, traverse all cells
	//-------------------------------------------------------

	int cellIndex = 0;
	m_cameraList.setCurrentIndex(cellIndex);
	ImpCamera* c = m_cameraList.getCurrent();
	m_occlusionWriteQueue->clear(c);			// NOTE: this discards some potentially

	while(m_objectList.hasContainer(cellIndex))
	{
		c = m_cameraList.getCurrent();
		VQData::get().newCamera(c);					// fake flow

		//-------------------------------------------------------
		// process all objects in a cell
		//-------------------------------------------------------

		ImpObject* const * objectList;
		const int objectCount = m_objectList.getContainer(cellIndex,objectList);

		for(int i=0;i<objectCount;i++)
		{
			VQData::get().newObject(objectList[i]);			// fake flow
			m_occlusionWriteQueue->appendOccluder();	// uses flow
		}

		m_cameraList.traverse();
		cellIndex++;
	};

	//-------------------------------------------------------
	// pop data
	//-------------------------------------------------------

	m_cameraList.setCurrentIndex(s_ci);
	VQData::get().newCamera(s_c);
	VQData::get().newObject(s_o);
}

//------------------------------------------------------------------------
