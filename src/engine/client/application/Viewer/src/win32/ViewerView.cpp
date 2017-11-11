// ======================================================================
//
// ViewerView.cpp
// Andy Sommers
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#include "FirstViewer.h"
#include "Viewer.h"

#include "LightDialog.h"
#include "LightListDialog.h"
#include "MainFrm.h"
#include "ViewerDoc.h"
#include "ViewerPreferences.h"
#include "ViewerView.h"

#include "clientGame/Bloom.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/DebugPrimitive.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/GraphicsOptionTags.h"
#include "clientGraphics/GraphicsDebugFlags.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/PostProcessingEffectsManager.h"
#include "clientGraphics/RenderWorld.h"
#include "clientGraphics/RenderWorldCamera.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/ComponentAppearance.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/MeshAppearance.h"
#include "clientObject/MeshAppearanceTemplate.h"
#include "clientObject/ShadowManager.h"
#include "clientObject/ShadowVolume.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientSkeletalAnimation/StateHierarchyAnimationController.h"
#include "clientSkeletalAnimation/TrackAnimationController.h"
#include "clientSkeletalAnimation/TrackAnimationController_Track.h"
#include "clientSkeletalAnimation/TrackAnimationController_TrackTemplate.h"
#include "clientTextureRenderer/TextureRenderer.h"
#include "clientObject/ShaderAppearance.h"
#include "clientObject/ViewerLightAppearance.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/CollisionInfo.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Extent.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/PackedRgb.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/Hardpoint.h"
#include "sharedObject/ObjectList.h"
#include "sharedUtility/FileName.h"

#include <typeinfo>

// ======================================================================

namespace
{
	bool debugMetrics = true;
	bool ms_noRender = false;

	const Tag TAG_DOT3 = TAG(D,O,T,3);

	static const char* const ms_testShaderName = "shader/test_shader.sht";
}
// ======================================================================

IMPLEMENT_DYNCREATE(CViewerView, CView)

BEGIN_MESSAGE_MAP(CViewerView, CView)
	//{{AFX_MSG_MAP(CViewerView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_BUTTON_AXIS, OnButtonAxis)
	ON_COMMAND(ID_BUTTON_GRID, OnButtonGrid)
	ON_COMMAND(ID_BUTTON_RESET, OnButtonReset)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_GRID, OnUpdateButtonGrid)
	ON_COMMAND(ID_BUTTON_POLYCOUNT, OnButtonPolycount)
	ON_COMMAND(ID_BUTTON_TOGGLE_WIRE, OnButtonToggleWire)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TOGGLE_WIRE, OnUpdateButtonToggleWire)
	ON_COMMAND(ID_BUTTON_SHOWEXTENTS, OnButtonShowextents)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SHOWEXTENTS, OnUpdateButtonShowextents)
	ON_COMMAND(ID_BUTTON_FACENORMALS, OnButtonFacenormals)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_FACENORMALS, OnUpdateButtonFacenormals)
	ON_COMMAND(ID_BUTTON_VERTEXNORMALS, OnButtonVertexnormals)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_VERTEXNORMALS, OnUpdateButtonVertexnormals)
	ON_COMMAND(ID_BUTTON_TOGGLEOBJECT, OnButtonToggleobject)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_TOGGLEOBJECT, OnUpdateButtonToggleobject)
	ON_WM_CHAR()
	ON_UPDATE_COMMAND_UI(ID_BUTTON_AXIS, OnUpdateButtonAxis)
	ON_COMMAND(ID_BUTTON_MIPMAP_DECREASE, OnButtonMipmapDecrease)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_MIPMAP_DECREASE, OnUpdateButtonMipmapDecrease)
	ON_COMMAND(ID_BUTTON_MIPMAP_INCREASE, OnButtonMipmapIncrease)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_MIPMAP_INCREASE, OnUpdateButtonMipmapIncrease)
	ON_COMMAND(ID_BUTTON_MIPMAP_RESET, OnButtonMipmapReset)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_MIPMAP_RESET, OnUpdateButtonMipmapReset)
	ON_COMMAND(ID_BUTTON_LOD_RESET, OnButtonLodReset)
	ON_COMMAND(ID_BUTTON_LOD_INCREASE, OnButtonLodIncrease)
	ON_COMMAND(ID_BUTTON_LOD_DECREASE, OnButtonLodDecrease)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_LOD_RESET, OnUpdateButtonLodReset)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_LOD_INCREASE, OnUpdateButtonLodIncrease)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_LOD_DECREASE, OnUpdateButtonLodDecrease)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_BUTTON_LIGHT_SETUP, OnButtonLightSetup)
	ON_COMMAND(ID_BUTTON_LIGHT_ICONS, OnButtonLightIcons)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_LIGHT_ICONS, OnUpdateButtonLightIcons)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_LIGHT_SETUP, OnUpdateButtonLightSetup)
	ON_COMMAND(ID_VIEW_14AMBIENTFULLDIRECTIONAL, OnView14ambientfulldirectional)
	ON_UPDATE_COMMAND_UI(ID_VIEW_14AMBIENTFULLDIRECTIONAL, OnUpdateView14ambientfulldirectional)
	ON_COMMAND(ID_VIEW_FULLLIGHTING, OnViewFulllighting)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLLIGHTING, OnUpdateViewFulllighting)
	ON_COMMAND(ID_VIEW_LIGHTINGUSERDEFINED, OnViewLightinguserdefined)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LIGHTINGUSERDEFINED, OnUpdateViewLightinguserdefined)
	ON_COMMAND(ID_VIEW_HEMISPHERIC_COLORS, OnViewLightingShowHemisphericColors)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HEMISPHERIC_COLORS, OnUpdateViewLightingShowHemisphericColors)
	ON_COMMAND(ID_VIEW_RENDERSOLID, OnViewRendersolid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERSOLID, OnUpdateViewRendersolid)
	ON_COMMAND(ID_VIEW_RENDERTEXTURED, OnViewRendertextured)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERTEXTURED, OnUpdateViewRendertextured)
	ON_COMMAND(ID_VIEW_RENDERWIRE, OnViewRenderwire)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERWIRE, OnUpdateViewRenderwire)
	ON_COMMAND(ID_VIEW_ANIMATEALPHA, OnViewAnimatealpha)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ANIMATEALPHA, OnUpdateViewAnimatealpha)
	ON_COMMAND(ID_VIEW_LIGHTINGOFF, OnViewLightingoff)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LIGHTINGOFF, OnUpdateViewLightingoff)
	ON_COMMAND(ID_VIEW_GLGRAPHICSLAYERMETRICS, OnViewGlgraphicslayermetrics)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GLGRAPHICSLAYERMETRICS, OnUpdateViewGlgraphicslayermetrics)
	ON_WM_KEYUP()
	ON_COMMAND(ID_VIEW_SHADOWVOLUMES, OnViewShadowvolumes)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHADOWVOLUMES, OnUpdateViewShadowvolumes)
	ON_COMMAND(ID_VIEW_RENDERNPATCHES, OnViewRendernpatches)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RENDERNPATCHES, OnUpdateViewRendernpatches)
	ON_COMMAND(ID_BUTTON_VIEWRADARSHAPE, OnButtonViewradarshape)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_VIEWRADARSHAPE, OnUpdateButtonViewradarshape)
	ON_COMMAND(ID_BUTTON_VIEWTESTSHAPE, OnButtonViewtestshape)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_VIEWTESTSHAPE, OnUpdateButtonViewtestshape)
	ON_COMMAND(ID_BUTTON_VIEWWRITESHAPE, OnButtonViewwriteshape)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_VIEWWRITESHAPE, OnUpdateButtonViewwriteshape)
	ON_COMMAND(ID_VIEW_SHOWSHADOWEXTRUDEDEDGES, OnViewShowshadowextrudededges)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWSHADOWEXTRUDEDEDGES, OnUpdateViewShowshadowextrudededges)
	ON_COMMAND(ID_VIEW_SHOWSHADOWNONMANIFOLDEDGES, OnViewShowshadownonmanifoldedges)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWSHADOWNONMANIFOLDEDGES, OnUpdateViewShowshadownonmanifoldedges)
	ON_COMMAND(ID_VIEW_ANIMATELIGHT, OnViewAnimatelight)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ANIMATELIGHT, OnUpdateViewAnimatelight)
	ON_COMMAND(ID_EDIT_USETESTSHADER, OnEditUsetestshader)
	ON_UPDATE_COMMAND_UI(ID_EDIT_USETESTSHADER, OnUpdateEditUsetestshader)
	ON_COMMAND(ID_VIEW_HARDPOINTS, OnViewHardpoints)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HARDPOINTS, OnUpdateViewHardpoints)
	ON_COMMAND(ID_COLLISIONINFO_DRAWCMESHES, OnCollisionInfoDrawCMeshes)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONINFO_DRAWCMESHES, OnUpdateCollisionInfoDrawCMeshes)
	ON_COMMAND(ID_COLLISIONINFO_DRAWEXTENTS, OnCollisionInfoDrawExtents)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONINFO_DRAWEXTENTS, OnUpdateCollisionInfoDrawExtents)
	ON_COMMAND(ID_COLLISIONINFO_DRAWFLOORS, OnCollisionInfoDrawFloors)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONINFO_DRAWFLOORS, OnUpdateCollisionInfoDrawFloors)
	ON_COMMAND(ID_COLLISIONINFO_DRAWPATHNODES, OnCollisionInfoDrawPathNodes)
	ON_UPDATE_COMMAND_UI(ID_COLLISIONINFO_DRAWPATHNODES, OnUpdateCollisionInfoDrawPathNodes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

inline bool keyDown (uint key)
{
	return (GetKeyState (key) & 0x8000) != 0;
}

// ======================================================================

CViewerView::CViewerView()
:	badRenderCount(0),
	nPatchSegments(1.0f),
	m_showCollideTestRenderGeometry (false),
	m_collideTestRenderGeometryStart (),
	m_collideTestRenderGeometryEnd (),
	m_showCollideTestCollisionGeometry (false),
	m_rayTestCollisionGeometry(false),
	m_collideTestCollisionGeometryStart (),
	m_collideTestCollisionGeometryEnd (),
	m_animateAlpha(false),
	m_animatedAlpha(0.0f)
{
	cellObject = new Object();
	cellProperty = new CellProperty(*cellObject);
	cellObject->addProperty(*cellProperty);
	cellObject->addToWorld();

	camera     = new RenderWorldCamera ();
	camera->setNearPlane (0.1f);
	camera->setParentCell(cellProperty);
	camera->addToWorld ();
}

// ----------------------------------------------------------------------

void CViewerView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	yaw   = PI - PI_OVER_4;
	pitch = PI_OVER_4;

	recenter ();

	m_showCollideTestRenderGeometry = false;
	m_collideTestRenderGeometryStart.makeZero ();
	m_collideTestRenderGeometryEnd.makeZero ();

	m_showCollideTestCollisionGeometry = false;
	m_rayTestCollisionGeometry = false;
	m_collideTestCollisionGeometryStart.makeZero ();
	m_collideTestCollisionGeometryEnd.makeZero ();
}

