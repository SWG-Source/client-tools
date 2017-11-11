// ======================================================================
//
// RenderWorld_Commander.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/RenderWorldCommander.h"

#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/RenderWorldCamera.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Production.h"
#include "sharedMath/Transform.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"

#include "dpvsCell.hpp"
#include "dpvsObject.hpp"

#define V3_TO_VECTOR(a)  (reinterpret_cast<Vector *>(&a)[0])

#include <typeinfo>
#include <map>


// ======================================================================

#define COMMANDER_USE_DPVS_TRANSFORMS 0

namespace RenderWorldCommanderNamespace
{
#ifdef _DEBUG
	int                       ms_numberOfVisibleObjects;
	int                       ms_numberOfPortalsCrossed;

	bool                      ms_displayTextMessages;
	bool                      ms_renderFrames;
	bool                      ms_renderSpheres;
#endif

	typedef std::set<NetworkId> NetworkIdSet;
	NetworkIdSet m_objectRenderedThisFrame;

#if COMMANDER_USE_DPVS_TRANSFORMS
	bool                      ms_useDpvsTransform;
#endif

	const RenderWorldCamera * ms_camera;
	CellProperty const *      ms_cameraCell;
	Transform                 ms_cellToWorldTransform;
	Transform                 ms_cameraToWorld;
	bool                      ms_useScissorRect;
	int                       ms_scissorRectOffsetX;
	int                       ms_scissorRectOffsetY;


#if PRODUCTION == 0
	ProfilerBlock             ms_reportingProfilerBlock("reporting");
	bool                      ms_profileInstanceImmediateReport;
	bool                      ms_profileInstanceVisible;
	bool                      ms_profileInstanceVisibleRenderCall;
	bool                      ms_profileRegionOfInfluenceActive;
	bool                      ms_profileRegionOfInfluenceInactive;
#endif
}
using namespace RenderWorldCommanderNamespace;

// ======================================================================

#ifdef _DEBUG

