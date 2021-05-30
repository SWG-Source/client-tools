//======================================================================
//
// CuiWidget3dObjectListViewer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer_TextOverlay.h"

#include "clientGame/BuildingObject.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/FadingTextAppearance.h"
#include "clientGame/Game.h"
#include "clientGame/GlowAppearance.h"
#include "clientGame/InstallationObject.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/ShaderPrimitive.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/ObjectWatcherListCamera.h"
#include "clientObject/RibbonAppearance.h"
#include "clientObject/ShadowManager.h"
#include "clientSkeletalAnimation/OwnerProxyShader.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiLayerRenderer.h"
#include "clientUserInterface/CuiObjectDataSource.h"
#include "clientUserInterface/CuiObjectDataSourceCallback.h"
#include "clientUserInterface/CuiTextManager.h"
#include "clientUserInterface/CuiUtils.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectWatcherList.h"

#include "UICanvas.h"
#include "UIClock.h"
#include "UIDataSource.h"
#include "UIDeformer.h"
#include "UIMessage.h"
#include "UIRectangleStyle.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"

//======================================================================

namespace CuiWidget3dObjectListViewerNamespace
{
	float angleClamp2Pi (float a)
	{
		if (a < 0)
			return a + floorf (-a / PI_TIMES_2) * PI_TIMES_2 + PI_TIMES_2;

		if (a > PI_TIMES_2)
			return a - floorf (a / PI_TIMES_2) * PI_TIMES_2;

		return a;
	}