// ----------------------------------------------------------------------

CViewerView::~CViewerView()
{
	delete camera;

	CViewerDoc * const pDoc = dynamic_cast<CViewerDoc*> (GetDocument ());
	if (pDoc && pDoc->getObject ())
	{
		if (pDoc->getObject ()->isInWorld ())
			pDoc->getObject ()->removeFromWorld ();
		pDoc->getObject()->setParentCell(CellProperty::getWorldCellProperty());
	}

	delete cellObject;
}

// ----------------------------------------------------------------------

void CViewerView::OnDestroy() 
{
	CView::OnDestroy();
	
	//-- close up all other views since our graphics viewer is the primary. We can
	//   have multiple views, but this one is the master.  when it dies, they all die.
	CViewerDoc *const pDoc = dynamic_cast<CViewerDoc*>(GetDocument());
	if (pDoc)
	{
		pDoc->closeSkeletalAppearanceTemplateView();
		pDoc->closeVariableSetView();
	}
}

// ----------------------------------------------------------------------

BOOL CViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.cy = cs.cx;

	return CView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

float CViewerView::getAlpha()
{
	if (!m_animateAlpha)
		return 1.0f;

	float alpha = 0.0f;
	if (m_animatedAlpha < 1.0f)
	{
		// from time 0 to 1, the object should be invisible
		alpha = 0.0f;
	}
	else
		if (m_animatedAlpha < 2.0f)
		{
			// from 1 to 2, the object should fade in
			alpha = m_animatedAlpha - 1.0f;
		}
		else
			if (m_animatedAlpha < 3.0f)
			{
				// from 2 to 3, the object should be opaque
				alpha = 1.0f;
			}
			else
			{
				// from 3 to 4, the object should be fading out
				alpha = 4.0f - m_animatedAlpha;
			}

	return alpha;
}

// ----------------------------------------------------------------------