int RenderWorldCommander::getNumberOfVisibleObjects()
{
	return ms_numberOfVisibleObjects;
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

int RenderWorldCommander::getNumberOfPortalsCrossed()
{
	return ms_numberOfPortalsCrossed;
}

#endif

bool RenderWorldCommander::wasObjectRenderedThisFrame(NetworkId const & id)
{
	NetworkIdSet::const_iterator ii = m_objectRenderedThisFrame.find(id);
	return ii != m_objectRenderedThisFrame.end();
}

// ----------------------------------------------------------------------

void RenderWorldCommander::setCamera(const RenderWorldCamera * camera, CellProperty const * cameraCell, Transform const & cameraToWorld)
{
	ms_camera = camera;
	ms_cameraCell = cameraCell;
	ms_cameraToWorld = cameraToWorld;
}

// ======================================================================

RenderWorldCommander::RenderWorldCommander()
: DPVS::Commander()
{
#ifdef _DEBUG
	DebugFlags::registerFlag(ms_displayTextMessages,              "ClientGraphics/Dpvs", "displayTextMessages");
	DebugFlags::registerFlag(ms_renderFrames,                     "ClientGraphics/Dpvs", "renderFrames");
	DebugFlags::registerFlag(ms_renderSpheres,                    "ClientGraphics/Dpvs", "renderSpheres");
#endif

#if COMMANDER_USE_DPVS_TRANSFORMS
	DebugFlags::registerFlag(ms_useDpvsTransform,                 "ClientGraphics/Dpvs", "useDpvsTransform");
#endif

#if PRODUCTION == 0
	DebugFlags::registerFlag(ms_profileInstanceVisible,           "ClientGraphics/Dpvs", "profileInstanceVisible");
	DebugFlags::registerFlag(ms_profileInstanceImmediateReport,   "ClientGraphics/Dpvs", "profileInstanceImmediateReport");
	DebugFlags::registerFlag(ms_profileInstanceVisibleRenderCall, "ClientGraphics/Dpvs", "profileInstanceVisibleRenderCall");
	DebugFlags::registerFlag(ms_profileRegionOfInfluenceActive,   "ClientGraphics/Dpvs", "profileRegionOfInfluenceActive");
	DebugFlags::registerFlag(ms_profileRegionOfInfluenceInactive, "ClientGraphics/Dpvs", "profileRegionOfInfluenceInactive");
#endif


	ms_useScissorRect = Graphics::supportsScissorRect();
}

// ----------------------------------------------------------------------

void RenderWorldCommander::command(DPVS::Commander::Command c)
{
	switch (c)
	{
#ifdef _DEBUG
			////////////////////////////////////////////////////////////////////////////////////////////////////
		case DRAW_LINE_3D:
			{
				DPVS::Vector3 start, end;
				DPVS::Vector4 color;
				IGNORE_RETURN(getLine3D(start, end, color));
				const VectorArgb argb(color.v[0], color.v[1], color.v[2], color.v[3]);
				ms_camera->addDebugPrimitive(new Line3dDebugPrimitive(UtilityDebugPrimitive::S_none, Transform::identity, V3_TO_VECTOR(start), V3_TO_VECTOR(end), argb));
			}
			break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case DRAW_LINE_2D:
			{
				DPVS::Vector2 start, end;
				DPVS::Vector4 color;
				IGNORE_RETURN(getLine2D(start, end, color));

				const VectorArgb argb(color.v[0], color.v[1], color.v[2], color.v[3]);
				ms_camera->addDebugPrimitive(new Line2dDebugPrimitive(UtilityDebugPrimitive::S_none, start.v[0], start.v[1], end.v[0], end.v[1], argb));
			};
			break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case TEXT_MESSAGE:
			DEBUG_REPORT_LOG_PRINT(ms_displayTextMessages, ("DPVSTM: %s\n", getTextMessage()));
			break;

#endif

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case CELL_IMMEDIATE_REPORT:
		{
				NP_PROFILER_AUTO_BLOCK_DEFINE("RenderWorldCommander::cellImmediateReport");

				const CellProperty *cell = static_cast<const CellProperty *>(getCell()->getUserPointer());
				RenderWorld::cellVisible(cell);
				cell->callPreVisibilityTraversalRenderHookFunctions();
		} break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case QUERY_BEGIN:
		{

			// the begin for this profiler block is in RenderWorld::drawScene()
			NP_PROFILER_BLOCK_LEAVE(RenderWorld::getDpvsQueryProfilerBlock());

			// the end for this profiler block is in QUERY_END
			NP_PROFILER_BLOCK_ENTER(ms_reportingProfilerBlock);

			if (ms_useScissorRect)
			{
				ms_scissorRectOffsetX = RenderWorld::getViewportX0();
				ms_scissorRectOffsetY = RenderWorld::getViewportY0();
			}

#ifdef _DEBUG
			ms_numberOfVisibleObjects = 0;
			ms_numberOfPortalsCrossed = 0;
#endif

			m_objectRenderedThisFrame.clear();

			ShaderPrimitiveSorter::pushCell(*ms_cameraCell);
		} break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case QUERY_END:
		{
			ShaderPrimitiveSorter::popCell();

			if (ms_useScissorRect)
				Graphics::setScissorRect(false, 0, 0, 0, 0);

			// the begining of this profiler block is in QUERY_BEGIN
			NP_PROFILER_BLOCK_LEAVE(ms_reportingProfilerBlock);
		} break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case PORTAL_ENTER:
		{
#ifdef _DEBUG
				++ms_numberOfPortalsCrossed;
#endif

				DPVS::Object *o = getInstance()->getObject();
				DPVS::PhysicalPortal *p = static_cast<DPVS::PhysicalPortal *>(o);
				void *destinationUserPointer = p->getTargetCell()->getUserPointer();
				const CellProperty *destinationCellProperty = static_cast<const CellProperty *>(destinationUserPointer);
				ShaderPrimitiveSorter::pushCell(*destinationCellProperty);
		} break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case PORTAL_PRE_EXIT: 
		{
			ShaderPrimitiveSorter::popCell();
		} break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case VIEW_PARAMETERS_CHANGED:
		{
			if (ms_useScissorRect)
			{
				// request scissor rectangle from dPVS
				int left = 0;
				int right = 0;
				int top = 0;
				int bottom = 0;
				getViewer()->getScissor(left, top, right, bottom);

				// calculate our form of the scissor rect
				int const scissorRectX      = ms_scissorRectOffsetX + left;
				int const scissorRectY      = ms_scissorRectOffsetY + + top;
				int const scissorRectWidth  = right - left;
				int const scissorRectHeight = bottom - top;
				Graphics::setScissorRect(true, scissorRectX, scissorRectY, scissorRectWidth, scissorRectHeight);
			}
		}	break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case REGION_OF_INFLUENCE_ACTIVE:
			{
#if PRODUCTION == 0
				NP_PROFILER_BLOCK_DEFINE(profilerBlockRegionOfInfluenceActive, "region of influence active");
				if (ms_profileRegionOfInfluenceActive)
					NP_PROFILER_BLOCK_ENTER(profilerBlockRegionOfInfluenceActive);
#endif

				const Object *ourObject = static_cast<const Object *>(getInstance()->getObject()->getUserPointer());
				ourObject->setRegionOfInfluenceEnabled(true);

#if PRODUCTION == 0
				if (ms_profileRegionOfInfluenceActive)
					NP_PROFILER_BLOCK_LEAVE(profilerBlockRegionOfInfluenceActive);
#endif
			}
			break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case REGION_OF_INFLUENCE_INACTIVE:
			{
#if PRODUCTION == 0
				NP_PROFILER_BLOCK_DEFINE(profilerBlockRegionOfInfluenceInactive, "region of influence inactive");
				if (ms_profileRegionOfInfluenceInactive)
					NP_PROFILER_BLOCK_ENTER(profilerBlockRegionOfInfluenceInactive);
#endif

				const Object * ourObject = static_cast<const Object *>(getInstance()->getObject()->getUserPointer());
				ourObject->setRegionOfInfluenceEnabled(false);

#if PRODUCTION == 0
				if (ms_profileRegionOfInfluenceInactive)
					NP_PROFILER_BLOCK_LEAVE(profilerBlockRegionOfInfluenceInactive);
#endif
			}
			break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case INSTANCE_IMMEDIATE_REPORT:
			{
#if PRODUCTION == 0
				NP_PROFILER_BLOCK_DEFINE(profilerBlockInstanceImmediateReport, "immediate report");
				if (ms_profileInstanceImmediateReport)
					NP_PROFILER_BLOCK_ENTER(profilerBlockInstanceImmediateReport);
#endif

#ifdef _DEBUG
				++ms_numberOfVisibleObjects;
#endif

				DPVS::Object * dpvsObject = getInstance()->getObject();
				const Appearance * ourAppearance = static_cast<const Appearance *>(dpvsObject->getUserPointer());
				const Object * ourObject = ourAppearance->getOwner();

				// portal appearances don't have objects
				if (ourObject)
				{
					m_objectRenderedThisFrame.insert(ourObject->getNetworkId());

#if COMMANDER_USE_DPVS_TRANSFORMS
					if (m_useDpvsTransform)
					{
						// get the DPVS appearance to camera transform
						DPVS::Matrix4x4 dpvsTransform;
						getInstance()->getObjectToCameraMatrix(dpvsTransform);
						Transform a2c(Transform::IF_none);
						RenderWorld::convertToTransform(dpvsTransform, a2c);

						// calculate the appearance to world transform
						Transform a2w(Transform::IF_none);
						a2w.multiply(ms_cameraToWorld, a2c);
						ourAppearance->setTransform_w(a2w);
					}
					else
#endif
					{
						ourAppearance->setTransform_w(ourObject->getTransform_o2w());
					}
				}

				ourAppearance->preRender();

				// inform the appearance it was visible this frame
				ourAppearance->setRenderedThisFrame();

#if PRODUCTION == 0
				if (ms_profileInstanceImmediateReport)
					NP_PROFILER_BLOCK_LEAVE(profilerBlockInstanceImmediateReport);
#endif
			}
			break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case INSTANCE_VISIBLE:
			{
#if PRODUCTION == 0
				NP_PROFILER_BLOCK_DEFINE(profilerBlockInstanceVisible, "instance visible");
				if (ms_profileInstanceVisible)
					NP_PROFILER_BLOCK_ENTER(profilerBlockInstanceVisible);
#endif

				DPVS::Object * dpvsObject = getInstance()->getObject();
				const Appearance * ourAppearance = static_cast<const Appearance *>(dpvsObject->getUserPointer());
				NOT_NULL(ourAppearance);
				const Object * ourObject = ourAppearance->getOwner();
				NOT_NULL(ourObject);

				// @todo if this object had a write shape, it can incorrectly occlude things behind it
				if (!ourObject->isActive())
				{
#if PRODUCTION == 0
					if (ms_profileInstanceVisible)
						NP_PROFILER_BLOCK_LEAVE(profilerBlockInstanceVisible);
#endif
					break;
				}

#ifdef _DEBUG
				++ms_numberOfVisibleObjects;
#endif

				m_objectRenderedThisFrame.insert(ourObject->getNetworkId());

#if COMMANDER_USE_DPVS_TRANSFORMS
				if (ms_useDpvsTransform)
				{
					// get the DPVS appearance to camera transform
					Transform a2w(Transform::IF_none);
					DPVS::Matrix4x4 dpvsTransform;
					getInstance()->getObjectToCameraMatrix(dpvsTransform);
					Transform a2c(Transform::IF_none);
					RenderWorld::convertToTransform(dpvsTransform, a2c);

					// calculate the appearance to world transform
					a2w.multiply(ms_cameraToWorld, a2c);
					ourAppearance->setTransform_w(a2w);
				}
				else
#endif
				{
					// use the o2w transform as the a2w transform
					ourAppearance->setTransform_w(ourObject->getTransform_o2w());
				}

#if PRODUCTION == 0
				char const * const typeName = ms_profileInstanceVisibleRenderCall ? typeid(*ourAppearance).name() : NULL;
				NP_PROFILER_BLOCK_DEFINE(profilerBlockRenderCall, typeName);
				if (typeName)
					NP_PROFILER_BLOCK_ENTER(profilerBlockRenderCall);
#endif

				ourAppearance->render();

				// inform the appearance it was visible this frame
				ourAppearance->setRenderedThisFrame();

#if PRODUCTION == 0
				if (typeName)
					NP_PROFILER_BLOCK_LEAVE(profilerBlockRenderCall);
#endif

#ifdef _DEBUG
				if (ms_renderFrames)
					ms_camera->addDebugPrimitive(new FrameDebugPrimitive(UtilityDebugPrimitive::S_none, ourAppearance->getTransform_w(), 1.0f));

				if (ms_renderSpheres)
				{
					// get the sphere.  DPVS returns it in cell coordinate space.
					DPVS::Vector3 center;
					float radius;
					dpvsObject->getSphere(center, radius);

					if (ourObject->getAppearance()->debugRenderingEnabled())
					{
						// get the cell-to-world transform
						DPVS::Cell *dpvsCell = dpvsObject->getCell();
						DPVS::Matrix4x4 dpvsCellTransform;
						dpvsCell->getWorldToCellMatrix(dpvsCellTransform);
						Transform cellTransform(Transform::IF_none);
						RenderWorld::convertToTransform(dpvsCellTransform, cellTransform);

						ms_camera->addDebugPrimitive(new SphereDebugPrimitive(UtilityDebugPrimitive::S_none, cellTransform, V3_TO_VECTOR(center), radius, 10, 50));
					}
				}
#endif

#if PRODUCTION == 0
				if (ms_profileInstanceVisible)
					NP_PROFILER_BLOCK_LEAVE(profilerBlockInstanceVisible);
#endif
			}
	 		break;

			////////////////////////////////////////////////////////////////////////////////////////////////////
		case REMOVAL_SUGGESTED:
			RenderWorld::leakedObject(Commander::getInstance()->getObject());
			break;
			
			////////////////////////////////////////////////////////////////////////////////////////////////////
		default:
			break;
	}; //lint !e788  //-- Command::* not used within switch
}

// ======================================================================