	float angleDiff2Pi( float a, float b )
	{
        float diff = a - b;

        if ( diff < -PI )
			diff += PI_TIMES_2;
        else if ( diff > PI )
			diff -= PI_TIMES_2;

        return diff;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool s_installed     = false;
	bool s_debugRender   = false;
	bool s_debugCamera   = false;
	bool s_debugControls = false;
	bool s_debugFitRect  = false;

	//----------------------------------------------------------------------

	const int internal_object_count = 3;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	float computeScale (const Object & obj, float scaleMaxOverride)
	{
		const SharedObjectTemplate * const tmpl = dynamic_cast<const SharedObjectTemplate *>(obj.getObjectTemplate ());

		if (tmpl)
		{
			const float scaleMax = (scaleMaxOverride != 0.0f) ? scaleMaxOverride : tmpl->getScaleMax ();

			DEBUG_WARNING (scaleMax == 0.0f, ("SharedObjectTemplate [%s] scalemax is set to 0.0f, this is probably not what you want.", tmpl->getName ()));
			const float objectScale = obj.getScale ().y;
			return 1.0f + (scaleMax - objectScale);
		}

		return 1.0f;
	}

	//----------------------------------------------------------------------


	bool isBoneDescendedFrom (const Skeleton & skeleton, const int root, int child)
	{
		if (root == child)
			return true;

		for ( ; ; )
		{
			const int parent = skeleton.getParentTransformIndex (child);
			if (parent < 0)
				break;

			if (parent == root)
				return true;

			child = parent;
		}

		return false;
	}

	//----------------------------------------------------------------------

	class CompareFirst
	{
	public:
		CompareFirst() : m_obj(NULL) {}
		explicit CompareFirst (Object & obj) : m_obj (&obj) {}

		bool operator() (const CuiWidget3dObjectListViewer::ObjectPair & rhs) const
		{
			return rhs.first.operator==(m_obj);
		}

	private:
		Object * m_obj;
	};

	//----------------------------------------------------------------------

	void transformBox (BoxExtent & box, const Transform & transform)
	{
		static Vector vs[8];
		box.getCornerVectors (vs);

		Vector tMin (Vector::maxXYZ);
		Vector tMax (Vector::negativeMaxXYZ);

		for (int i = 0; i < 8; ++i)
		{
			Vector & v = vs [i];
			v = transform.rotateTranslate_l2p (v);

			tMin.x = std::min (v.x, tMin.x);
			tMin.y = std::min (v.y, tMin.y);
			tMin.z = std::min (v.z, tMin.z);

			tMax.x = std::max (v.x, tMax.x);
			tMax.y = std::max (v.y, tMax.y);
			tMax.z = std::max (v.z, tMax.z);
		}

		box.setMin (tMin);
		box.setMax (tMax);
	}

	//----------------------------------------------------------------------

	ShaderTemplate const * ms_currentOverrideShaderTemplate = 0;
	typedef std::map<std::pair<ShaderTemplate const *, Texture const *>, StaticShader *> OverrideShaderMap;
	OverrideShaderMap ms_overrideShaderMap;

	//----------------------------------------------------------------------

	StaticShader const & prepareToViewOverrideFunction(ShaderPrimitive const & shaderPrimitive)
	{
		StaticShader const & shaderPrimitiveShader = shaderPrimitive.prepareToView();

		if (ms_currentOverrideShaderTemplate)
		{
			//-- Extract MAIN from the existing shader primitive and set it in the override shader
			Tag const TAG_MAIN = TAG(M,A,I,N);
			Texture const * mainTexture = 0;
			if (shaderPrimitiveShader.getTexture(TAG_MAIN, mainTexture))
			{
				OverrideShaderMap::iterator iter = ms_overrideShaderMap.find(std::make_pair(ms_currentOverrideShaderTemplate, mainTexture));
				if (iter != ms_overrideShaderMap.end())
					return *iter->second;

				Shader * const shader = ms_currentOverrideShaderTemplate->fetchModifiableShader();
				StaticShader * const staticShader = shader->getStaticShader();
				if (staticShader)
				{
					mainTexture->fetch();
					staticShader->setTexture(TAG_MAIN, *mainTexture);
					IGNORE_RETURN(ms_overrideShaderMap.insert(std::make_pair(std::make_pair(ms_currentOverrideShaderTemplate, mainTexture), staticShader)));

					return staticShader->prepareToView();
				}
				else
					shader->release();
			}
		}

		return shaderPrimitiveShader;
	}

	//----------------------------------------------------------------------

	void clearOverrideShaderMap()
	{
		for (OverrideShaderMap::iterator iter = ms_overrideShaderMap.begin(); iter != ms_overrideShaderMap.end(); ++iter)
		{
			iter->first.second->release();
			iter->second->release();
		}

		ms_overrideShaderMap.clear();
	}

	//----------------------------------------------------------------------

	float s_nearPlaneMinimum = 0.01f;
	float s_nearPlaneMaximum = 2.00f;
	float s_farPlaneMaximum = 16384.0f;
	float s_nearPlaneDefault = 0.1f;
	float s_farPlaneDefault = 1024.0f;

	float const ZOOM_THRESHOLD  = 0.001f;
	float const LOOKAT_THRESHOLD_SQUARED  = 0.00001f;
	float const LOOKAT_CHANGERATE = 2.0f;

	float const s_nameUpdateRate = 2.0f;
}

//----------------------------------------------------------------------

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (CuiWidget3dObjectListViewer, true, 0, 0, 0);

//----------------------------------------------------------------------

using namespace CuiWidget3dObjectListViewerNamespace;


class CuiWidget3dObjectListViewer::MyObjectDataSourceCallback :
public CuiObjectDataSourceCallback
{
public:
	void addObject(Object & obj)
	{
		m_viewer.addObject(obj);
	}

	void removeObject(Object & obj)
	{
		m_viewer.removeObject(obj);
	}

	~MyObjectDataSourceCallback () {};

	explicit MyObjectDataSourceCallback(CuiWidget3dObjectListViewer & viewer) : CuiObjectDataSourceCallback (), m_viewer (viewer) {}

private:
	CuiWidget3dObjectListViewer & m_viewer;

	MyObjectDataSourceCallback (const MyObjectDataSourceCallback & rhs);
	MyObjectDataSourceCallback & operator= (const MyObjectDataSourceCallback & rhs);
};

//----------------------------------------------------------------------

struct CuiWidget3dObjectListViewer::Text3d
{
	Vector m_location;
	UIString m_text;
	UIColor m_color;
	float m_backgroundOpacity;
};

//----------------------------------------------------------------------

const char * const CuiWidget3dObjectListViewer::TypeName = "CuiWidget3dObjectListViewer";

const UILowerString CuiWidget3dObjectListViewer::PropertyName::AutoZoomOutOnly        = UILowerString ("AutoZoomOutOnly");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraAutoZoom         = UILowerString ("CameraAutoZoom");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraCompensateScale  = UILowerString ("CameraCompensateScale");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraFitBone          = UILowerString ("CameraFitBone");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraForceTarget      = UILowerString ("CameraForceTarget");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraLodBias          = UILowerString ("CameraLodBias");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraLodBiasOverride  = UILowerString ("CameraLodBiasOverride");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraLookAt           = UILowerString ("CameraLookAt");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraLookAtBone       = UILowerString ("CameraLookAtBone");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraLookAtCenter     = UILowerString ("CameraLookAtCenter");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraPitch            = UILowerString ("CameraPitch");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraRoll             = UILowerString ("CameraRoll");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraTransformToObj   = UILowerString ("CameraTransformToObj");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::CameraYaw              = UILowerString ("CameraYaw");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::DragPitchMax           = UILowerString ("DragPitchMax");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::DragPitchMin           = UILowerString ("DragPitchMin");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::DragPitchOk            = UILowerString ("DragPitchOk");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::DragYawOk              = UILowerString ("DragYawOk");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::DrawName               = UILowerString ("DrawName");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::FieldOfView            = UILowerString ("FieldOfView");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::FitDistanceFactor      = UILowerString ("FitDistanceFactor");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::FitRect                = UILowerString ("FitRect");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::LightAmbientColor      = UILowerString ("LightAmbientColor");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::LightColor             = UILowerString ("LightColor");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::LightColor2            = UILowerString ("LightColor2");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::LightLockToCamera      = UILowerString ("LightLockToCamera");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::LightPitch             = UILowerString ("LightPitch");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::LightPitch2            = UILowerString ("LightPitch2");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::LightYaw               = UILowerString ("LightYaw");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::LightYaw2              = UILowerString ("LightYaw2");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::LookAtBoneOnlyY        = UILowerString ("LookAtBoneOnlyY");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::ObjectDataSource       = UILowerString ("ObjectDataSource");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::OverrideShader         = UILowerString ("OverrideShader");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::OverrideShaderTexture  = UILowerString ("OverrideShaderTexture");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::Paused                 = UILowerString ("Paused");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::RenderObjectEffects    = UILowerString ("RenderObjectEffects");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::RStyleOverlay          = UILowerString ("RStyleOverlay");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::RStyleOverlayColor     = UILowerString ("RStyleOverlayColor");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::RStyleOverlayOpacity   = UILowerString ("RStyleOverlayOpacity");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::Rotation               = UILowerString ("RotateSpeed");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::ScaleMaxOverride       = UILowerString ("ScaleMaxOverride");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::Shadows                = UILowerString ("Shadows");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::TextStyleBottom        = UILowerString ("TextStyleBottom");
const UILowerString CuiWidget3dObjectListViewer::PropertyName::TextStyleTop           = UILowerString ("TextStyleTop");

const UILowerString CuiWidget3dObjectListViewer::MethodName::SetObject = UILowerString("SetObject");
const UILowerString CuiWidget3dObjectListViewer::MethodName::ClearObjects = UILowerString("ClearObjects");

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer::CuiWidget3dObjectListViewer () :
UIWidget                    (),
UINotification              (),
m_camera                    (new ObjectWatcherListCamera),
m_cameraLookAt              (),
m_cameraLookAtTarget        (),
m_cameraYawPitchZoom        (PI, 0.0f, 0.0f),
m_cameraRoll                (0.0f),
m_cameraYawPitchZoomLast    (PI, 0.0f, 0.0f),
m_cameraYawPitchZoomTarget  (PI, 0.0f, 0.0f),
m_objectVector              (new ObjectRenderVector),
m_objectWatcherList         (new ObjectWatcherList),
m_light                     (new Light (Light::T_parallel, VectorArgb::solidWhite)),
m_light2                    (new Light (Light::T_parallel, VectorArgb::solidWhite)),
m_lightAmbient              (new Light (Light::T_ambient,  VectorArgb::solidWhite)),
m_lightPitch                (0.4f),
m_lightPitch2               (0.4f),
m_lightYaw                  (0.5f),
m_lightYaw2                 (-2.0f),
m_lightAmbientColor         (1.0f, 0.13f, 0.13f, 0.13f),
m_lightColor                (1.0f, 1.0f, 1.0f, 1.0f),
m_lightColor2               (1.0f, 1.0f, 1.0f, 1.0f),
m_flags                     (F_alterObjects | F_paused | F_dragYawOk | F_cameraLookAtBoneDirty | F_cameraInterpolate | F_lookAtBoneOnlyY | F_lightLockToCamera | F_cameraLodBiasOverride | F_cameraAutoZoom | F_cameraZoomInWhileTurn | F_useOverrideShaderTexture),
m_lastMousePoint            (),
m_fieldOfView               (PI_OVER_8), // 22.5 degrees default
m_viewport                  (),
m_fitRect                   (0, 0, 100, 100),
m_cameraLookAtBone          (),
m_cameraZoomLookAtBone      (),
m_zoomBoneInterpFactor      (0.0f),
m_cameraDeflectionAngle     (),
m_cameraLookAtBoneOk        (false),
m_rotateSpeed               (0.0f),
m_lastDragTime              (0),
m_scaleMaxOverride          (0.0f),
m_viewDirtyFrame            (0),
m_overrideShaderTemplate    (0),
m_fitDistanceFactor         (1.0f),
m_objectDataSource          (0),
m_objectDataSourceCallback  (0),
m_overlay                   (0),
m_overlayColor              (UIColor::white),
m_overlayOpacity            (1.0f),
m_margin                    (4L,4L,4L,4L),
m_dragPitchMax              (PI_OVER_3),
m_dragPitchMin              (-PI_OVER_3),
m_dragPitchMinActual        (-PI_OVER_3),
m_lastViewportSize          (),
m_cameraLodBias             (1.0f),
m_numCreatures              (0),
m_zoomInterpolationRate     (2.0f),
m_lastNameUpdateSecs        (0.0f),
m_maximumDesiredDetailLevel (3),
m_text3d                    (0),
m_renderObjectEffects       (false),
m_fitScale                  (),
m_rotationSlowsToStop       (false),
m_forceDefaultClippingPlanes(false),
m_ignoreMouseWheel          (false),
m_noCameraClamp				(false)
{
	// @todo I would have made install and remove members, but I didn't know where to call them
	// from (there's no SetupClientUserInterface) so john should figure out where to put this -ALS
//	install ();

	for (int i = 0; i < TOT_numTextOverlays; ++i)
	{
		m_textOverlays [i] = 0;
	}

	m_camera->setNearPlane (s_nearPlaneDefault);
	m_camera->setFarPlane  (s_farPlaneDefault);
	m_objectWatcherList->setSkipCellRegionOfInfluence(true);
	m_camera->addObjectWatcherList (*m_objectWatcherList);

	m_light->setDiffuseColor        (m_lightColor);
	m_light->setSpecularColor       (m_lightColor);
	m_light->setSpecularColorScale  (0.50f);

	m_light2->setDiffuseColor       (m_lightColor2);
	m_light2->setSpecularColor      (m_lightColor2);
	m_light2->setSpecularColorScale (0.50f);

	m_lightAmbient->setDiffuseColor (m_lightAmbientColor);
	m_objectWatcherList->addObject  (*m_light);
	m_objectWatcherList->addObject  (*m_light2);
	m_objectWatcherList->addObject  (*m_lightAmbient);

	m_objectDataSourceCallback = new MyObjectDataSourceCallback (*this);
}

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer::~CuiWidget3dObjectListViewer ()
{
	setObjectDataSource (0);
	setOverlay          (0);

	m_objectWatcherList->removeObject (*m_light);
	delete m_light;
	m_light = 0;

	m_objectWatcherList->removeObject (*m_light2);
	delete m_light2;
	m_light2 = 0;

	m_objectWatcherList->removeObject (*m_lightAmbient);
	delete m_lightAmbient;
	m_lightAmbient = 0;

	m_objectWatcherList->removeAll (false);
	m_camera->removeObjectWatcherList (*m_objectWatcherList);

	delete m_objectVector;
	delete m_objectWatcherList;
	delete m_camera;

	m_objectVector  = 0;
	m_objectWatcherList    = 0;
	m_camera        = 0;

	clearOverrideShaderMap();

	if (m_overrideShaderTemplate)
	{
		m_overrideShaderTemplate->release();
		m_overrideShaderTemplate = 0;
	}

	delete m_objectDataSource;
	m_objectDataSource = 0;

	delete m_objectDataSourceCallback;
	m_objectDataSourceCallback = 0;

	for (int i = 0; i < TOT_numTextOverlays; ++i)
	{
		delete m_textOverlays [i];
		m_textOverlays [i] = 0;
	}

	if (m_text3d != 0)
	{
		Text3dVector::iterator ii = m_text3d->begin();
		Text3dVector::iterator iiEnd = m_text3d->end();
		for (; ii != iiEnd; ++ii)
		{
			Text3d * const text3d = *ii;
			delete text3d;
		}

		delete m_text3d;
		m_text3d = 0;
	}

	m_overlay = 0;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::Destroy()
{
	setPaused (true);
	UIWidget::Destroy ();
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::remove ()
{
	if (s_installed)
	{
		DebugFlags::unregisterFlag(s_debugFitRect);
		DebugFlags::unregisterFlag(s_debugRender);
		DebugFlags::unregisterFlag(s_debugCamera);
		DebugFlags::unregisterFlag(s_debugControls);

		removeMemoryBlockManager ();

		s_installed = false;
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::install ()
{
	if (!s_installed)
	{
		s_installed = true;

		DebugFlags::registerFlag(s_debugFitRect,  "ClientUserInterface", "CuiWidget3dObjectListViewerFitRect");
		DebugFlags::registerFlag(s_debugRender,   "ClientUserInterface", "CuiWidget3dObjectListViewer");
		DebugFlags::registerFlag(s_debugCamera,   "ClientUserInterface", "CuiWidget3dObjectListViewerCamera");
		DebugFlags::registerFlag(s_debugControls, "ClientUserInterface", "CuiWidget3dObjectListViewerDebugControls");

		installMemoryBlockManager ();
		ExitChain::add(remove, "CuiWidget3dObjectListViewer::remove");
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::Render( UICanvas & canvas) const
{
	if (boolEqual(m_renderObjectEffects, false))
	{
		Appearance::enableRenderEffects(false);
	}

	bool const useClip = ShaderPrimitiveSorter::getUseClipRectangle();
	ShaderPrimitiveSorter::setUseClipRectangle(false);
	RenderStart   (canvas);
	RenderStop    ();
	ShaderPrimitiveSorter::setUseClipRectangle(useClip);

	if (boolEqual(m_renderObjectEffects, false))
	{
		Appearance::enableRenderEffects(true);
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::RenderStart   (UICanvas & canvas) const
{
	UIWidget::Render (canvas);

	CuiLayerRenderer::flushRenderQueue (); //-- do not use UICanvas::flush (), it is ignored by the Cui

	UIPoint canvasTranslation(canvas.GetTranslation());
	UISize const widgetSize(GetSize());
	UIFloatPoint const widgetCenter(widgetSize / 2);

	bool renderCameraScene = true;

	if (canvas.IsDeforming())
	{
		const_cast<CuiWidget3dObjectListViewer *>(this)->setFlags(F_sizeChanged);
		
		UIFloatPoint oldTranslation(UIFloatPoint(canvasTranslation) + widgetCenter);
		UIFloatPoint newTranslation;
		canvas.Deform(&oldTranslation, &newTranslation, 1);

		UIFloatPoint widgetScale;
		canvas.GetDeformScale(widgetScale);
		m_fitScale.set(widgetScale.x, widgetScale.y, 1.0f);

		canvasTranslation.Set(static_cast<UIScalar>(newTranslation.x - widgetCenter.x),
								static_cast<UIScalar>(newTranslation.y - widgetCenter.y));

		// Don't render the 3D objects until the viewport is 50% of its original size.
		renderCameraScene = (m_fitScale.x > 0.5f) && (m_fitScale.y > 0.5f);
	}
	else
	{
		m_fitScale = Vector::xyz111;
	}

	m_viewport.Set(UIPoint::zero, widgetSize);

	UIRect clip;
	canvas.GetClip (clip);


	if (!UIUtils::ClipRect (m_viewport, clip))
		return;

	DEBUG_FATAL (m_viewport.Width () <= 0 || m_viewport.Height () <= 0, ("cliprect fubar'd\n"));

	m_viewport += canvasTranslation;

	if (!UIUtils::ClipRect (m_viewport, UIRect (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ())))
		return;

	DEBUG_FATAL (m_viewport.Width () <= 0 || m_viewport.Height () <= 0, ("cliprect fubar'd\n"));

	const UISize & viewportSize = m_viewport.Size ();

	if (viewportSize != m_lastViewportSize)
	{
		const_cast<CuiWidget3dObjectListViewer *>(this)->setFlags (F_sizeChanged);
		m_lastViewportSize = viewportSize;
	}

	if (canvas.IsDeforming())
	{
		UIScalar const viewportWidth = m_viewport.Width();
		UIScalar const viewportHeight = m_viewport.Height();

		UIScalar const diffx = clamp(static_cast<UIScalar>(1), static_cast<UIScalar>(widgetCenter.x * (1.0f - m_fitScale.x)), static_cast<UIScalar>(viewportWidth - 1));
		UIScalar const diffy = clamp(static_cast<UIScalar>(1), static_cast<UIScalar>(widgetCenter.y * (1.0f - m_fitScale.y)), static_cast<UIScalar>(viewportHeight - 1));

		m_camera->setViewport(m_viewport.left + diffx, m_viewport.top + diffy, viewportWidth - diffx, viewportHeight - diffy);
	}
	else
	{
		m_camera->setViewport(m_viewport.left, m_viewport.top, m_viewport.Width(), m_viewport.Height());
	}

	UISize visibleSize(m_viewport.Size());


	float horizontalFOV = m_fieldOfView;

//	if (widgetSize.x != visibleSize.x)
//		horizontalFOV = horizontalFOV * static_cast<float>(visibleSize.x) / static_cast<float>(widgetSize.x);

	if (visibleSize.x < visibleSize.y)
	{
		horizontalFOV *= static_cast<float>(visibleSize.x) / static_cast<float>(visibleSize.y);
	}

	m_camera->setHorizontalFieldOfView (horizontalFOV);

	/*
	if (!hasFlags (F_viewDirty))
	{
		if (!checkAppearancesReady ())
		{
			const_cast<CuiWidget3dObjectListViewer *>(this)->setViewDirty (true);
			const_cast<CuiWidget3dObjectListViewer *>(this)->setCameraLookAtBoneDirty (true);
		}
	}
	*/

	visibleSize.x = std::max(static_cast<UIScalar>(1), visibleSize.x);
	visibleSize.y = std::max(static_cast<UIScalar>(1), visibleSize.y);

	if (hasFlags (F_viewDirty))
		const_cast<CuiWidget3dObjectListViewer*>(this)->recomputeZoom ();

	gotoCameraPivotPoint (false);

	m_camera->move_o    (Vector (0.0f, 0.0f, -m_cameraYawPitchZoom.z));
	m_camera->yaw_o     (m_cameraDeflectionAngle.x);
	m_camera->pitch_o   (m_cameraDeflectionAngle.y);

	m_light->resetRotate_o2p ();
	m_light2->resetRotate_o2p ();

	if (hasFlags (F_lightLockToCamera))
	{
		m_light->setTransform_o2p (m_camera->getTransform_o2p ());
		m_light2->setTransform_o2p (m_camera->getTransform_o2p ());
	}

	m_light->yaw_o   (m_lightYaw);
	m_light->pitch_o (m_lightPitch);

	m_light2->yaw_o   (m_lightYaw2);
	m_light2->pitch_o (m_lightPitch2);

	Graphics::clearViewport (false, 0, true, 1.0f, true, 0);

	const GlFillMode oldFillMode = Graphics::getFillMode();

	if (hasFlags (F_wireFrame))
		Graphics::setFillMode (GFM_wire);

	const bool wasShadowed = ShadowManager::getEnabled ();
	ShadowManager::setEnabled (hasFlags (F_useShadows));

	const float old_lodBiasDetail   = DetailAppearance::getDetailLevelBias ();
	const float old_lodBiasSkeletal = SkeletalAppearance2::getDetailLevelBias ();

	//-- default lod bias factor is based on a 1000 pixel wide viewport,
	//-- with a 60 degree (pi/6) FOV

	float viewerLodBias                     = m_cameraLodBias;//sqr (viewerLodBiasFactor / defaultLodBiasFactor);

	if (!hasFlags (F_cameraLodBiasOverride))
		viewerLodBias *= old_lodBiasDetail;

	//-- @todo: detail level biasing doesn't work right multiple times per frame on a single object

	DetailAppearance::setDetailLevelBias    (viewerLodBias);
	SkeletalAppearance2::setDetailLevelBias (viewerLodBias);
	bool skeletalMaximumDesiredLodEnabled;
	int  skeletalMaximumLodIndex;

	SkeletalAppearance2::getMaximumDesiredDetailLevel(skeletalMaximumDesiredLodEnabled, skeletalMaximumLodIndex);
	SkeletalAppearance2::setMaximumDesiredDetailLevel(true, m_maximumDesiredDetailLevel);

	//-- Tell skeletal appearance to ignore lod manager settings and to prevent future ui-enabled renderings from
	//   stomping on internal world state settings.
	bool const oldUiContextEnabled = SkeletalAppearance2::getUiContextEnabled();
	SkeletalAppearance2::setUiContextEnabled(Game::getScene() != 0);

	if (m_numCreatures == 0)
		OwnerProxyShader::setEnabled (false);

	//-- HACK
	//-- HACK: exclude player ship cockpit
	//-- HACK
	
	bool cockpitWasActive = true;
	Object * cockpit = NULL;
	
	{
		for (int i = 0; i < m_objectWatcherList->getNumberOfObjects(); ++i)
		{
			Object * const object = m_objectWatcherList->getObject(i);
			if (NULL != object)
			{
				ClientObject * const clientObject = object->asClientObject();
				if (NULL != clientObject && NULL != clientObject->asShipObject())
				{
					if (clientObject->asShipObject())
					{
						const int numChildren = object->getNumberOfChildObjects ();
						
						for (int j = 0; j < numChildren; ++j)
						{
							Object * const child = object->getChildObject (j);
							//-- HACK: this debugName is set in CockpitCamera
							if (NULL != child && NULL != child->getDebugName() && !strcmp(child->getDebugName(), "ShipCockpitFrame"))
							{
								cockpit = child;
								cockpitWasActive = cockpit->isActive();
								cockpit->setActive(false);
								break;
							}
						}
					}
				}
			}
		}
	}

	//--
	//-- Perform any override shader special rendering first
	//--

	if (m_overrideShaderTemplate)
	{
		if (!hasFlags(F_useOverrideShaderTexture) && renderCameraScene)
			m_camera->renderScene();

		ms_currentOverrideShaderTemplate = m_overrideShaderTemplate;
		ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(prepareToViewOverrideFunction);
	}

	const bool fadingTextAppearanceEnabled = FadingTextAppearance::isEnabled ();
	FadingTextAppearance::setEnabled (false);

	//--
	//-- Finally perform the actual render
	//--

	if (renderCameraScene)
	{
		m_camera->renderScene ();
	}

	//--
	//-- Rendering complete, now restore the pre-render states
	//--

	if (NULL != cockpit)
		cockpit->setActive(cockpitWasActive);

	FadingTextAppearance::setEnabled (fadingTextAppearanceEnabled);

	if (m_overrideShaderTemplate)
	{
		ms_currentOverrideShaderTemplate = 0;
		ShaderPrimitiveSorter::setPrepareToViewOverrideFunction(0);
	}

	OwnerProxyShader::setEnabled (true);

	//-- Revert skeletal appearance to previous ui state.
	SkeletalAppearance2::setUiContextEnabled(oldUiContextEnabled);

	ShadowManager::setEnabled (wasShadowed);
	DetailAppearance::setDetailLevelBias    (old_lodBiasDetail);
	SkeletalAppearance2::setDetailLevelBias (old_lodBiasSkeletal);
	SkeletalAppearance2::setMaximumDesiredDetailLevel(skeletalMaximumDesiredLodEnabled, skeletalMaximumLodIndex);

	if (hasFlags (F_wireFrame))
		Graphics::setFillMode (oldFillMode);

	if (s_debugRender || s_debugFitRect)
		debugRender ();
}

void CuiWidget3dObjectListViewer::RenderText(UICanvas & canvas) const
{
	const UISize & size = GetSize ();
	for (int i = 0; i < TOT_numTextOverlays; ++i)
	{
		TextOverlay * const to = m_textOverlays [i];
		if (to)
		{
			to->render (size, canvas, static_cast<TextOverlayTypes>(i));
		}
	}

	if (m_text3d != 0)
	{
		Text3dVector::iterator ii = m_text3d->begin();
		Text3dVector::iterator iiEnd = m_text3d->end();
		for (; ii != iiEnd; ++ii)
		{
			Text3d const * const text3d = *ii;

			if (text3d != 0)
			{
				Vector screen2d;

				if (m_camera->projectInWorldSpace(text3d->m_location, &screen2d.x, &screen2d.y, &screen2d.z, false))
				{
					const UIPoint gpt = m_viewport.Location ();
					screen2d.x -= gpt.x;
					screen2d.y -= gpt.y;

					UIPoint location2d(static_cast<int>(screen2d.x), static_cast<int>(screen2d.y));

					UIText uiText3d;
					uiText3d.SetTextColor(text3d->m_color);
					uiText3d.SetBackgroundOpacity(text3d->m_backgroundOpacity);
					uiText3d.SetMargin(UIRect (4,4,4,4));
					uiText3d.SetStyle(CuiTextManager::getTextStyle(4));
					uiText3d.SetLocalText(text3d->m_text);
					uiText3d.SetTextAlignment (UITextStyle::Center);
					uiText3d.SizeToContent();

					location2d.x -= uiText3d.GetWidth() / 2;

					canvas.PushState();
					canvas.ModifyOpacity(1.0f);
					canvas.Translate(location2d);
					uiText3d.Render(canvas);
					canvas.PopState();
				}
			}
		}
	}

	if (m_overlay)
	{
		const UIColor preBackgroundColor (canvas.GetColor ());
		const float OldOpacity   = canvas.GetOpacity();

		canvas.ModifyColor   (m_overlayColor);
		canvas.ModifyOpacity (m_overlayOpacity);

		UISize scrollExtent;
		GetScrollExtent   (scrollExtent);
		m_overlay->Render (UITime(GetAnimationState()), canvas, scrollExtent);

		canvas.SetColor   (preBackgroundColor);
		canvas.SetOpacity (OldOpacity);
	}

}
//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::computeTransformedBoxExtent (const Object & obj, BoxExtent & boxExtent, bool const debugRender) const
{
	//-- For non-skeletal appearances, we are going to use Object::getTangibleExtent
	if ((obj.getAppearance() != 0) && (obj.getAppearance()->asSkeletalAppearance2() == 0))
	{
		Extent const * const extent = obj.getAppearance()->getExtent();
		AxialBox axialBox = (extent != 0) ? extent->getBoundingBox() : obj.getTangibleExtent();

		//-- Transform box from child space into parent space
		if (!axialBox.isEmpty() && !hasFlags(F_cameraTransformToObj))
		{
			//-- Grow axial box in parent space
			Transform const & transform = obj.getTransform_o2w();

			AxialBox result;
			for (int j = 0; j < 8; ++j)
				result.add(transform.rotateTranslate_l2p(axialBox.getCorner(j)));

			axialBox = result;
		}

		boxExtent.setBox(axialBox);

		return;
	}

	typedef std::pair<const Object *, Transform> ObjectInfo;


	typedef stdvector<ObjectInfo>::fwd ObjectVector;
	static ObjectVector ov;
	ov.clear ();

	Transform startingTransform;

	if (hasFlags (F_cameraTransformToObj))
		startingTransform = Transform::identity;
	else
		startingTransform = obj.getTransform_o2w ();

	ov.push_back (ObjectInfo (&obj, startingTransform));

	const ClientObject * const clientObject = obj.asClientObject ();
	const CreatureObject * const creature   = clientObject ? clientObject->asCreatureObject () : 0;

	bool root = true;

	while (!ov.empty ())
	{
		const Object * const parent   = ov.back ().first;
		const Transform  transform    = ov.back ().second;

		ov.pop_back ();

		if (!parent)
			continue;

		const int numChildren = parent->getNumberOfChildObjects ();

		for (int i = 0; i < numChildren; ++i)
		{
			const Object * const child = parent->getChildObject (i);

			if (!creature || child->asClientObject ())
			{
				Transform t;
				t.multiply (transform, child->getTransform_o2p ());
				ov.push_back (ObjectInfo (child, t));
			}
		}

		//----------------------------------------------------------------------
		Appearance const * const app = parent->getAppearance ();
		if (!app)
			continue;

		BoxExtent const * theBox = 0;
		BoxExtent skeletalBox;

		SkeletalAppearance2 const * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			if (hasFlags (F_cameraFitBone) && !m_cameraLookAtBone.empty ())
			{
				if (computeSkeletalBoundingBox (*skelApp, skeletalBox))
					theBox = &skeletalBox;
			}
			else
			{
				skeletalBox.copy (&const_cast<SkeletalAppearance2 *>(skelApp)->getExactMeshExtent ());
				theBox = &skeletalBox;
			}
		}
		else
		{
			if (root ||
				(dynamic_cast<const FadingTextAppearance *>(app) == 0 &&
				 dynamic_cast<const RibbonAppearance *>(app) == 0 &&
				 app->asParticleEffectAppearance() == 0 &&
				 app->asSwooshAppearance() == 0 &&
				 dynamic_cast<GlowAppearance const *>(app) == 0))
			{
				Extent const * const extent = app->getExtent ();
				theBox = dynamic_cast<const BoxExtent *>(extent);
			}
		}

		if (theBox)
		{
			BoxExtent tmpBox (theBox->getBox ());
			transformBox     (tmpBox, transform);
			boxExtent.grow   (tmpBox);

			if (debugRender)
			{
				CuiLayerRenderer::flushRenderQueue (); //-- do not use UICanvas::flush (), it is ignored by the Cui

				if (hasFlags (F_cameraTransformToObj))
					Graphics::setObjectToWorldTransformAndScale (obj.getTransform_o2w (),    Vector::xyz111);
				else
					Graphics::setObjectToWorldTransformAndScale (Transform::identity,    Vector::xyz111);

				if (root)
					Graphics::drawExtent                        (&tmpBox,                VectorArgb::solidMagenta);
				else
					Graphics::drawExtent                        (&tmpBox,                VectorArgb::solidRed);
			}
		}

		root = false;
	}

	boxExtent.calculateCenterAndRadius ();
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::debugRender () const
{
	CuiLayerRenderer::flushRenderQueue (); //-- do not use UICanvas::flush (), it is ignored by the Cui

	///-- debugging render stuff
	Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());

	if (s_debugRender)
	{
		Graphics::setObjectToWorldTransformAndScale (Transform::identity, Vector::xyz111);
		Graphics::drawLine (Vector::zero, Vector::unitX,  VectorArgb::solidRed);
		Graphics::drawLine (Vector::zero, Vector::unitY,  VectorArgb::solidGreen);
		Graphics::drawLine (Vector::zero, Vector::unitZ,  VectorArgb::solidBlue);

		const int count = m_objectWatcherList->getNumberOfObjects();

		for (int i = 0; i < count; ++i)
		{
			const Object * const object = m_objectWatcherList->getObject(i);

			if (object != 0)
			{
				BoxExtent boxExtent;
				computeTransformedBoxExtent(*object, boxExtent, true);
			}
		}
	}

	if (s_debugFitRect)
	{
		UIFloatPoint fitRect_size;
		fitRect_size.x = static_cast<float>((m_fitRect.right  - m_fitRect.left) * GetWidth  ()) * 0.01f;
		fitRect_size.y = static_cast<float>((m_fitRect.bottom - m_fitRect.top)  * GetHeight ()) * 0.01f;

		UIFloatPoint fitRectOffset;
		fitRectOffset.x = static_cast<float>(m_fitRect.left * GetWidth  ()) * 0.01f;
		fitRectOffset.y = static_cast<float>(m_fitRect.top  * GetHeight ()) * 0.01f;

		const UIPoint & worldLoc = GetWorldLocation ();
		const UIFloatPoint fWorldLoc = UIFloatPoint (static_cast<float>(worldLoc.x), static_cast<float>(worldLoc.y)) + fitRectOffset;

		Graphics::drawRectangle (fWorldLoc.x, fWorldLoc.y, fWorldLoc.x + fitRect_size.x, fWorldLoc.y + fitRect_size.y, VectorArgb::solidWhite);
	}
}

//----------------------------------------------------------------------

void  CuiWidget3dObjectListViewer::RenderStop    () const
{
	Graphics::setViewport (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());
}

//----------------------------------------------------------------------

bool CuiWidget3dObjectListViewer::ProcessMessage( const UIMessage & msg)
{
	if( UIWidget::ProcessMessage( msg ) )
		return true;

	if (msg.Type == UIMessage::MiddleMouseDown)
	{
		setFlags (F_draggingDebug);
		m_lastMousePoint = msg.MouseCoords;
		return true;
	}
	else if (msg.Type == UIMessage::MiddleMouseUp)
	{
		unsetFlags (F_draggingDebug);
		return false;
	}
	if (msg.Type == UIMessage::MouseWheel)
	{
		if (m_ignoreMouseWheel)
			return true;

		float val = getFitDistanceFactor() - 0.1f * msg.Data;
		if(val < 0.1f) 
			val = 0.1f;
		if(val > 5.0f)
			val = 5.0f;
		else
		{		
			m_zoomBoneInterpFactor += 0.1f * msg.Data;
			if(m_zoomBoneInterpFactor > 1.0f)
				m_zoomBoneInterpFactor = 1.0f;
		}
		setFitDistanceFactor (val);
		recomputeZoom();
		setViewDirty (true);
		setCameraLookAtBoneDirty(true);
		return true;
	}
	else if (msg.Type == UIMessage::LeftMouseDown)
	{
		bool startDrag = true;
		if (hasFlags(F_rotateNeedsControlKey))
		{
			startDrag = msg.Modifiers.isControlDown();
		}

		if ((startDrag) && (s_debugControls || hasFlags (F_dragYawOk) || hasFlags (F_dragPitchOk)))
		{
			setFlags (F_dragging);
			m_lastMousePoint = msg.MouseCoords;
			return true;
		}

		return false;
	}
	else if (msg.Type == UIMessage::LeftMouseUp)
	{
		if (hasFlags (F_dragging))
		{
			//-----------------------------------------------------------------
			//-- stop rotating if the user pauses dragging

			if ((UIClock::gUIClock ().GetTime () - m_lastDragTime) > 10)
				setRotateSpeed (0.0f);

			unsetFlags (F_dragging);
		}
		return false;
	}
	else if (msg.Type == UIMessage::MouseMove)
	{
		if (s_debugControls && hasFlags (F_draggingDebug))
		{
			const UIPoint diff (msg.MouseCoords - m_lastMousePoint);

			setCameraYaw (m_cameraYawPitchZoom.x + static_cast<float>(diff.x) * 0.008f);
			m_cameraYawPitchZoom.y += static_cast<float>(diff.y) * 0.005f;
			m_lastMousePoint = msg.MouseCoords;

			m_cameraYawPitchZoom.y = angleClamp2Pi (m_cameraYawPitchZoom.y);

			REPORT_LOG_PRINT (s_debugCamera, ("Yaw: %5.2f, Pitch: %5.2f\n", m_cameraYawPitchZoom.x, m_cameraYawPitchZoom.y));

			setViewDirty (true);
			return true;
		}
		else if (hasFlags (F_dragging))
		{
			{
				Vector delta;
				delta.x = static_cast<real>(msg.MouseCoords.x - m_lastMousePoint.x);
				delta.y = static_cast<real>(msg.MouseCoords.y - m_lastMousePoint.y);

				const float frameTime = Clock::frameTime ();
				static const float SENSITIVITY_ROTATE_YAW   = 0.010f;
				static const float SENSITIVITY_ROTATE_PITCH = 0.005f;

				if (hasFlags (F_dragYawOk) && delta.x)
				{
					//-- reverse the delta to match yaw angle
					delta.x *= SENSITIVITY_ROTATE_YAW;

					setRotateSpeed (frameTime != 0.0f ? delta.x / frameTime : delta.x);
					m_cameraYawPitchZoom.x      += delta.x;
					m_cameraYawPitchZoom.x       = angleClamp2Pi (m_cameraYawPitchZoom.x);

					m_cameraYawPitchZoomTarget.x = m_cameraYawPitchZoom.x;
				}

				if (hasFlags (F_dragPitchOk) && delta.y)
				{
					//-- reverse the delta to match pitch angle
					delta.y *= SENSITIVITY_ROTATE_PITCH;

					m_cameraYawPitchZoom.y      += delta.y;

					if (!m_noCameraClamp)
						m_cameraYawPitchZoom.y       = std::max (std::min (m_dragPitchMax, m_cameraYawPitchZoom.y), m_dragPitchMinActual);
								
					m_cameraYawPitchZoomTarget.y = m_cameraYawPitchZoom.y;
				}

				m_lastMousePoint = msg.MouseCoords;
				m_lastDragTime   = UIClock::gUIClock ().GetTime ();
				return true;
			}
		}
	}

	return UIWidget::ProcessUserMessage (msg);
}

//----------------------------------------------------------------------

void  CuiWidget3dObjectListViewer::SetVisible (bool b)
{
	UIWidget::SetVisible (b);
	setPaused (!IsVisible ());

	//-- Clear out cached shaders
	if (!b)
		clearOverrideShaderMap();
}

//----------------------------------------------------------------------

void  CuiWidget3dObjectListViewer::SetSize( const UISize & size)
{
	UIWidget::SetSize (size);
	setViewDirty (true);
}

//----------------------------------------------------------------------

void  CuiWidget3dObjectListViewer::Notify( UINotificationServer *, UIBaseObject *, Code  )
{
	//-- make sure this widget will render this frame
	{
		const UIWidget * wid = this;

		for (; wid; )
		{
			const UIBaseObject * const p = wid->GetParent ();
			if (p)
			{
				if (!wid->WillDraw())
					return;

				if (p->IsA(TUIWidget))
					wid = static_cast<const UIWidget *>(p);
				else
					return;
			}
			else
				break;
		}
	}

	IGNORE_RETURN(m_objectWatcherList->removeNulls());

	{
		for (ObjectRenderVector::iterator it = m_objectVector->begin (); it != m_objectVector->end ();)
		{
			const ObjectPair & op = *it;

			const Object * const obj = op.second.getPointer ();
			if (!obj)
			{
				it = m_objectVector->erase (it);
			}
			else
			{
				const Appearance * const app = obj->getAppearance ();
				if (app)
					app->setRenderedThisFrame ();

				++it;
			}
		}
	}

	const float frameTime = Clock::frameTime ();

	if (hasFlags (F_alterObjects))
	{
		m_objectWatcherList->prepareToAlter ();
		m_objectWatcherList->alter          (frameTime);
		m_objectWatcherList->conclude       ();
	}

	{
		for (int i = 0; i < TOT_numTextOverlays; ++i)
		{
			TextOverlay * const to = m_textOverlays [i];
			if (to)
				to->update (frameTime);
		}
	}

	bool turned = false;
	bool shouldRecomputeClipPlanes = false;

	if (!hasFlags(F_dragging) && m_rotateSpeed != 0.0f)
	{
		static const real MAX_ROTATE_SPEED = 1.0f;
		static const float DEFAULT_RESTING_YAW = convertDegreesToRadians(215.0f);

		if (fabs(m_rotateSpeed) > MAX_ROTATE_SPEED)
		{
			setRotateSpeed(m_rotateSpeed * 0.96f);
		}

		m_cameraYawPitchZoom.x += m_rotateSpeed * frameTime;
		m_cameraYawPitchZoom.x = angleClamp2Pi(m_cameraYawPitchZoom.x);

		if ((m_rotationSlowsToStop) && (fabs(m_rotateSpeed) <= MAX_ROTATE_SPEED))
		{
			if (withinEpsilon(static_cast<float>(m_cameraYawPitchZoom.x), DEFAULT_RESTING_YAW, 0.1f))
			{
				m_rotateSpeed = 0.0f;
			}
		}

		m_cameraYawPitchZoomTarget.x = m_cameraYawPitchZoom.x;
		shouldRecomputeClipPlanes = true; 
	}

	//----------------------------------------------------------------------
	//-- interpolate the yaw, pitch, zoom, and lookat

	if (!hasFlags (F_cameraForceTarget) && hasFlags (F_cameraInterpolate))
	{
		//-- interpolate the yaw

		static const float ANGLE_THRESHOLD  = 0.001f;
		static const float ANGLE_CHANGERATE = 2.0f;
		const float last_frameTime = Clock::frameTime ();

		if (!WithinEpsilonInclusive(m_cameraYawPitchZoom.x, m_cameraYawPitchZoomTarget.x, ZOOM_THRESHOLD))
		{
			const bool up = m_cameraYawPitchZoomTarget.x > m_cameraYawPitchZoom.x;

			m_cameraYawPitchZoom.x += angleDiff2Pi (m_cameraYawPitchZoomTarget.x, m_cameraYawPitchZoom.x) * last_frameTime * ANGLE_CHANGERATE;

			if (up)
				m_cameraYawPitchZoom.x = std::min (m_cameraYawPitchZoom.x, m_cameraYawPitchZoomTarget.x);
			else
				m_cameraYawPitchZoom.x = std::max (m_cameraYawPitchZoom.x, m_cameraYawPitchZoomTarget.x);

			if (WithinEpsilonExclusive (m_cameraYawPitchZoomTarget.x, m_cameraYawPitchZoom.x, ANGLE_THRESHOLD))
				m_cameraYawPitchZoom.x = m_cameraYawPitchZoomTarget.x;

			turned = true;
			shouldRecomputeClipPlanes = true;
		}

		//-- interpolate the zoom
		if (!WithinEpsilonInclusive(m_cameraYawPitchZoom.z, m_cameraYawPitchZoomTarget.z, ZOOM_THRESHOLD))
		{
			//-- sometimes don't zoom in while turning
			if (hasFlags (F_cameraZoomInWhileTurn) || !turned || m_cameraYawPitchZoom.z < m_cameraYawPitchZoomTarget.z)
			{
				const bool up = m_cameraYawPitchZoomTarget.z > m_cameraYawPitchZoom.z;

				m_cameraYawPitchZoom.z += (m_cameraYawPitchZoomTarget.z - m_cameraYawPitchZoom.z) * last_frameTime * m_zoomInterpolationRate;

				if (up)
					m_cameraYawPitchZoom.z = std::min (m_cameraYawPitchZoom.z, m_cameraYawPitchZoomTarget.z);
				else
					m_cameraYawPitchZoom.z = std::max (m_cameraYawPitchZoom.z, m_cameraYawPitchZoomTarget.z);

				if (WithinEpsilonExclusive (m_cameraYawPitchZoomTarget.z, m_cameraYawPitchZoom.z, ZOOM_THRESHOLD))
					m_cameraYawPitchZoom.z = m_cameraYawPitchZoomTarget.z;
			}
			shouldRecomputeClipPlanes = true;
		}

		//-- interpolate the lookat
		if (!m_cameraLookAt.withinEpsilon(m_cameraLookAtTarget, LOOKAT_THRESHOLD_SQUARED))
		{
			const float t = std::min (1.0f, last_frameTime * LOOKAT_CHANGERATE);
			m_cameraLookAt = Vector::linearInterpolate (m_cameraLookAt, m_cameraLookAtTarget, t);

			if (WithinEpsilonExclusive (0.0f, m_cameraLookAtTarget.magnitudeBetweenSquared (m_cameraLookAt), LOOKAT_THRESHOLD_SQUARED))
				m_cameraLookAt = m_cameraLookAtTarget;

			shouldRecomputeClipPlanes = true;
		}
	}

	//----------------------------------------------------------------------
	//-- don't interpolate, just force the value

	else
	{
		if (!WithinEpsilonInclusive(m_cameraYawPitchZoom.x, m_cameraYawPitchZoomTarget.x, LOOKAT_THRESHOLD_SQUARED) ||
			!WithinEpsilonInclusive(m_cameraYawPitchZoom.z, m_cameraYawPitchZoomTarget.z, LOOKAT_THRESHOLD_SQUARED) ||
			!m_cameraLookAt.withinEpsilon(m_cameraLookAtTarget, LOOKAT_THRESHOLD_SQUARED))
			shouldRecomputeClipPlanes = true;
		
		m_cameraYawPitchZoom.x = m_cameraYawPitchZoomTarget.x;
		m_cameraYawPitchZoom.z = m_cameraYawPitchZoomTarget.z;
		m_cameraLookAt         = m_cameraLookAtTarget;
	}
	
	
	//--

	if (!WithinEpsilonInclusive(m_cameraYawPitchZoom.x, m_cameraYawPitchZoomLast.x, ZOOM_THRESHOLD) ||
		!WithinEpsilonInclusive(m_cameraYawPitchZoom.y, m_cameraYawPitchZoomLast.y, ZOOM_THRESHOLD))
	{
		m_cameraYawPitchZoomLast = m_cameraYawPitchZoom;
		setViewDirty(true);
	}

	if (shouldRecomputeClipPlanes)
		recomputeNearAndFarPlanes();

	float const curTime = Game::getElapsedTime ();
	if (curTime > (m_lastNameUpdateSecs + s_nameUpdateRate))
		updateObjectName();
}

//----------------------------------------------------------------------

bool  CuiWidget3dObjectListViewer::SetProperty( const UILowerString & Name, const UIString &Value )
{

	//-----------------------------------------------------
	//-- method properties
	//-----------------------------------------------------

	if (Name == MethodName::SetObject)
	{
		NetworkId const id (Unicode::wideToNarrow(Value));
		Object * const object = NetworkIdManager::getObjectById(id);
		setObject(object);

		if (!hasFlags(F_cameraForceTarget))
		{
			setViewDirty             (true);
			setCameraForceTarget     (true);
			recomputeZoom            ();
			setCameraForceTarget     (false);
		}

		return true;
	}

	//-----------------------------------------------------

	else if (Name == MethodName::ClearObjects)
	{
		clearObjects();
		return true;
	}

	//-----------------------------------------------------

	else if (Name == PropertyName::CameraTransformToObj)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setCameraTransformToObj (b);
			return true;
		}

		return false;
	}

	else if (Name == PropertyName::CameraLookAt)
	{
		Vector v;
		if (CuiUtils::ParseVector (Value, v))
		{
			setCameraLookAt (v, false);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::CameraLookAtBone)
	{
		setCameraLookAtBone (Unicode::wideToNarrow (Value));
		return true;
	}
	else if (Name == PropertyName::CameraFitBone  )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setCameraFitBone (b);
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::LookAtBoneOnlyY )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setUnsetFlags (F_lookAtBoneOnlyY, b);
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::CameraYaw)
	{
		float f = 0.0f;
		if (UIUtils::ParseFloat (Value, f))
		{
			setCameraYaw (f, false);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::CameraForceTarget)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setCameraForceTarget (b);
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::CameraLodBias)
	{
		float f = 0.0f;
		if (UIUtils::ParseFloat (Value, f))
		{
			setCameraLodBias (f);
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::CameraLodBiasOverride)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setCameraLodBiasOverride (b);
			return true;
		}

		return false;
	}

	else if (Name == PropertyName::CameraPitch)
	{
		return UIUtils::ParseFloat (Value, m_cameraYawPitchZoom.y);
	}
	else if (Name == PropertyName::CameraRoll)
	{
		float f = 0.0f;
		if (UIUtils::ParseFloat (Value, f))
		{
			setCameraRoll (f);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::LightYaw)
	{
		return UIUtils::ParseFloat (Value, m_lightYaw);
	}
	else if (Name == PropertyName::LightYaw2)
	{
		return UIUtils::ParseFloat (Value, m_lightYaw2);
	}
	else if (Name == PropertyName::LightPitch)
	{
		return UIUtils::ParseFloat (Value, m_lightPitch);
	}
	else if (Name == PropertyName::LightPitch2)
	{
		return UIUtils::ParseFloat (Value, m_lightPitch2);
	}
	else if (Name == PropertyName::FieldOfView)
	{
		if (UIUtils::ParseFloat (Value, m_fieldOfView))
		{
			//- convert from degrees to radians
			m_fieldOfView = m_fieldOfView * PI / 180.0f;
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::LightAmbientColor)
	{
		if (CuiUtils::ParseVectorArgb (Value, m_lightAmbientColor))
		{
			m_lightAmbient->setDiffuseColor (m_lightAmbientColor);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::LightColor)
	{
		if (CuiUtils::ParseVectorArgb (Value, m_lightColor))
		{
			m_light->setDiffuseColor  (m_lightColor);
			m_light->setSpecularColor (m_lightColor);
			m_light->setSpecularColorScale (0.50f);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::LightColor2)
	{
		if (CuiUtils::ParseVectorArgb (Value, m_lightColor2))
		{
			m_light2->setDiffuseColor (m_lightColor2);
			m_light2->setSpecularColor (m_lightColor2);
			m_light2->setSpecularColorScale (0.50f);
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::LightLockToCamera  )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setUnsetFlags (F_lightLockToCamera, b);
			return true;
		}

		return false;
	}


	else if (Name == PropertyName::FitRect)
	{
		UIRect fitRect;
		if (UIUtils::ParseRect (Value, fitRect))
		{
			setFitRect (fitRect);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::AutoZoomOutOnly)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setAutoZoomOutOnly (b);
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::DragPitchMax)
	{
		return UIUtils::ParseFloat (Value, m_dragPitchMax);
	}
	else if (Name == PropertyName::DragPitchMin)
	{
		return UIUtils::ParseFloat (Value, m_dragPitchMin);
	}

	else if (Name == PropertyName::DragPitchOk  )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setUnsetFlags (F_dragPitchOk, b);
			return true;
		}

		return false;
	}

	else if (Name == PropertyName::DragYawOk  )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setUnsetFlags (F_dragYawOk, b);
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::CameraCompensateScale  )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setUnsetFlags (F_cameraCompensateScale, b);
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::ScaleMaxOverride  )
	{
		return UIUtils::ParseFloat (Value, m_scaleMaxOverride);
	}

	else if (Name == PropertyName::CameraLookAtCenter)
	{
		bool b = false;
		bool success = UIUtils::ParseBoolean(Value, b);
		if (success) 
		{
			setCameraLookAtCenter(b);
		}
		return success;
	}

	else if (Name == PropertyName::Shadows)
	{
		bool b = false;
		bool success = UIUtils::ParseBoolean(Value, b);
		if (success) 
		{
			setUnsetFlags(F_useShadows, b);
		}
		return success;
	}

	else if (Name == PropertyName::FitDistanceFactor)
	{
		float f = 1.0f;
		if (UIUtils::ParseFloat (Value, f))
		{
			setFitDistanceFactor (f);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::Rotation)
	{
		float f = 1.0f;
		if (UIUtils::ParseFloat (Value, f))
		{
			setRotateSpeed (f);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::ObjectDataSource)
	{
		UIDataSource * const ds = safe_cast<UIDataSource *>(GetObjectFromPath (Value, TUIDataSource));
		if (ds || Value.empty ())
		{
			setObjectDataSource (ds);
			RemoveProperty (Name);
			return true;
		}
	}
	else if (Name == PropertyName::OverrideShader)
	{
		bool b = false;

		if (UIUtils::ParseBoolean(Value, b))
		{
			setUseOverrideShader(UIUnicode::wideToNarrow(Value), b);	
		}
		return true;
	}
	else if (Name == PropertyName::OverrideShaderTexture)
	{
		bool b = false;

		if (UIUtils::ParseBoolean(Value, b))
		{
			setUnsetFlags(F_useOverrideShaderTexture, b);
			return true;
		}

		return false;
	}
	else if (Name == PropertyName::Paused)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setPaused (b);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::TextStyleBottom)
	{
		UITextStyle * const style = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);
		if( style || Value.empty() )
		{
			setTextStyle (TOT_bottom, style);
			return true;
		}
	}
	else if (Name == PropertyName::TextStyleTop)
	{
		UITextStyle * const style = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);
		
		if( style || Value.empty() )
		{
			setTextStyle (TOT_topRight, style);
			setTextStyle (TOT_topLeft,  style);
			return true;
		}
	}
	else if (Name == PropertyName::RenderObjectEffects)
	{
		bool newValue = false;
		bool result = UIUtils::ParseBoolean(Value, newValue);
		m_renderObjectEffects = newValue;
		return result;
	}
	else if (Name == PropertyName::RStyleOverlay)
	{
		UIRectangleStyle * const rs = UI_ASOBJECT(UIRectangleStyle, GetObjectFromPath( Value, TUIRectangleStyle));
		if (rs || Value.empty())
		{
			setOverlay (rs);
			RemoveProperty (Name);
			return true;
		}
	}
	else if (Name == PropertyName::RStyleOverlayColor)
	{
		return UIUtils::ParseColor (Value, m_overlayColor);
	}
	else if (Name == PropertyName::RStyleOverlayOpacity)
	{
		return UIUtils::ParseFloat (Value, m_overlayOpacity);
	}
	else if (Name == PropertyName::CameraAutoZoom)
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			setCameraAutoZoom (b);
			return true;
		}
		return false;
	}