void CViewerView::OnDraw(CDC* pDC)
{
	if (ms_noRender)
		return;

	CViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	Object* const object = pDoc->getObject ();

	if (!object)
		return;

	if (!object->getAppearance ())
	{
		char buffer [1000];
		sprintf (buffer, "could not render %s", (LPCTSTR) pDoc->getFileName ());
		pDC->SetBkMode (OPAQUE);
		pDC->TextOut (0, 0, buffer);

		return;
	}

	if (m_animateAlpha)
	{
		MeshAppearance * const meshAppearance = dynamic_cast<MeshAppearance *>(object->getAppearance());
		if (meshAppearance)
		{
			float const alpha = getAlpha();
			meshAppearance->setAlpha(true, alpha, true, alpha);
		}
	}

	pDC->SetBkMode (TRANSPARENT);

	if (!object->isInWorld())
	{
		object->setParentCell (cellProperty);
		object->addToWorld ();
	}

	// don't try to draw if we're in the middle of crashing, because that will just crash all over again
	if (!ExitChain::isFataling())
	{
		// point the Gl at this window
		Graphics::setViewport(0, 0, camera->getViewportWidth(), camera->getViewportHeight());

		// render a frame
		Graphics::beginScene();

			PostProcessingEffectsManager::preSceneRender();
			Bloom::setEnabled(ViewerPreferences::getBloomEnabled());
			Bloom::preSceneRender();

			ViewerPreferences::beginScene (object, cellProperty);

			bool didFatal = false;

			// render the object
			FatalSetThrowExceptions(false);
			try
			{
				// render the scene
				camera->renderScene();

				ShadowVolume::renderShadowAlpha (camera);
			}
			catch (FatalException &e)
			{
				didFatal = true;
				++badRenderCount;
				if (badRenderCount == 1)
				{
					// only display this message the first time
					MessageBox(e.getMessage(), "Error rendering object (is animation valid for mesh)?", MB_ICONSTOP);
				}

				// stop any animations --- allows the user to see the object if the problem was related to animation
				pDoc->stopAnimation();
			}
			if (!didFatal)
				badRenderCount = 0;

#ifdef _DEBUG
			int vertices = 0;
			int points = 0;
			int lines = 0;
			int triangles = 0;
			int calls = 0;
			Graphics::getRenderedVerticesPointsLinesTrianglesCalls(vertices, points, lines, triangles, calls);
#endif

			FatalSetThrowExceptions(false);

			ViewerPreferences::endScene ();

			//-- render extents
			if (ViewerPreferences::getShowExtents () && object->getAppearance () && object->getAppearance ()->getExtent ())
			{
				camera->applyState();
				Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorStaticShader());
				Graphics::setObjectToWorldTransformAndScale(object->getTransform_o2w(), object->getScale());
				Graphics::drawExtent (object->getAppearance ()->getExtent (), VectorArgb::solidYellow);
			}

			//-- render axis
			if (ViewerPreferences::getShowAxis ())
			{
				const float radius = object->getAppearanceSphereRadius ();

				camera->applyState();
				Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
				Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);
				Graphics::drawFrame (radius * 1.25f);
			}

			if (m_showCollideTestRenderGeometry)
			{
				Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
				Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);

				VectorArgb color = VectorArgb::solidGreen;

				const Vector start_o = object->rotateTranslate_w2o (m_collideTestRenderGeometryStart);
				const Vector end_o   = object->rotateTranslate_w2o (m_collideTestRenderGeometryEnd);

				CollisionInfo result;
				result.setPoint (end_o);

				bool hit = false;
				if (object->getAppearance () && object->getAppearance ()->collide (start_o, end_o, CollideParameters::cms_default, result))
				{
					hit = true;
					result.setPoint (object->rotateTranslate_o2w (result.getPoint ()));
					result.setNormal (object->rotate_o2w (result.getNormal ()));

					color = VectorArgb::solidRed;
				}

				Graphics::drawLine (m_collideTestRenderGeometryStart, m_collideTestRenderGeometryEnd, color);

				if (hit)
					Graphics::drawLine(result.getPoint(), result.getPoint() + (result.getNormal() * object->getAppearanceSphereRadius() * 0.1f), VectorArgb::solidGreen);
			}

			if (m_showCollideTestCollisionGeometry)
			{
				Graphics::setStaticShader(ShaderTemplateList::get3dVertexColorZStaticShader());
				Graphics::setObjectToWorldTransformAndScale(Transform::identity, Vector::xyz111);

				VectorArgb color(VectorArgb::solidGreen);

				Vector const start_o(object->rotateTranslate_w2o(m_collideTestCollisionGeometryStart));
				Vector const end_o(object->rotateTranslate_w2o(m_collideTestCollisionGeometryEnd));

				bool hit = false;
				if (object->getAppearance() && object->getAppearance()->getAppearanceTemplate())
				{
					Vector point;
					Vector normal;
					float time;

					Extent const * const extent = object->getAppearance()->getAppearanceTemplate()->getCollisionExtent();
					if (extent->intersect(start_o, end_o, &normal, &time))
					{
						hit = true;

						point = object->rotateTranslate_o2w(Vector::linearInterpolate(start_o, end_o, time));
						normal = object->rotate_o2w(normal);

						color = VectorArgb::solidRed;
					}

					Graphics::drawLine(m_collideTestCollisionGeometryStart, m_collideTestCollisionGeometryEnd, color);

					if (hit)
						Graphics::drawLine(point, point + (normal * object->getAppearanceSphereRadius() * 0.1f), VectorArgb::solidGreen);
				}
			}

			if (m_rayTestCollisionGeometry)
			{
				m_rayTestCollisionGeometry = false;
				if (object->getCollisionProperty() != 0)
				{
					typedef std::vector<float> ElapsedTimes;
					ElapsedTimes totalTimes;
					ElapsedTimes hitTimes;
					ElapsedTimes missedTimes;

					// get the bounds and scale 25%
					AxialBox const boundingBox(object->getCollisionProperty()->getExtent_l()->getBoundingBox());
					Vector const minimumBounds = boundingBox.getMin() * 1.25f;
					Vector const maximumBounds = boundingBox.getMax() * 1.25f;

					int const numberOfStepsAlongAxis = 50;
					float const testLength = 60.0f;
					Vector const span((maximumBounds - minimumBounds) / static_cast<float>(numberOfStepsAlongAxis));

					for (int x = 0; x < numberOfStepsAlongAxis; ++x)
					{
						for (int y = 0; y < numberOfStepsAlongAxis; ++y)
						{
							for (int z = 0; z < numberOfStepsAlongAxis; ++z)
							{
								Vector const begin_o(span.x * x, span.y * y, span.z * z);
								Vector direction(begin_o.x * -1.0f, begin_o.y * -1.0f, begin_o.z * -1.0f);
								if (direction.normalize())
								{
									Vector const end_o(begin_o + (direction * testLength));

									BaseExtent const * const extent = object->getCollisionProperty()->getExtent_l();
									float elapsedTimeSeconds = 0.0f;
									bool hit = false;

									if (extent != 0)
									{
										unsigned long const startTimeMilliSeconds = Clock::timeMs();
										hit = extent->intersect(begin_o, end_o);
										unsigned long const stopTimeMilliSeconds = Clock::timeMs();
										elapsedTimeSeconds = static_cast<float>(stopTimeMilliSeconds - startTimeMilliSeconds) / 1000.0f;
									}

									totalTimes.push_back(elapsedTimeSeconds);
									if (hit)
									{
										hitTimes.push_back(elapsedTimeSeconds);
									}
									else
									{
										missedTimes.push_back(elapsedTimeSeconds);
									}
								}
							}
						}
					}

					if (totalTimes.size() > 0)
					{
						float total = 0.0f;
						for (unsigned int i = 0; i < totalTimes.size(); ++i)
							total += totalTimes[i];
						DEBUG_REPORT_LOG(true, ("%d total tests averaging %f seconds each\n", totalTimes.size(), total / static_cast<float>(totalTimes.size())));
					}

					if (hitTimes.size() > 0)
					{
						float total = 0.0f;
						for (unsigned int i = 0; i < hitTimes.size(); ++i)
							total += hitTimes[i];
						DEBUG_REPORT_LOG(true, ("%d total hits averaging %f seconds each\n", hitTimes.size(), total / static_cast<float>(hitTimes.size())));
					}

					if (missedTimes.size() > 0)
					{
						float total = 0.0f;
						for (unsigned int i = 0; i < missedTimes.size(); ++i)
							total += missedTimes[i];
						DEBUG_REPORT_LOG(true, ("%d total misses averaging %f seconds each\n", missedTimes.size(), total / static_cast<float>(missedTimes.size())));
					}
				}
			}


#ifdef _DEBUG
			if (ViewerPreferences::getShowHardpoints ())
			{
				const AppearanceTemplate* const appearanceTemplate = object ? (object->getAppearance () ? object->getAppearance ()->getAppearanceTemplate () : 0) : 0;
				if (appearanceTemplate)
				{
					const int numberOfHardpoints = appearanceTemplate->getHardpointCount();
					int i;
					for (i = 0; i < numberOfHardpoints; ++i)
					{
						const Hardpoint& hardpoint = appearanceTemplate->getHardpoint (i);
						Transform hardpointToWorldTransform;
						hardpointToWorldTransform.multiply (object->getTransform_o2w (), hardpoint.getTransform ());
						camera->addDebugPrimitive (new FrameDebugPrimitive (FrameDebugPrimitive::S_z, hardpointToWorldTransform, object->getAppearanceSphereRadius () * 0.2f));

						float screenX;
						float screenY;
						if (camera->projectInWorldSpace (hardpointToWorldTransform.getPosition_p (), &screenX, &screenY, 0))
						{
							char buffer [1000];
							sprintf (buffer, "%s", hardpoint.getName ().getString ());
							pDC->TextOut (static_cast<int> (screenX), static_cast<int> (screenY), buffer);
						}
					}
				}
			}
#endif

			Bloom::postSceneRender();
			PostProcessingEffectsManager::postSceneRender();

		// done rendering the frame
		Graphics::endScene();
		Graphics::present(m_hWnd, camera->getViewportWidth(), camera->getViewportHeight());

		//-- special text
#ifdef _DEBUG
		{
			if (debugMetrics)
			{
				char buffer [256];
				sprintf (buffer, "%8d=verts %8d=points %8d=lines %8d=tris %8d=calls", vertices, points, lines, triangles, calls);
				pDC->TextOut (0, 0, buffer);
			}
		}
#endif

		{
			const ShaderAppearance* sappear = dynamic_cast<const ShaderAppearance*> (object->getAppearance ());
			if (sappear)
			{
				char buffer [1000];
				sprintf (buffer, "Ignoring %i mipmap levels", sappear->getIgnoreMipmapLevelCount ());
				pDC->TextOut (0, 12, buffer);
			}
		}

		{
			Appearance const * appearance = object->getAppearance();

			Vector center = appearance->getSphere().getCenter();

			Vector worldCenter = object->getTransform_a2w().rotateTranslate_l2p(center);

			const real distance = camera->getPosition_w().magnitudeBetween(worldCenter);

			char buffer [1000];
			sprintf (buffer, "Distance to camera: %1.2f", distance);
			pDC->TextOut (0, 24, buffer);
		}

		{
			Appearance const * const appearance = object->getAppearance();
			if (appearance)
			{
				BoxExtent const * boxExtent = dynamic_cast<BoxExtent const *> (appearance->getExtent ());
				if (boxExtent)
				{
					Vector const min = boxExtent->getMin ();
					Vector const max = boxExtent->getMax ();
					Vector const center = boxExtent->getSphere ().getCenter ();
					float const radius = boxExtent->getSphere ().getRadius ();

					char buffer [1000];
					sprintf (buffer, "SphereExtent: center=<%1.2f,%1.2f,%1.2f>  radius=%1.2f", center.x, center.y, center.z, radius);
					pDC->TextOut (0, 36, buffer);

					sprintf (buffer, "BoxExtent: left=%1.2f  right=%1.2f  bottom=%1.2f  top=%1.2f  back=%1.2f  front=%1.2f", min.x, max.x, min.y, max.y, min.z, max.z);
					pDC->TextOut (0, 48, buffer);
				}
			}

		

			if (m_animateAlpha)
			{
				char buffer [128];
				sprintf (buffer, "Alpha: %6.4f", getAlpha());
				pDC->TextOut (0, 60, buffer);
			}

			{
				Object* object = pDoc->getObject();
				Appearance *appearance = NULL;
				SkeletalAppearance2* skeletalAppearance2 = NULL;
				TransformAnimationController* transformAnimationController = NULL;
				StateHierarchyAnimationController* stateHierarchyAnimationController = NULL;

				// drill down to the controller
				if(object)
				{
					appearance = object->getAppearance();
					if(appearance)
					{
						skeletalAppearance2 = dynamic_cast<SkeletalAppearance2 *>(appearance);
						if(skeletalAppearance2)
						{
							transformAnimationController = skeletalAppearance2->getAnimationController();
							if(transformAnimationController)
							{
								stateHierarchyAnimationController = transformAnimationController->asStateHierarchyAnimationController();
							}
						}
					}
				}

				if(stateHierarchyAnimationController)
				{
					TrackAnimationController& trackAnimationController = stateHierarchyAnimationController->getTrackAnimationController();
					UNREF(trackAnimationController);
					int numTrackTemplates = trackAnimationController.getTrackTemplateCount();
					char buffer[1000];
					int x0,y0,x1,y1;
					camera->getViewport(x0,y0,x1,y1);
					for(int i = 0; i < numTrackTemplates; ++i)
					{
						const TrackAnimationController::TrackTemplate& trackTemplate = trackAnimationController.getTrackTemplate(i);
						AnimationTrackId animationTrackId = trackTemplate.getTrackId();

						const TrackAnimationController::Track* track = trackAnimationController.getTrackConstPtr(animationTrackId);
						const SkeletalAnimation* skeletalAnimation = track->fetchCurrentAnimation();
						std::string animationName = "-none-";
						if(skeletalAnimation)
						{
							animationName = skeletalAnimation->getLeafAnimationTemplateName().getString();
							skeletalAnimation->release();
						}
							
						sprintf(buffer,"(%d)(%s)",
							i,
							trackTemplate.getTrackName().getString()
						);
						int yText = (y1 - (numTrackTemplates - i) * 14);
						if(yText < 0)
							yText = 0;
						pDC->TextOutA(0,yText, buffer);
						sprintf(buffer,"[%s]", animationName.c_str());
						pDC->TextOutA(150,yText, buffer);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void CViewerView::AssertValid() const
{
	CView::AssertValid();
}

// ----------------------------------------------------------------------

void CViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

// ----------------------------------------------------------------------

CViewerDoc* CViewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CViewerDoc)));
	return (CViewerDoc*)m_pDocument;
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void CViewerView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here

	// do not set the viewport to a 0 width or height
	if (cx && cy)
		camera->setViewport(0, 0, cx, cy);

	Invalidate ();
}

// ----------------------------------------------------------------------

BOOL CViewerView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	UNREF(pDC);
	return TRUE; // CView::OnEraseBkgnd(pDC);	
}

// ----------------------------------------------------------------------