	return UIWidget::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool  CuiWidget3dObjectListViewer::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::CameraTransformToObj)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_cameraTransformToObj));
	}
	else if (Name == PropertyName::CameraLookAt)
	{
		return CuiUtils::FormatVector (Value, m_cameraLookAt);
	}
	else if (Name == PropertyName::CameraLookAtBone)
	{
		Value = Unicode::narrowToWide (m_cameraLookAtBone);
		return true;
	}
	else if (Name == PropertyName::CameraFitBone  )
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_cameraFitBone));
	}
	else if (Name == PropertyName::LookAtBoneOnlyY  )
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_lookAtBoneOnlyY));
	}
	else if (Name == PropertyName::CameraYaw)
	{
		return UIUtils::FormatFloat (Value, m_cameraYawPitchZoom.x);
	}
	else if (Name == PropertyName::CameraForceTarget)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_cameraForceTarget));
	}
	else if (Name == PropertyName::CameraLodBias)
	{
		return UIUtils::FormatFloat (Value, m_cameraLodBias);
	}
	else if (Name == PropertyName::CameraLodBiasOverride)
	{
		return UIUtils::FormatBoolean(Value, hasFlags(F_cameraLodBiasOverride));
	}
	else if (Name == PropertyName::CameraPitch)
	{
		return UIUtils::FormatFloat (Value, m_cameraYawPitchZoom.y);
	}
	else if (Name == PropertyName::CameraRoll)
	{
		return UIUtils::FormatFloat (Value, m_cameraRoll);
	}
	else if (Name == PropertyName::LightYaw)
	{
		return UIUtils::FormatFloat (Value, m_lightYaw);
	}
	else if (Name == PropertyName::LightYaw2)
	{
		return UIUtils::FormatFloat (Value, m_lightYaw2);
	}
	else if (Name == PropertyName::LightPitch)
	{
		return UIUtils::FormatFloat (Value, m_lightPitch);
	}
	else if (Name == PropertyName::LightPitch2)
	{
		return UIUtils::FormatFloat (Value, m_lightPitch2);
	}
	else if (Name == PropertyName::LightAmbientColor)
	{
		return CuiUtils::FormatVectorArgb (Value, m_lightAmbientColor);
	}
	else if (Name == PropertyName::LightColor)
	{
		return CuiUtils::FormatVectorArgb (Value, m_lightColor);
	}
	else if (Name == PropertyName::LightColor2)
	{
		return CuiUtils::FormatVectorArgb (Value, m_lightColor2);
	}
	else if (Name == PropertyName::LightLockToCamera )
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_lightLockToCamera));
	}
	else if (Name == PropertyName::FieldOfView)
	{
		//- convert from radians to degrees
		return UIUtils::FormatFloat (Value, m_fieldOfView * 180.0f / PI);
	}
	else if (Name == PropertyName::FitRect)
	{
		return UIUtils::FormatRect (Value, m_fitRect);
	}
	else if (Name == PropertyName::AutoZoomOutOnly)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_autoZoomOutOnly));
	}
	else if (Name == PropertyName::DragPitchMax)
	{
		return UIUtils::FormatFloat (Value, m_dragPitchMax);
	}
	else if (Name == PropertyName::DragPitchMin)
	{
		return UIUtils::FormatFloat (Value, m_dragPitchMin);
	}
	else if (Name == PropertyName::DragYawOk  )
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_dragYawOk));
	}
	else if (Name == PropertyName::DragPitchOk  )
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_dragPitchOk));
	}
	else if (Name == PropertyName::CameraCompensateScale  )
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_cameraCompensateScale));
	}
	else if (Name == PropertyName::ScaleMaxOverride  )
	{
		return UIUtils::FormatFloat (Value, m_scaleMaxOverride);
	}
	else if (Name == PropertyName::CameraLookAtCenter)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_cameraLookAtCenter));
	}
	else if (Name == PropertyName::Shadows)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_useShadows));
	}
	else if (Name == PropertyName::FitDistanceFactor)
	{
		return UIUtils::FormatFloat (Value, m_fitDistanceFactor);
	}
	else if (Name == PropertyName::Rotation)
	{
		return UIUtils::FormatFloat (Value, m_rotateSpeed);
	}
	else if (Name == PropertyName::ObjectDataSource)
	{
		if (m_objectDataSource)
		{
			const UIDataSource * const ds = m_objectDataSource->getDataSource ();

			if (ds)
			{
				GetPathTo (Value, ds);
				return true;
			}
		}
	}
	else if (Name == PropertyName::OverrideShader)
	{
		return UIUtils::FormatBoolean(Value, hasFlags(F_useOverrideShader));
	}
	else if (Name == PropertyName::OverrideShaderTexture)
	{
		return UIUtils::FormatBoolean(Value, hasFlags(F_useOverrideShaderTexture));
	}
	else if (Name == PropertyName::Paused)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_paused));
	}
	else if (Name == PropertyName::TextStyleBottom)
	{
		if (m_textOverlays [TOT_bottom] && m_textOverlays [TOT_bottom]->style)
		{
			Value = m_textOverlays [TOT_bottom]->style->GetLogicalName();
			return true;
		}
	}
	else if (Name == PropertyName::TextStyleTop)
	{
		if (m_textOverlays [TOT_topLeft] && m_textOverlays [TOT_topLeft]->style)
		{
			Value = m_textOverlays [TOT_topLeft]->style->GetLogicalName();
			return true;
		}
	}
	else if (Name == PropertyName::RenderObjectEffects)
	{
		return UIUtils::FormatBoolean (Value, m_renderObjectEffects);
	}
	else if (Name == PropertyName::RStyleOverlay)
	{
		if (m_overlay)
		{
			GetPathTo (Value, m_overlay);
			return true;
		}
	}
	else if (Name == PropertyName::RStyleOverlayColor)
	{
		return UIUtils::FormatColor (Value, m_overlayColor);
	}
	else if (Name == PropertyName::RStyleOverlayOpacity)
	{
		return UIUtils::FormatFloat (Value, m_overlayOpacity);
	}
	else if (Name == PropertyName::CameraAutoZoom)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_cameraAutoZoom));
	}
	return UIWidget::GetProperty (Name, Value);
}