void CViewerView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CViewerDoc* pDoc = GetDocument();

	// TODO: Add your message handler code here and/or call default
	static CPoint lastPoint = point;
	static bool   first     = false;
	static Vector lastVector;
	static real   dist;

	if (keyDown (VK_MENU))
	{
		SetCapture ();

		CRect rect;
		GetClientRect (&rect);

		if (ViewerPreferences::getCameraMode () == ViewerPreferences::CM_free)
		{
			if (nFlags & MK_LBUTTON)
			{
				if (!first)
				{
					first = true;
				}
				else
				{
					real yaw = PI_TIMES_2 * static_cast<real> (point.x - lastPoint.x) / rect.Width ();
					real pitch = PI_TIMES_2 * static_cast<real> (point.y - lastPoint.y) / rect.Height ();

					camera->yaw_o (yaw);
					camera->pitch_o (pitch);

					// keep the up vector pointed up
					Vector up = camera->rotate_w2o(Vector::unitY);
					const float roll = -atan2(up.x, up.y);
					camera->roll_o(roll);

					Invalidate ();
				}

				lastPoint = point;
			}
			else
				if (nFlags & MK_MBUTTON)
				{
					real xMod = pDoc->getObject ()->getAppearanceSphereRadius () * static_cast<real> (point.x - lastPoint.x) / rect.Width ();
					real yMod = pDoc->getObject ()->getAppearanceSphereRadius () * static_cast<real> (point.y - lastPoint.y) / rect.Height ();

					camera->move_o (Vector (-xMod, yMod, 0));

					Invalidate ();
				}
		}
		else
		{
			{
				//-- zoom mode
				if (nFlags & MK_LBUTTON && nFlags & MK_MBUTTON)
				{
					first = false;

					//-- calculate how far to move
					real zoomMod = distanceToPoint * static_cast<real> (point.x - lastPoint.x) / rect.Width ();

					//-- move camera
					camera->move_o (Vector (0, 0, zoomMod));

					//-- push point if needed
					distanceToPoint -= zoomMod;
					const real minViewDistance = pDoc->getObject()->getAppearance() ? pDoc->getObject ()->getAppearanceSphereRadius () * CONST_REAL(0.5) : CONST_REAL(0.5);

					if (distanceToPoint < minViewDistance)
						distanceToPoint = minViewDistance;
					
					Invalidate ();
				}
				else
					//-- rotate mode
					if (nFlags & MK_LBUTTON)
					{
						//--
						CRect rect;
						GetClientRect (&rect);

						if (!first)
						{
							first      = true;

							lastVector = camera->rotateTranslate_o2w (Vector::unitZ * CONST_REAL (distanceToPoint));
							dist       = distanceToPoint;
						}
						else
						{
							real yawMod = PI_TIMES_2 * static_cast<real> (point.x - lastPoint.x) / rect.Width ();
							yaw += yawMod;

							real pitchMod = PI_TIMES_2 * static_cast<real> (point.y - lastPoint.y) / rect.Height ();
							pitch += pitchMod;
							pitch = clamp (-PI_OVER_2, pitch, PI_OVER_2);

							Transform t;
							t.setPosition_p(lastVector);
							t.yaw_l (yaw);
							t.pitch_l (pitch);
							t.move_l (Vector (0, 0, -dist));
							camera->setTransform_o2p(t);

							Invalidate ();
						}
					}
					else
						//-- pan mode
						if (nFlags & MK_MBUTTON)
						{
							first = false;

							//--
							CRect rect;
							GetClientRect (&rect);

							real xMod = pDoc->getObject ()->getAppearanceSphereRadius () * static_cast<real> (point.x - lastPoint.x) / rect.Width ();
							real yMod = pDoc->getObject ()->getAppearanceSphereRadius () * static_cast<real> (point.y - lastPoint.y) / rect.Height ();

							camera->move_o (Vector (-xMod, yMod, 0));
							
							Invalidate ();
						}
						else
							//-- rotate light mode
							if (nFlags & MK_RBUTTON)
							{
								first = false;

								//--
								CRect rect;
								GetClientRect (&rect);

								const float yawMod = PI_TIMES_2 * static_cast<real> (point.x - lastPoint.x) / rect.Width ();
								const float pitchMod = PI_TIMES_2 * static_cast<real> (point.y - lastPoint.y) / rect.Height ();

								ViewerPreferences::setLightYaw (ViewerPreferences::getLightYaw () + yawMod);
								ViewerPreferences::setLightPitch (ViewerPreferences::getLightPitch () + pitchMod);

								Invalidate ();
							}
			}
		}
	}
	else
	{
		ReleaseCapture ();
		first = false;
	}

	//-- 
	lastPoint = point;

	CView::OnMouseMove(nFlags, point);
}

// ----------------------------------------------------------------------