//----------------------------------------------------------------------

void  CuiWidget3dObjectListViewer::GetLinkPropertyNames( UIPropertyNameVector & in) const
{
	in.push_back (PropertyName::ObjectDataSource);
	in.push_back (PropertyName::TextStyleBottom);
	in.push_back (PropertyName::TextStyleTop);
	in.push_back (PropertyName::RStyleOverlay);

	UIWidget::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void  CuiWidget3dObjectListViewer::GetPropertyNames( UIPropertyNameVector & in, bool forCopy) const
{
	in.push_back (PropertyName::AutoZoomOutOnly);
	in.push_back (PropertyName::CameraAutoZoom);
	in.push_back (PropertyName::CameraCompensateScale);
	in.push_back (PropertyName::CameraFitBone   );
	in.push_back (PropertyName::CameraForceTarget);
	in.push_back (PropertyName::CameraLodBias);
	in.push_back (PropertyName::CameraLodBiasOverride);
	in.push_back (PropertyName::CameraLookAt   );
	in.push_back (PropertyName::CameraLookAtBone);
	in.push_back (PropertyName::CameraLookAtCenter );
	in.push_back (PropertyName::CameraPitch   );
	in.push_back (PropertyName::CameraRoll  );
	in.push_back (PropertyName::CameraTransformToObj);
	in.push_back (PropertyName::CameraYaw   );
	in.push_back (PropertyName::DragPitchMax);
	in.push_back (PropertyName::DragPitchMin);
	in.push_back (PropertyName::DragPitchOk);
	in.push_back (PropertyName::DragYawOk);
	in.push_back (PropertyName::FieldOfView );
	in.push_back (PropertyName::FitDistanceFactor);
	in.push_back (PropertyName::FitRect );
	in.push_back (PropertyName::LightAmbientColor);
	in.push_back (PropertyName::LightColor   );
	in.push_back (PropertyName::LightColor2  );
	in.push_back (PropertyName::LightLockToCamera);
	in.push_back (PropertyName::LightPitch   );
	in.push_back (PropertyName::LightPitch2  );
	in.push_back (PropertyName::LightYaw   );
	in.push_back (PropertyName::LightYaw2  );
	in.push_back (PropertyName::LookAtBoneOnlyY );
	in.push_back (PropertyName::ObjectDataSource);
	in.push_back (PropertyName::OverrideShader);
	in.push_back (PropertyName::OverrideShaderTexture);
	in.push_back (PropertyName::Paused);
	in.push_back (PropertyName::RenderObjectEffects);
	in.push_back (PropertyName::RStyleOverlay);
	in.push_back (PropertyName::RStyleOverlayColor);
	in.push_back (PropertyName::RStyleOverlayOpacity);
	in.push_back (PropertyName::Rotation);
	in.push_back (PropertyName::ScaleMaxOverride );
	in.push_back (PropertyName::Shadows);
	in.push_back (PropertyName::TextStyleBottom);
	in.push_back (PropertyName::TextStyleTop);

	UIWidget::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

UIWidget * CuiWidget3dObjectListViewer::GetCustomDragWidget (const UIPoint & point, UIPoint& offset)
{
	return UIWidget::GetCustomDragWidget (point, offset);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setPaused (bool b)
{
	if (b == hasFlags (F_paused)) //lint !e731
		return;

	setUnsetFlags (F_paused, b);

	if (b)
		UIClock::gUIClock().StopListening( this );
	else
		UIClock::gUIClock().ListenPerFrame ( this );

}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setAlterObjects (bool b)
{
	setUnsetFlags (F_alterObjects, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setViewDirty (bool b)
{
	const int curFrame = Graphics::getFrameNumber ();
	if (b)
	{
		setUnsetFlags (F_viewDirty, b);
		m_viewDirtyFrame = curFrame;
	}

	else
	{
		const int DIRTY_THRESHOLD = 2;
		if (curFrame > m_viewDirtyFrame + DIRTY_THRESHOLD)
		{
			m_viewDirtyFrame = 0;
			setUnsetFlags (F_viewDirty, b);
		}
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraCompensateScale (bool b)
{
	setUnsetFlags (F_cameraCompensateScale, b);
	setViewDirty (true);
	setCameraLookAtBoneDirty (true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraForceTarget     (bool b)
{
	setUnsetFlags (F_cameraForceTarget, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraLookAtBoneDirty (bool b)
{
	const int curFrame = Graphics::getFrameNumber ();
	if (b)
	{
		setUnsetFlags (F_cameraLookAtBoneDirty, b);
		m_viewDirtyFrame = curFrame;
	}

	else
	{
		const int DIRTY_THRESHOLD = 2;
		if (curFrame > m_viewDirtyFrame + DIRTY_THRESHOLD)
		{
			m_viewDirtyFrame = 0;
			setUnsetFlags (F_cameraLookAtBoneDirty, b);
		}
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setAutoZoomOutOnly (bool b)
{
	setUnsetFlags (F_autoZoomOutOnly, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraFitBone (bool b)
{
	setUnsetFlags (F_cameraFitBone, b);
	setViewDirty (true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setFitRect   (const UIRect & rect)
{
	m_fitRect = rect;
	setViewDirty (true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setFlags (Flags f)
{
	m_flags |= f;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::unsetFlags (Flags f)
{
	m_flags &= ~f;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setUnsetFlags (Flags f, bool set)
{
	if (set)
		setFlags (f);
	else
		unsetFlags (f);
}

//----------------------------------------------------------------------

const float CuiWidget3dObjectListViewer::computeFitDistance (bool & appearancesReady) const
{
	static const float TANGENT_FUDGE_FACTOR = 1.00f;

	//-- determine the effective FOV by comparing the fitRect to the widget's rectangle
	//-- the effective vertical fov is:
	//-- f' = 2 * atan ( h * tan (f / 2) / H)
	//-- where h = the height of the fitRect
	//--       H = the height of the widget
	//--       f = the camera's vertical field of view

	//-- m_fitRect is in percentage units

	UIFloatPoint fitRect_size;
	fitRect_size.x = static_cast<float>((m_fitRect.right - m_fitRect.left) * GetWidth()) * 0.01f * m_fitScale.x;
	fitRect_size.y = static_cast<float>((m_fitRect.bottom - m_fitRect.top) * GetHeight()) * 0.01f * m_fitScale.y;

	const float n_vertical                = fitRect_size.y * tan (m_camera->getVerticalFieldOfView   () * 0.5f / static_cast<float>(GetHeight ()));
	const float fov_vertical_half         = static_cast<float>(atan (n_vertical));

	const float n_horizontal              = fitRect_size.x * tan (m_camera->getHorizontalFieldOfView   () * 0.5f / static_cast<float>(GetWidth ()));
	const float fov_horizontal_half       = static_cast<float>(atan (n_horizontal));

	const float halfTan_vertical          = tan (fov_vertical_half   * TANGENT_FUDGE_FACTOR);
	const float halfTan_horizontal        = tan (fov_horizontal_half * TANGENT_FUDGE_FACTOR);

	const float recip_halfTan_vertical    = halfTan_vertical != 0.0f   ? RECIP (halfTan_vertical)   : 0.0f;
	const float recip_halfTan_horizontal  = halfTan_horizontal != 0.0f ? RECIP (halfTan_horizontal) : 0.0f;

	float max_dz = -100000.0f;

	Transform transformDiff = Transform::identity;
	transformDiff.move_l  (m_cameraLookAtTarget);
	transformDiff.yaw_l   (m_cameraYawPitchZoom.x);
	transformDiff.pitch_l (m_cameraYawPitchZoom.y);

	static std::vector<Vector> vv;
	vv.clear ();
	computeFitPointCloud (vv, appearancesReady);

	for (std::vector<Vector>::const_iterator it = vv.begin (); it != vv.end (); ++it)
	{
		const Vector transformed (transformDiff.rotateTranslate_p2l (*it));

		//----------------------------------------------------------------------

		float dz = 0.0f;

		{
			if (transformed.y >= 0.0f)
				dz = transformed.y * recip_halfTan_vertical - transformed.z;
			else
				dz = -transformed.y * recip_halfTan_vertical - transformed.z;

			max_dz = std::max (max_dz, dz);
		}

		//----------------------------------------------------------------------

		{
			if (transformed.x >= 0.0f)
				dz = transformed.x * recip_halfTan_horizontal - transformed.z;
			else
				dz = -transformed.x * recip_halfTan_horizontal - transformed.z;

			max_dz = std::max (max_dz, dz);
		}
	}

	

	return max_dz * m_fitDistanceFactor;
}

//----------------------------------------------------------------------

bool CuiWidget3dObjectListViewer::checkAppearancesReady () const
{
	const int count = m_objectWatcherList->getNumberOfObjects();

	const bool lastObjectOnly = hasFlags (F_cameraFitBone) && !m_cameraLookAtBone.empty ();

	for (int i = 0; i < count; ++i)
	{
		const Object * const obj = m_objectWatcherList->getObject (i);
		if (!obj)
			continue;

		const Appearance * const app = obj->getAppearance ();

		if (!app)
			continue;

		if (lastObjectOnly && i < count-1)
			continue;

		const SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
		if (skelApp)
		{
			if (!const_cast<SkeletalAppearance2 *>(skelApp)->rebuildIfDirtyAndAvailable())
				return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::computeFitPointCloud (stdvector<Vector>::fwd & vv, bool & appearancesReady) const
{
	const int count = m_objectWatcherList->getNumberOfObjects();
	m_cameraLookAtBoneOk = false;

	vv.reserve(static_cast<size_t>(count * 8));

	float scale = 1.0f;

	const bool useScale = hasFlags (F_cameraCompensateScale) && (count == internal_object_count + 1);

	const bool lastObjectOnly = hasFlags (F_cameraFitBone) && !m_cameraLookAtBone.empty ();

	for (int i = 0; i < count; ++i)
	{
		const Object * const obj = m_objectWatcherList->getObject (i);
		if (!obj)
			continue;

		const Appearance * const app = obj->getAppearance ();

		if (!app)
			continue;

		if (lastObjectOnly && i < count-1)
			continue;

		if (appearancesReady)
		{
			const SkeletalAppearance2 * const skelApp = app->asSkeletalAppearance2 ();
			if (skelApp)
				appearancesReady = const_cast<SkeletalAppearance2 *>(skelApp)->rebuildIfDirtyAndAvailable();
		}

		if (useScale)
		{
			scale = computeScale (*obj, m_scaleMaxOverride);
		}

		BoxExtent boxExtent;
		computeTransformedBoxExtent (*obj, boxExtent, false);

		if (!boxExtent.getBox ().isEmpty ())
		{
			static Vector vs[8];
			boxExtent.getCornerVectors (vs);

			for (int c = 0; c < 8; ++c)
			{
				Vector const & corner = (vs[c]) * scale;
				vv.push_back(corner);
			}
		}
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraLookAt (const Vector & v, bool stopRotating)
{
	m_cameraLookAt       = v;
	m_cameraLookAtTarget = v;
	if (stopRotating)
		setRotateSpeed       (0.0f);
	setViewDirty (true);
}

//----------------------------------------------------------------------

Vector CuiWidget3dObjectListViewer::getCameraFrame_k() const
{
	return m_camera->getObjectFrameK_p();
}

//----------------------------------------------------------------------

bool CuiWidget3dObjectListViewer::findWorldLocation(UIPoint const & pointLocalToControl, Vector & begin_w, Vector & end_w) const
{
	long const viewportWidth = m_viewport.Width();
	long const viewportHeight = m_viewport.Height();

	if ((viewportWidth <= 0) || (viewportHeight <= 0))
	{
		return false;
	}

	UIPoint const globalPoint(pointLocalToControl + m_viewport.Location ());

	m_camera->setViewport(m_viewport.left, m_viewport.top, viewportWidth, viewportHeight);

	begin_w = m_camera->getPosition_w();
	end_w = begin_w + (10000.0f * m_camera->rotate_o2w(m_camera->reverseProjectInScreenSpace (globalPoint.x, globalPoint.y)));

	Graphics::setViewport(0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());

	return true;
}

//----------------------------------------------------------------------

/**
* @param pt is in the coordinate space of this widget
*/

const ClientObject * CuiWidget3dObjectListViewer::getObjectAt (const UIPoint & pt) const
{
	Vector start;
	Vector end;

	if (!findWorldLocation(pt, start, end))
	{
		return 0;
	}

	const int count = m_objectWatcherList->getNumberOfObjects();

	float minimumDistanceSquared       = 100000.0f;
	const ClientObject * minimumObject = 0;

	for (int i = 0; i < count; ++i)
	{
		const Object * const obj_base = m_objectWatcherList->getObject (i);
		const ClientObject * obj = obj_base ? obj_base->asClientObject () : 0;

		if (obj)
		{
			const Appearance * const app = obj->getAppearance ();

			if (app)
			{
				const Vector objectStart = obj->rotateTranslate_w2o (start);
				const Vector objectEnd   = obj->rotateTranslate_w2o (end);

				const Extent * const extent = app->getExtent ();

				real t = 0.0f;
				if (extent && extent->intersect (objectStart, objectEnd, &t))
				{
					const Vector point = Vector::linearInterpolate (objectStart, objectEnd, t);
					const real dist = point.magnitudeBetweenSquared (start);

					if (!minimumObject || dist < minimumDistanceSquared)
					{
						minimumDistanceSquared = dist;
						minimumObject          = obj;
					}
				}
			}
		}
	}

	return minimumObject;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraPitch           (float f)
{
	m_cameraYawPitchZoomTarget.y = m_cameraYawPitchZoom.y = angleClamp2Pi (f);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraRoll           (float f)
{
	m_cameraRoll = angleClamp2Pi (f);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraYaw (float f, bool stopRotating)
{
	m_cameraYawPitchZoomTarget.x = m_cameraYawPitchZoom.x = angleClamp2Pi (f);
	if (stopRotating)
		setRotateSpeed (0.0f);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraYawTarget (float f, bool stopRotating)
{
	m_cameraYawPitchZoomTarget.x = angleClamp2Pi (f);
	if (stopRotating)
		setRotateSpeed (0.0f);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setLightYaw (float radians)
{
	m_lightYaw = radians;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setLightYaw2 (float radians)
{
	m_lightYaw2 = radians;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::forceUpdate ()
{
	Notify (0, 0, UINotification::ObjectChanged);
}

//----------------------------------------------------------------------

bool CuiWidget3dObjectListViewer::findScreenLocation (const ClientObject & obj, const Vector & objectLocation, UIPoint & screenLocation)
{
	if (m_objectWatcherList->find(obj))
	{
		if (m_viewport.Width () <= 0 || m_viewport.Height () <= 0)
			return 0;

		m_camera->setViewport (m_viewport.left, m_viewport.top, m_viewport.Width (), m_viewport.Height ());

		Vector screenVect;
		const Vector parentLocation (obj.rotateTranslate_o2w (objectLocation));
		const bool retval = m_camera->projectInWorldSpace(parentLocation, &screenVect.x, &screenVect.y, 0);
		Graphics::setViewport (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());

		if (retval)
		{
			const UIPoint gpt = m_viewport.Location ();

			screenLocation.x = static_cast<long>(screenVect.x - gpt.x);
			screenLocation.y = static_cast<long>(screenVect.y - gpt.y);
			return true;
		}

	}

	return false;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::checkRenderLimits ()
{
	m_maximumDesiredDetailLevel = 3;

	bool pitchMinOk = true;
	for (ObjectRenderVector::const_iterator it = m_objectVector->begin (); pitchMinOk && it != m_objectVector->end (); ++it)
	{
		const ObjectPair & op = *it;
		const Object * const obj_base = op.second.getPointer ();
		const ClientObject * const renderObject = obj_base ? obj_base->asClientObject () : 0;

		if (!renderObject)
			continue;

		pitchMinOk = dynamic_cast<const BuildingObject *>(renderObject) == 0 && dynamic_cast<const InstallationObject *>(renderObject) == 0;

		if (pitchMinOk)
		{
			pitchMinOk = !GameObjectTypes::isTypeOf (renderObject->getGameObjectType (), SharedObjectTemplate::GOT_terminal);

			if (pitchMinOk)
				pitchMinOk = !GameObjectTypes::isTypeOf (renderObject->getGameObjectType (), SharedObjectTemplate::GOT_static);

			if (pitchMinOk)
				pitchMinOk = renderObject->getGameObjectType () != SharedObjectTemplate::GOT_misc_crafting_station;

			if (pitchMinOk)
				pitchMinOk = renderObject->getGameObjectType () != SharedObjectTemplate::GOT_misc_furniture;
		}

		if (GameObjectTypes::isTypeOf (renderObject->getGameObjectType (), SharedObjectTemplate::GOT_jewelry))
			m_maximumDesiredDetailLevel = 1;
	}

	if (!m_noCameraClamp)
	{
		if (pitchMinOk)
			m_dragPitchMinActual = m_dragPitchMin;
		else
		{
			m_dragPitchMinActual = PI / 32.0f;
			m_cameraYawPitchZoom.y = m_cameraYawPitchZoomTarget.y = std::max (m_dragPitchMinActual, m_cameraYawPitchZoom.y);
		}
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::clearObjects ()
{
	m_objectVector->clear ();

	m_objectWatcherList->removeObject (*m_light);
	m_objectWatcherList->removeObject (*m_light2);
	m_objectWatcherList->removeObject (*m_lightAmbient);

	m_objectWatcherList->removeAll (false);

	m_objectWatcherList->addObject (*m_light);
	m_objectWatcherList->addObject (*m_light2);
	m_objectWatcherList->addObject (*m_lightAmbient);

	setCameraLookAtBoneDirty (true);
	setViewDirty (true);

	m_numCreatures = 0;

	updateObjectName ();
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::addObject    (Object & obj)
{
	Object * const renderObject = &obj;

	m_objectVector->push_back (ObjectPair (Watcher<Object>(&obj), Watcher<Object>(renderObject)));

	m_objectWatcherList->addObject (*renderObject);
	setCameraLookAtBoneDirty (true);
	setViewDirty (true);

	if (hasFlags (F_alterObjects))
	{
		if (obj.isInitialized() && !obj.isInWorld() && (obj.alter(Clock::frameTime()) != AlterResult::cms_kill))
		{
			obj.conclude();
		}
	}

	if (dynamic_cast<const CreatureObject *>(&obj))
		++m_numCreatures;

	updateObjectName ();

	checkRenderLimits ();
}


//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::removeObject (Object & obj)
{
	const ObjectRenderVector::iterator it = std::find_if (m_objectVector->begin (), m_objectVector->end (), CompareFirst (obj));

	if (it == m_objectVector->end ())
		WARNING (true, ("Removing an object that does not exist in the m_objectMap"));
	else
	{
		Object * const renderObject = (*it).second;
		m_objectWatcherList->removeObject (*renderObject);
		IGNORE_RETURN(m_objectVector->erase(it));

		setCameraLookAtBoneDirty (true);
		setViewDirty (true);
	}

	if (dynamic_cast<const CreatureObject *>(&obj))
		--m_numCreatures;

	updateObjectName ();
	checkRenderLimits ();
}

//----------------------------------------------------------------------

Object * CuiWidget3dObjectListViewer::getLastObject ()
{
	if (m_objectVector->empty ())
		return 0;
	else
		return m_objectVector->back ().first;
}

//----------------------------------------------------------------------

const Object * CuiWidget3dObjectListViewer::getLastObject () const
{
	return const_cast<CuiWidget3dObjectListViewer *>(this)->getLastObject ();
}

//----------------------------------------------------------------------

Object * CuiWidget3dObjectListViewer::getLastRenderObject ()
{
	Object * const obj = getLastObject ();

	if (!obj)
		return 0;

	const ObjectRenderVector::iterator it = std::find_if (m_objectVector->begin (), m_objectVector->end (), CompareFirst (*obj));

	if (it == m_objectVector->end ())
		WARNING (true, ("get Last RenderObject does not exist in the m_objectMap"));
	else
		return (*it).second;

	return 0;
}

//----------------------------------------------------------------------

const Object * CuiWidget3dObjectListViewer::getLastRenderObject () const
{
	return const_cast<CuiWidget3dObjectListViewer *>(this)->getLastRenderObject ();
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraLookAtBone (const std::string & bone)
{
	m_zoomBoneInterpFactor = 0.0f;
	m_cameraLookAtBone = bone;
	setCameraLookAtBoneDirty (true);
	setViewDirty (true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraZoomLookAtBone (const std::string & bone)
{
	m_zoomBoneInterpFactor = 0.0f;
	m_cameraZoomLookAtBone = bone;
	setCameraLookAtBoneDirty (true);
	setViewDirty (true);
}
//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraLookAtCenter    (bool b)
{
	setUnsetFlags            (F_cameraLookAtCenter, b);
	setCameraLookAtBoneDirty (true);
	setViewDirty             (true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setScaleMaxOverride (float f)
{
	m_scaleMaxOverride = f;
	setViewDirty (true);
	setCameraLookAtBoneDirty (true);
}

//----------------------------------------------------------------------

bool CuiWidget3dObjectListViewer::computeSkeletalBoundingBox (const SkeletalAppearance2 & skelApp, BoxExtent & box) const
{
	bool found = false;
	int root_index = 0;

	Skeleton const * const skeleton = skelApp.getDisplayLodSkeleton();
	if (skeleton)
	{
		skeleton->findTransformIndex(TemporaryCrcString (m_cameraLookAtBone.c_str (), true), &root_index, &found);

		if (found && (root_index >= 0))
		{
			int const count = skeleton->getTransformCount();

			for (int i = 0; i < count; ++i)
			{
				if (isBoneDescendedFrom(*skeleton, root_index, i))
				{
					const Transform & transform = skeleton->getJointToRootTransformArray() [i];
					const Vector & position_p   = transform.getPosition_p ();
					box.grow (position_p, false);
				}
			}

			Vector const & rootPos = skeleton->getJointToRootTransformArray() [root_index].getPosition_p ();

			//-- grow the box in the x-z plane to make it symmetric around 0,0

			Vector box_min = box.getMin ();
			Vector box_max = box.getMax ();

			box_min -= rootPos;
			box_max -= rootPos;

			box_min.x = std::min (box_min.x, - box_max.x);
			box_min.z = std::min (box_min.z, - box_max.z);
			box_min.x = box_min.z = std::min (box_min.x, box_min.z);

			box_max.x = std::max (box_max.x, - box_min.x);
			box_max.z = std::max (box_min.z, - box_min.z);
			box_max.x = box_max.z = std::max (box_max.x, box_max.z);

			box_min += rootPos;
			box_max += rootPos;

			box.setMin (box_min);
			box.setMax (box_max);

			box.calculateCenterAndRadius ();

			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::gotoCameraPivotPoint (bool useTarget) const
{
	m_camera->resetRotateTranslate_o2p ();

	Object const * obj = 0;

	{
		ObjectRenderVector::iterator ii = m_objectVector->begin();
		ObjectRenderVector::iterator iiEnd = m_objectVector->end();

		for (; ii != iiEnd; ++ii)
		{
			Object const * o = ii->first;
			Appearance const * const appearance = (o != 0) ? o->getAppearance() : 0;
			if ((appearance != 0) && (appearance->asSkeletalAppearance2()))
			{
				obj = o;
			}
		}
	}

	if (obj == 0)
	{
		obj = getLastRenderObject();

		if (obj == 0)
		{
			return;
		}
	}

	if (hasFlags(F_cameraLookAtBoneDirty))
	{
		bool lookAtUpdated = true;
		bool isReady       = true;

		const Appearance * const app = obj->getAppearance ();

		if (!m_cameraLookAtBone.empty ())
		{
			const SkeletalAppearance2 * const skelApp = app ? app->asSkeletalAppearance2 () : 0;
			if (skelApp)
			{
				bool found = false;
				int index = 0;

				//-- Force the skeleton and mesh processing to occur if it is needed.
				// @todo fix this, get a non-const Appearance instance.
				isReady = const_cast<SkeletalAppearance2 *>(skelApp)->rebuildIfDirtyAndAvailable ();

				Skeleton const * const skeleton = skelApp->getDisplayLodSkeleton ();
				if (skeleton)
				{
					skeleton->findTransformIndex(TemporaryCrcString (m_cameraLookAtBone.c_str (), true), &index, &found);

					if (found && index >= 0)
					{
						const Transform & transform = skeleton->getJointToRootTransformArray() [index];
						m_cameraLookAtTarget = transform.getPosition_p ();
						if(!m_cameraZoomLookAtBone.empty())
						{
							skeleton->findTransformIndex(TemporaryCrcString (m_cameraZoomLookAtBone.c_str (), true), &index, &found);
							if (found && index >= 0)
							{
								const Transform & transform = skeleton->getJointToRootTransformArray() [index];
								float clampedVal = std::max(std::min(m_zoomBoneInterpFactor, 1.0f), 0.0f);
								m_cameraLookAtTarget = Vector::linearInterpolate(m_cameraLookAtTarget, transform.getPosition_p (), clampedVal);
							}
						}
					}
				}
			}

		}
		else if (hasFlags (F_cameraLookAtCenter))
		{
			if (app)
			{
				BoxExtent boxExtent;
				computeTransformedBoxExtent (*obj, boxExtent, false);
				m_cameraLookAtTarget = boxExtent.getSphere ().getCenter ();
			}
		}
		else
			lookAtUpdated = false;

		if (lookAtUpdated)
		{
			if (!hasFlags (F_cameraLookAtCenter))
			{
				if (hasFlags (F_lookAtBoneOnlyY))
					m_cameraLookAtTarget.x = m_cameraLookAtTarget.z = 0.0f;

				if (hasFlags (F_cameraCompensateScale))
					m_cameraLookAtTarget *= computeScale (*obj, m_scaleMaxOverride);
			}

//			if (hasFlags (F_cameraTransformToObj))
//				m_cameraLookAtTarget = obj->rotateTranslate_w2o (m_cameraLookAtTarget);

			if (hasFlags (F_cameraForceTarget))
				const_cast<CuiWidget3dObjectListViewer *>(this)->m_cameraLookAt = m_cameraLookAtTarget;
		}

		const_cast<CuiWidget3dObjectListViewer *>(this)->setCameraLookAtBoneDirty (!isReady);
	}

	if (hasFlags (F_cameraTransformToObj))
		m_camera->setTransform_o2p (obj->getTransform_o2w ());

	if (useTarget)
		m_camera->move_o  (m_cameraLookAtTarget);
	else
		m_camera->move_o  (m_cameraLookAt);

	m_camera->yaw_o   (m_cameraYawPitchZoom.x);
	m_camera->pitch_o (m_cameraYawPitchZoom.y);
	m_camera->roll_o  (m_cameraRoll);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::recomputeZoom ()
{
	gotoCameraPivotPoint (true);

	bool appearancesReady = true;

	if (hasFlags (F_cameraAutoZoom))
	{
		//-- push the distance out 0.2f meters to always leave room for the near plane
		const float newDistance = computeFitDistance(appearancesReady) + 0.2f;

		if (hasFlags (F_autoZoomOutOnly) && !hasFlags (F_sizeChanged))
			m_cameraYawPitchZoomTarget.z = std::max (m_cameraYawPitchZoomTarget.z, newDistance);
		else
			m_cameraYawPitchZoomTarget.z = newDistance;

		if (!appearancesReady)
			m_cameraYawPitchZoomTarget.z = 0.0f;
	}
	else
	{
		m_cameraYawPitchZoomTarget.z = m_fitDistanceFactor;
	}

	UIPoint      fitRect_center = m_viewport.Location ();
	fitRect_center.x += static_cast<long>(static_cast<float>((m_fitRect.right  + m_fitRect.left) * GetWidth  ()) * 0.01f * 0.5f);
	fitRect_center.y += static_cast<long>(static_cast<float>((m_fitRect.bottom + m_fitRect.top)  * GetHeight ()) * 0.01f * 0.5f);

	const Vector targetVector = m_camera->reverseProjectInScreenSpace (fitRect_center.x, fitRect_center.y);

	m_cameraDeflectionAngle.x = -targetVector.theta ();
	m_cameraDeflectionAngle.y = -targetVector.phi   ();

	if (hasFlags (F_cameraForceTarget))
		const_cast<CuiWidget3dObjectListViewer *>(this)->m_cameraYawPitchZoom.z = m_cameraYawPitchZoomTarget.z;

	unsetFlags (F_sizeChanged);

	const_cast<CuiWidget3dObjectListViewer *>(this)->setViewDirty             (!appearancesReady);
	const_cast<CuiWidget3dObjectListViewer *>(this)->setCameraLookAtBoneDirty (!appearancesReady);

	recomputeNearAndFarPlanes();
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setUseOverrideShader     (const std::string & name, bool b)
{
	IS_NULL(ms_currentOverrideShaderTemplate);

	clearOverrideShaderMap();

	if (m_overrideShaderTemplate)
	{
		m_overrideShaderTemplate->release();
		m_overrideShaderTemplate = 0;
	}

	setUnsetFlags (F_useOverrideShader, b);

	if (b && !name.empty ())
		m_overrideShaderTemplate = ShaderTemplateList::fetch(name.c_str());
}

//----------------------------------------------------------------------

int CuiWidget3dObjectListViewer::getRenderObjectCount        () const
{
	return m_objectWatcherList->getNumberOfObjects ();
}

//----------------------------------------------------------------------

Object * CuiWidget3dObjectListViewer::getRenderObjectByIndex      (int index)
{
	DEBUG_FATAL (index < 0 || index >= m_objectWatcherList->getNumberOfObjects (), (""));
	return m_objectWatcherList->getObject (index);
}

//----------------------------------------------------------------------

const Object * CuiWidget3dObjectListViewer::getRenderObjectByIndex      (int index) const
{
	return const_cast<CuiWidget3dObjectListViewer *>(this)->getRenderObjectByIndex (index);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setObjectDataSource     (UIDataSource * ds)
{
	if (!m_objectDataSource && ds)
		m_objectDataSource = new CuiObjectDataSource;

	if (m_objectDataSource)
		m_objectDataSource->setDataSource (ds, m_objectDataSourceCallback);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setRotateSpeed  (float f)
{
	m_rotateSpeed = f;
}

//----------------------------------------------------------------------

void  CuiWidget3dObjectListViewer::setDragYawOk  (bool b)
{
	setUnsetFlags (F_dragYawOk, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setDragPitchOk           (bool b)
{
	setUnsetFlags (F_dragPitchOk, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setDrawName  (bool b)
{
	setUnsetFlags (F_drawName, b);
	updateObjectName ();
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraTransformToObj  (bool b)
{
	setUnsetFlags (F_cameraTransformToObj, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setTextStyle (TextOverlayTypes tot, UITextStyle * style)
{
	if (style)
	{
		if (!m_textOverlays [tot])
			m_textOverlays [tot] = new TextOverlay;
	}

	if (m_textOverlays [tot])
		m_textOverlays [tot]->setStyle (style);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setText (TextOverlayTypes tot, const Unicode::String & str, const UIColor & color)
{
	if (!str.empty ())
	{
		if (!m_textOverlays [tot])
			m_textOverlays [tot] = new TextOverlay;
	}

	if (m_textOverlays [tot])
		m_textOverlays [tot]->setText    (str, color);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setTextFadeOut           (TextOverlayTypes tot, bool b)
{
	if (m_textOverlays [tot])
		m_textOverlays [tot]->setFadeOut    (b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::updateObjectName ()
{
	if (hasFlags (F_drawName))
	{
		const Object * const obj_base = getLastObject ();
		const ClientObject * const obj = obj_base ? obj_base->asClientObject () : 0;

		if (obj)
		{
			const TangibleObject * const tangible = obj->asTangibleObject ();
			if (tangible && tangible->hasCondition (TangibleObject::C_magicItem))
			{
				const UIColor & color = CuiIconManager::getTextColorMagic ();
				setText (TOT_bottom, obj->getLocalizedName (), color);
			}
			else
			{
				const UIColor & color = CuiIconManager::getTextColorNormal ();
				setText (TOT_bottom, obj->getLocalizedName (), color);
			}

			return;
		}
	}

	setText (TOT_bottom, Unicode::emptyString, UIColor::white);

	m_lastNameUpdateSecs = Game::getElapsedTime ();
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setOverlay (UIRectangleStyle * rs)
{
	AttachMember (m_overlay, rs);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setObject (Object * obj)
{
	clearObjects ();
	if (obj)
	{
		addObject (*obj);
	}
}

//----------------------------------------------------------------------

float CuiWidget3dObjectListViewer::getFitDistanceFactor() const
{
	return m_fitDistanceFactor;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setFitDistanceFactor(float f)
{
	m_fitDistanceFactor = f;
	setViewDirty (true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraLodBias (float f)
{
	m_cameraLodBias = f;
	setViewDirty (true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraLodBiasOverride (bool b)
{
	setUnsetFlags (F_cameraLodBiasOverride, b);
	setViewDirty (true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setRotateNeedsControlKey(bool b)
{
	setUnsetFlags(F_rotateNeedsControlKey, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraAutoZoom (bool b)
{
	setUnsetFlags (F_cameraAutoZoom, b);
	setViewDirty (true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setCameraZoomInWhileTurn (bool b)
{
	setUnsetFlags (F_cameraZoomInWhileTurn, b);
	setViewDirty (true);
}

//----------------------------------------------------------------------

bool CuiWidget3dObjectListViewer::CanSelect() const
{
	return UIWidget::CanSelect () && IsSelectable ();
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::setEnvironmentTexture(Texture const * texture)
{
	m_camera->setEnvironmentTexture(texture);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::OnSizeChanged(UISize const & newSize, UISize const & oldSize)
{
	UIWidget::OnSizeChanged(newSize, oldSize);
	setFlags(F_sizeChanged);
	setViewDirty(true);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::addText3d(Vector const & location, UIString const & text, UIColor const & color, float backgroundOpacity)
{
	if (m_text3d == 0)
	{
		m_text3d = new Text3dVector;
	}

	Text3d * const text3d = new Text3d;
	text3d->m_location = location;
	text3d->m_text = text;
	text3d->m_color = color;
	text3d->m_backgroundOpacity = backgroundOpacity;
	m_text3d->push_back(text3d);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::clearText3d()
{
	if (m_text3d != 0)
	{
		Text3dVector::iterator ii = m_text3d->begin();
		Text3dVector::iterator iiEnd = m_text3d->end();
		for (; ii != iiEnd; ++ii)
		{
			Text3d * const text3d = *ii;
			delete text3d;
		}

		delete m_text3d;
		m_text3d = 0;
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectListViewer::recomputeNearAndFarPlanes()
{
	const int count = m_objectWatcherList->getNumberOfObjects();

	AxialBox totalAxialBox;

	for (int i = 0; i < count; ++i)
	{
		Object * const obj = m_objectWatcherList->getObject(i);
		if (!obj)
			continue;

		Appearance * const appearance = obj->getAppearance();
		if (!appearance)
			continue;

		AxialBox const & ab = obj->getTangibleExtent();

		//-- Transform box from child space into 'world' space and grow result
		if (!ab.isEmpty())
		{
			for (int j = 0; j < 8; ++j)
				totalAxialBox.add(obj->getTransform_o2w().rotateTranslate_l2p(ab.getCorner(j)));
		}
	}

	//-- we don't need to transform the totalAxialBox back to the camera lookat target because we only care about the
	//-- final radius of the box, not its position

	if (!totalAxialBox.isEmpty() && !m_forceDefaultClippingPlanes)
	{
		float const radius = totalAxialBox.getRadius();

		float nearPlane = clamp(s_nearPlaneMinimum, (m_cameraYawPitchZoom.z - radius) * 0.5f, s_nearPlaneMaximum);
		float farPlane = clamp(nearPlane + 1.0f, m_cameraYawPitchZoom.z + radius, s_farPlaneMaximum);
		m_camera->setNearPlane (nearPlane);
		m_camera->setFarPlane  (farPlane);
	}
	else
	{
		m_camera->setNearPlane (s_nearPlaneDefault);
		m_camera->setFarPlane  (s_farPlaneDefault);
	}
	
}

//=======================================================================