void CViewerView::recenter (void)
{
	CViewerDoc* pDoc = GetDocument();

	FatalSetThrowExceptions (true);
	try
	{
		const bool hasAppearance    = (pDoc->getObject() && pDoc->getObject()->getAppearance());
		Vector     appearanceCenter = (hasAppearance ? pDoc->getObject ()->getAppearanceSphereCenter () : Vector::zero);
		real       appearanceRadius = (hasAppearance ? pDoc->getObject ()->getAppearanceSphereRadius () : 5.0f);

		if (hasAppearance)
		{
			SkeletalAppearance2* sa = dynamic_cast<SkeletalAppearance2*> (pDoc->getObject ()->getAppearance ());
			if (sa)
			{
				const BoxExtent& extent = sa->getExactMeshExtent ();
				appearanceCenter = extent.getSphere ().getCenter ();
				appearanceRadius = extent.getSphere ().getRadius ();
			}
		}

		distanceToPoint = 3.f * appearanceRadius;

		camera->resetRotateTranslate_o2p ();
		camera->setPosition_p (appearanceCenter);
		camera->yaw_o (yaw);
		camera->pitch_o (pitch);
		camera->move_o (Vector (0, 0, -distanceToPoint));
		camera->setParentCell(cellProperty);
	}
	catch (FatalException& e)
	{
		CONSOLE_PRINT (e.getMessage ());
		CONSOLE_PRINT ("\r\n");

		//-- delete the appearance
		pDoc->getObject ()->setAppearance (0);
	}
	FatalSetThrowExceptions (false);
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonAxis() 
{
	// TODO: Add your command handler code here
	ViewerPreferences::setShowAxis (!ViewerPreferences::getShowAxis ());

	Invalidate ();
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonGrid() 
{
	// TODO: Add your command handler code here
	ViewerPreferences::setShowGrid (!ViewerPreferences::getShowGrid ());

	Invalidate ();
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonReset() 
{
	// TODO: Add your command handler code here
	recenter ();	

	Invalidate ();
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonAxis(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (ViewerPreferences::getShowAxis ());
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonGrid(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (ViewerPreferences::getShowGrid ());
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonPolycount() 
{
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonToggleWire() 
{
	CViewerDoc* pDoc = GetDocument();

	if (pDoc->getObject () && pDoc->getObject ()->getAppearance ())
	{
		SkeletalAppearance2::setShowSkeleton(!SkeletalAppearance2::getShowSkeleton());
		Invalidate ();
	}
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonToggleWire(CCmdUI* pCmdUI) 
{
	CViewerDoc* pDoc = GetDocument();

	if (pDoc->getObject () && pDoc->getObject ()->getAppearance ())
	{
		const SkeletalAppearance2 *const sa = dynamic_cast<const SkeletalAppearance2*> (pDoc->getObject ()->getAppearance ());

		pCmdUI->Enable (sa != 0);
		pCmdUI->SetCheck (sa && SkeletalAppearance2::getShowSkeleton ());

		Invalidate ();
	}
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonShowextents() 
{
	ViewerPreferences::setShowExtents (!ViewerPreferences::getShowExtents ());

	Invalidate ();
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonShowextents(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getShowExtents ());
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonFacenormals() 
{
	ViewerPreferences::setShowFaceNormals (!ViewerPreferences::getShowFaceNormals ());

	Invalidate ();
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonFacenormals(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getShowFaceNormals ());
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonVertexnormals() 
{
	ViewerPreferences::setShowVertexNormals (!ViewerPreferences::getShowVertexNormals ());

	Invalidate ();
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonVertexnormals(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getShowVertexNormals ());
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonToggleobject() 
{
	CViewerDoc* pDoc = GetDocument();

	if (pDoc->getObject () && pDoc->getObject ()->getAppearance ())
	{
		SkeletalAppearance2 *const appearance = dynamic_cast<SkeletalAppearance2*>(pDoc->getObject ()->getAppearance());
		if (appearance)
			appearance->setShowMesh(!appearance->getShowMesh());

		Invalidate ();
	}
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonToggleobject(CCmdUI* pCmdUI) 
{
	CViewerDoc* pDoc = GetDocument();

	if (pDoc->getObject () && pDoc->getObject ()->getAppearance ())
	{
		const SkeletalAppearance2 *const sa = dynamic_cast<const SkeletalAppearance2*> (pDoc->getObject ()->getAppearance ());

		pCmdUI->Enable (sa != 0);
		pCmdUI->SetCheck (sa && sa->getShowMesh ());

		Invalidate ();
	}
}

// ----------------------------------------------------------------------

void CViewerView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	UNREF(nRepCnt);
	UNREF(nFlags);

	
	// do not use numbers because the artists bind those to animations
	
	// CView::OnChar(nChar, nRepCnt, nFlags);
	switch (nChar)
	{
	case ' ':
		{
			CViewerDoc* pDoc = GetDocument();
			pDoc->stopAnimation ();
		}
		break;

	case '!':
		{
#ifdef _DEBUG
			GraphicsDebugFlags::renderVertexMatrices = !GraphicsDebugFlags::renderVertexMatrices;
#endif
		}
		break;

	case '@':
		ViewerPreferences::setAnimateLight (!ViewerPreferences::getAnimateLight ());
		break;

	case '#':
		GraphicsOptionTags::toggle(TAG_DOT3);
		break;

	case 'f':
	case 'F':
		recenter ();
		break;

	case 'p':
	case 'P':
		distanceToPoint = 10.0f;
		break;

	case 'l':
	case 'L':
		{
			LightManager::LightData data;
			data.position = camera->getPosition_w ();
			data.theta    = camera->getObjectFrameK_w ().theta ();
			data.phi      = camera->getObjectFrameK_w ().phi ();

			LightDialog dlg (this, LightDialog::M_edit, ViewerPreferences::getConstLightManager (), UINT_MAX);
			dlg.setLightData (data);

			if (dlg.DoModal () == IDOK)
			{
				ViewerPreferences::getLightManager ().addLight (dlg.getLightData ());
				ViewerPreferences::reloadLights ();
			}
		}
		break;

	case 'a':
	case 'A':
		if (ViewerPreferences::getCameraMode () == ViewerPreferences::CM_free)
		{
			camera->move_o (Vector::negativeUnitX);
		}
		break;

	case 'd':
	case 'D':
		if (ViewerPreferences::getCameraMode () == ViewerPreferences::CM_free)
		{
			camera->move_o (Vector::unitX);
		}
		break;

	case 'w':
	case 'W':
		if (ViewerPreferences::getCameraMode () == ViewerPreferences::CM_free)
		{
			camera->move_o (Vector::unitZ);
		}
		break;

	case 's':
	case 'S':
		if (ViewerPreferences::getCameraMode () == ViewerPreferences::CM_free)
		{
			camera->move_o (Vector::negativeUnitZ);
		}
		break;

	case 'q':
	case 'Q':
		if (ViewerPreferences::getCameraMode () == ViewerPreferences::CM_free)
		{
			camera->move_o (Vector::unitY);
		}
		break;

	case 'z':
	case 'Z':
		if (ViewerPreferences::getCameraMode () == ViewerPreferences::CM_free)
		{
			camera->move_o (Vector::negativeUnitY);
		}
		break;

	case 'h':
	case 'H':
		{
			m_showCollideTestRenderGeometry = !m_showCollideTestRenderGeometry;

			if (m_showCollideTestRenderGeometry)
			{
				m_collideTestRenderGeometryStart = camera->getPosition_w ();
				m_collideTestRenderGeometryEnd   = m_collideTestRenderGeometryStart + (camera->getObjectFrameK_w () * 1000.f);
			}
		}
		break;

	case 'j':
	case 'J':
		{
			m_showCollideTestCollisionGeometry = !m_showCollideTestCollisionGeometry;

			if (m_showCollideTestCollisionGeometry)
			{
				m_collideTestCollisionGeometryStart = camera->getPosition_w ();
				m_collideTestCollisionGeometryEnd   = m_collideTestCollisionGeometryStart + (camera->getObjectFrameK_w () * 1000.f);
			}
		}
		break;
	case 'k':
	case 'K':
		{
			m_rayTestCollisionGeometry = true;
		}
		break;

#if 0
	// Test skinning.
	// handle transform scaling for skeleton appearance's first transform.

	// keys:
	//   q = scale x 75%, w = reset scale x, e = scale x 125%
	//   a = scale y 75%, s = reset scale y, d = scale y 125%
	//   z = scale z 75%, x = reset scale z, c = scale z 125%
	case 'q':
	case 'Q':
	case 'w':
	case 'W':
	case 'e':
	case 'E':
	case 'a':
	case 'A':
	case 's':
	case 'S':
	case 'd':
	case 'D':
	case 'z':
	case 'Z':
	case 'x':
	case 'X':
	case 'c':
	case 'C':
		{
			CViewerDoc* pDoc = GetDocument();
			if ( !(pDoc->getObject() && pDoc->getObject()->getAppearance()) )
				break;

			SkeletalAppearance2 *appearance = dynamic_cast<SkeletalAppearance2*>(pDoc->getObject()->getAppearance());
			if (!appearance)
				break;

			Skeleton *skeleton = appearance->getSkeleton();
			if (!skeleton)
				break;

			if (skeleton->getTransformCount() < 1)
				break;

			real *matrix = reinterpret_cast<real*>(skeleton->getTransformArray() + 0);
			
			switch (nChar)
			{
				case 'q':
				case 'Q':
					matrix[0*4 + 0] *= 0.75f;
					break;
				case 'w':
				case 'W':
					matrix[0*4 + 0] = 1.0f;
					break;
				case 'e':
				case 'E':
					matrix[0*4 + 0] *= 1.25f;
					break;

				case 'a':
				case 'A':
					matrix[1*4 + 1] *= 0.75f;
					break;
				case 's':
				case 'S':
					matrix[1*4 + 1] = 1.0f;
					break;
				case 'd':
				case 'D':
					matrix[1*4 + 1] *= 1.25f;
					break;

				case 'z':
				case 'Z':
					matrix[2*4 + 2] *= 0.75f;
					break;
				case 'x':
				case 'X':
					matrix[2*4 + 2] = 1.0f;
					break;
				case 'c':
				case 'C':
					matrix[2*4 + 2] *= 1.25f;
					break;
			}
		}
		break;
#endif

	default:
		{
			CViewerDoc* pDoc = GetDocument();
			pDoc->processKeyPress(nChar);
		}
		break;
	}
}

// ======================================================================

void CViewerView::OnButtonMipmapDecrease() 
{
	CViewerDoc* pDoc = GetDocument();

	ShaderAppearance* sappear = dynamic_cast<ShaderAppearance*> (pDoc->getObject ()->getAppearance ());
	if (sappear)
		sappear->decreaseIgnoreMipmapLevelCount ();
}

void CViewerView::OnUpdateButtonMipmapDecrease(CCmdUI* pCmdUI) 
{
	const CViewerDoc* const       pDoc = GetDocument();
	const ShaderAppearance* const sa   = dynamic_cast<const ShaderAppearance*> (pDoc->getObject ()->getAppearance ());
	pCmdUI->Enable (sa != 0);
}

void CViewerView::OnButtonMipmapIncrease() 
{
	CViewerDoc* pDoc = GetDocument();

	ShaderAppearance* sappear = dynamic_cast<ShaderAppearance*> (pDoc->getObject ()->getAppearance ());
	if (sappear)
		sappear->increaseIgnoreMipmapLevelCount ();
}

void CViewerView::OnUpdateButtonMipmapIncrease(CCmdUI* pCmdUI) 
{
	OnUpdateButtonMipmapDecrease (pCmdUI);
}

void CViewerView::OnButtonMipmapReset() 
{
	CViewerDoc* pDoc = GetDocument();

	ShaderAppearance* sappear = dynamic_cast<ShaderAppearance*> (pDoc->getObject ()->getAppearance ());
	if (sappear)
		sappear->resetIgnoreMipmapLevelCount ();
}

void CViewerView::OnUpdateButtonMipmapReset(CCmdUI* pCmdUI) 
{
	OnUpdateButtonMipmapDecrease (pCmdUI);
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonLodReset() 
{
	// TODO: Add your command handler code here
	CViewerDoc* pDoc = GetDocument();

	DetailAppearance* da = dynamic_cast<DetailAppearance*> (pDoc->getObject ()->getAppearance ());
	if (da)
	{
		da->toggleUserControlled ();
		return;
	}

	SkeletalAppearance2* sa = dynamic_cast<SkeletalAppearance2*> (pDoc->getObject ()->getAppearance ());
	if (sa)
		sa->setUserControlledDetailLevel (!sa->getUserControlledDetailLevel ());
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonLodIncrease() 
{
	// TODO: Add your command handler code here
	CViewerDoc* pDoc = GetDocument();

	DetailAppearance* da = dynamic_cast<DetailAppearance*> (pDoc->getObject ()->getAppearance ());
	if (da)
	{
		da->decrementDetailLevel ();
		return;
	}

	SkeletalAppearance2* sa = dynamic_cast<SkeletalAppearance2*> (pDoc->getObject ()->getAppearance ());
	if (sa)
	{
		sa->decrementDetailLevel ();
		pDoc->updateAnimationView (this, true);
	}
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonLodDecrease() 
{
	// TODO: Add your command handler code here
	CViewerDoc* pDoc = GetDocument();

	DetailAppearance* da = dynamic_cast<DetailAppearance*> (pDoc->getObject ()->getAppearance ());
	if (da)
	{
		da->incrementDetailLevel ();
		return;
	}

	SkeletalAppearance2* sa = dynamic_cast<SkeletalAppearance2*> (pDoc->getObject ()->getAppearance ());
	if (sa)
	{
		sa->incrementDetailLevel ();
		pDoc->updateAnimationView (this, true);
	}
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonLodReset(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CViewerDoc* pDoc = GetDocument();

	BOOL  enableIt = FALSE;
	BOOL  checkIt  = FALSE;

	DetailAppearance* da = dynamic_cast<DetailAppearance*> (pDoc->getObject ()->getAppearance ());
	if (da)
	{
		enableIt = TRUE;
		checkIt  = da->getUserControlled () ? TRUE : FALSE;
	}
	else
	{
		SkeletalAppearance2* sa = dynamic_cast<SkeletalAppearance2*> (pDoc->getObject ()->getAppearance ());
		if (sa)
		{
			enableIt = TRUE;
			checkIt  = sa->getUserControlledDetailLevel () ? TRUE : FALSE;
		}
	}

	pCmdUI->Enable (enableIt);
	pCmdUI->SetCheck (checkIt);
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonLodIncrease(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CViewerDoc* pDoc     = GetDocument();
	BOOL        enableIt = FALSE;

	DetailAppearance* da = dynamic_cast<DetailAppearance*> (pDoc->getObject ()->getAppearance ());
	if (da)
	{
		if (da->getUserControlled())
			enableIt = TRUE;
	}
	else
	{
		SkeletalAppearance2* sa = dynamic_cast<SkeletalAppearance2*> (pDoc->getObject ()->getAppearance ());
		if (sa)
			enableIt = TRUE;
	}

	pCmdUI->Enable (enableIt);
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonLodDecrease(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CViewerDoc* pDoc     = GetDocument();
	BOOL        enableIt = FALSE;


	DetailAppearance* da = dynamic_cast<DetailAppearance*> (pDoc->getObject ()->getAppearance ());
	if (da)
	{
		if (da->getUserControlled())
			enableIt = TRUE;
	}
	else
	{
		SkeletalAppearance2* sa = dynamic_cast<SkeletalAppearance2*> (pDoc->getObject ()->getAppearance ());
		if (sa)
			enableIt = TRUE;
	}

	pCmdUI->Enable (enableIt);
}

// ----------------------------------------------------------------------

BOOL CViewerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	if (ViewerPreferences::getCameraMode () == ViewerPreferences::CM_free)
	{
		camera->move_o (Vector::unitZ * (static_cast<float>(zDelta) / 120.0f));
	}
	else
		if (ViewerPreferences::getEnableWheelZoom ())
		{
			CViewerDoc* pDoc = GetDocument();
			if (keyDown (VK_MENU) && (!((nFlags & MK_LBUTTON) | (nFlags & MK_MBUTTON) || (nFlags & MK_RBUTTON))))
			{
				real zoomFraction = ViewerPreferences::getWheelZoomScale () * RECIP (1000);
				camera->move_o (Vector::unitZ * (static_cast<float>(zDelta) * zoomFraction) * pDoc->getObject ()->getAppearanceSphereRadius ());
			}
		}
	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonLightSetup() 
{
	// TODO: Add your command handler code here
	LightListDialog dlg (this, ViewerPreferences::getConstLightManager ());

	if (dlg.DoModal () == IDOK)
	{
		ViewerPreferences::getLightManager ().setLights (dlg.getLightDataList ());
		ViewerPreferences::reloadLights ();

		Invalidate ();
	}
}

// ----------------------------------------------------------------------


void CViewerView::OnButtonLightIcons() 
{
	// TODO: Add your command handler code here
	ViewerLightAppearance::setShowLights (!ViewerLightAppearance::getShowLights ());
}

void CViewerView::OnUpdateButtonLightIcons(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (ViewerLightAppearance::getShowLights ());	
}

void CViewerView::OnUpdateButtonLightSetup(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (ViewerPreferences::getLightMode () == ViewerPreferences::LM_userDefined);
}

void CViewerView::OnViewLightingoff() 
{
	ViewerPreferences::setLightMode (ViewerPreferences::LM_off);
	Invalidate ();
}

void CViewerView::OnUpdateViewLightingoff(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getLightMode () == ViewerPreferences::LM_off);
}

void CViewerView::OnView14ambientfulldirectional() 
{
	ViewerPreferences::setLightMode (ViewerPreferences::LM_quarterAmbientFullDirectional);
	Invalidate ();
}

void CViewerView::OnUpdateView14ambientfulldirectional(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getLightMode () == ViewerPreferences::LM_quarterAmbientFullDirectional);
}

void CViewerView::OnViewFulllighting() 
{
	ViewerPreferences::setLightMode (ViewerPreferences::LM_full);
	Invalidate ();
}

void CViewerView::OnUpdateViewFulllighting(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getLightMode () == ViewerPreferences::LM_full);
}

void CViewerView::OnViewLightinguserdefined() 
{
	ViewerPreferences::setLightMode (ViewerPreferences::LM_userDefined);
	Invalidate ();
}

void CViewerView::OnUpdateViewLightinguserdefined(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (ViewerPreferences::getLightMode () == ViewerPreferences::LM_userDefined);
}

void CViewerView::OnViewLightingShowHemisphericColors()
{
	ViewerPreferences::setShowHemisphericColors (!ViewerPreferences::getShowHemisphericColors());
	Invalidate ();
}

void CViewerView::OnUpdateViewLightingShowHemisphericColors(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getShowHemisphericColors ());
}

void CViewerView::OnViewRendersolid() 
{
	ViewerPreferences::setFillMode (GFM_solid);
	Invalidate ();
}

void CViewerView::OnUpdateViewRendersolid(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (ViewerPreferences::getFillMode () == GFM_solid);
}

void CViewerView::OnViewRendertextured() 
{
	ViewerPreferences::setEnableTextures (!ViewerPreferences::getEnableTextures ());
	Invalidate ();
}

void CViewerView::OnUpdateViewRendertextured(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck (ViewerPreferences::getEnableTextures ());
}

void CViewerView::OnViewRenderwire() 
{
	ViewerPreferences::setFillMode (GFM_wire);
	Invalidate ();
}

void CViewerView::OnUpdateViewRenderwire(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getFillMode () == GFM_wire);
}

void CViewerView::OnViewAnimatealpha() 
{
	m_animateAlpha = !m_animateAlpha;
	if (m_animateAlpha)
		m_animatedAlpha = -1.0f;

	Invalidate ();
}

void CViewerView::OnUpdateViewAnimatealpha(CCmdUI* pCmdUI) 
{
	CViewerDoc * const pDoc = dynamic_cast<CViewerDoc*> (GetDocument ());
	if (pDoc)
	{
		Object * const object = pDoc->getObject ();

		if (dynamic_cast<MeshAppearance *>(object->getAppearance()))
		{
			pCmdUI->Enable (true);
			pCmdUI->SetCheck (m_animateAlpha);
		}
		else
		{
			pCmdUI->Enable (false);
			pCmdUI->SetCheck (false);
		}
	}
}

void CViewerView::OnViewGlgraphicslayermetrics() 
{
	ViewerPreferences::setDisplayStatistics (!ViewerPreferences::getDisplayStatistics ());
}

void CViewerView::OnUpdateViewGlgraphicslayermetrics(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getDisplayStatistics ());	
}

void CViewerView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == 112)
	{
		GetMainFrame()->updateStatusBar("Reloading...");

		ViewerPreferences::unload();
		camera->setParentCell(cellProperty);
		CViewerDoc* pDoc = GetDocument();

		pDoc->reload ();

		ViewerPreferences::reload ();
		GetMainFrame()->updateStatusBar("Ready");

		// restart the alpha fading as well
		m_animatedAlpha = 0.0f;
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CViewerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (bActivate)
	{
		CViewerDoc* pDoc = GetDocument();
		pDoc->updateAnimationView (this);
		pDoc->updateAnimationTreeView(this);
		GetMainFrame()->updateHardpointTree(this);
	}
	
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

//-------------------------------------------------------------------

void CViewerView::OnViewShadowvolumes() 
{
	// TODO: Add your command handler code here
	ShadowManager::setEnabled (!ShadowManager::getEnabled ());	
//	OnView14ambientfulldirectional ();
}

//-------------------------------------------------------------------

void CViewerView::OnUpdateViewShadowvolumes(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (ShadowManager::getEnabled ());
}

//-------------------------------------------------------------------

void CViewerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// simply call parent.  we just needed to redefine access permissions
	// to this function.  the doc needs to update just this view in some
	// instances.
	CView::OnUpdate(pSender, lHint, pHint);
}

// ----------------------------------------------------------------------

void CViewerView::alter (const float elapsedTime)
{
	if (m_animatedAlpha < 0.0f)
		m_animatedAlpha = 0.0f;
	else
	{
		m_animatedAlpha += elapsedTime;
		while (m_animatedAlpha > 4.0f)
			m_animatedAlpha -= 4.0f;
	}

	Invalidate ();
}

// ----------------------------------------------------------------------

void CViewerView::setNoRender (bool const noRender)
{
	ms_noRender = noRender;
}

// ----------------------------------------------------------------------

void CViewerView::OnViewRendernpatches() 
{
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateViewRendernpatches(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonViewradarshape() 
{
	DetailAppearance::setDrawRadarShape (!DetailAppearance::getDrawRadarShape ());
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonViewradarshape(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (DetailAppearance::getDrawRadarShape ());
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonViewtestshape() 
{
	DetailAppearance::setDrawTestShape (!DetailAppearance::getDrawTestShape ());
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonViewtestshape(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (DetailAppearance::getDrawTestShape ());
}

// ----------------------------------------------------------------------

void CViewerView::OnButtonViewwriteshape() 
{
	DetailAppearance::setDrawWriteShape (!DetailAppearance::getDrawWriteShape ());
}

// ----------------------------------------------------------------------

void CViewerView::OnUpdateButtonViewwriteshape(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (DetailAppearance::getDrawWriteShape ());
}

// ======================================================================

void CViewerView::OnViewShowshadowextrudededges() 
{
	ShadowVolume::setShowExtrudedEdges (!ShadowVolume::getShowExtrudedEdges ());	
}

//===================================================================

void CViewerView::OnUpdateViewShowshadowextrudededges(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (ShadowVolume::getShowExtrudedEdges ());
}

//===================================================================

void CViewerView::OnViewShowshadownonmanifoldedges() 
{
	ShadowVolume::setShowNonManifoldEdges (!ShadowVolume::getShowNonManifoldEdges ());	
}

//===================================================================

void CViewerView::OnUpdateViewShowshadownonmanifoldedges(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (ShadowVolume::getShowNonManifoldEdges ());
}

//===================================================================

void CViewerView::OnViewAnimatelight() 
{
	ViewerPreferences::setAnimateLight (!ViewerPreferences::getAnimateLight ());
}

//===================================================================

void CViewerView::OnUpdateViewAnimatelight(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ViewerPreferences::getAnimateLight ());
}

//===================================================================

void CViewerView::OnEditUsetestshader() 
{
	ViewerPreferences::setUseOverrideShader (!ViewerPreferences::getUseOverrideShader ());
}

//===================================================================

void CViewerView::OnUpdateEditUsetestshader(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (ViewerPreferences::getUseOverrideShader ());
}

//===================================================================

void CViewerView::OnViewHardpoints() 
{
	ViewerPreferences::setShowHardpoints (!ViewerPreferences::getShowHardpoints ());
}

//===================================================================

void CViewerView::OnUpdateViewHardpoints(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (ViewerPreferences::getShowHardpoints ());
}

//===================================================================

void CViewerView::OnCollisionInfoDrawCMeshes() 
{
	ViewerPreferences::setShowCMeshes (!ViewerPreferences::getShowCMeshes ());
}

//===================================================================

void CViewerView::OnUpdateCollisionInfoDrawCMeshes(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (ViewerPreferences::getShowCMeshes ());
}

//===================================================================

void CViewerView::OnCollisionInfoDrawExtents() 
{
	ViewerPreferences::setShowColExtents (!ViewerPreferences::getShowColExtents ());
}

//===================================================================

void CViewerView::OnUpdateCollisionInfoDrawExtents(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (ViewerPreferences::getShowColExtents ());
}

//===================================================================

void CViewerView::OnCollisionInfoDrawFloors() 
{
	ViewerPreferences::setShowFloors (!ViewerPreferences::getShowFloors ());
}

//===================================================================

void CViewerView::OnUpdateCollisionInfoDrawFloors(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (ViewerPreferences::getShowFloors ());
}

//===================================================================

void CViewerView::OnCollisionInfoDrawPathNodes() 
{
	ViewerPreferences::setShowPathNodes (!ViewerPreferences::getShowPathNodes ());
}

//===================================================================

void CViewerView::OnUpdateCollisionInfoDrawPathNodes(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (true);
	pCmdUI->SetCheck (ViewerPreferences::getShowPathNodes ());
}

//===================================================================


