// ======================================================================
//
// CuiWidget3dObjectViewer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiWidget3dObjectViewer.h"

#include "UICanvas.h"
#include "UIClock.h"
#include "UIMessage.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UIUtils.h"
#include "UIRectangleStyle.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientObject/ObjectListCamera.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientSkeletalAnimation/Skeleton.h"
#include "clientUserInterface/CuiLayerRenderer.h"
#include "sharedCollision/BoxExtent.h"
#include "sharedFoundation/Clock.h"
#include "sharedMath/Vector2d.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"

// ======================================================================

const UILowerString CuiWidget3dObjectViewer::PropertyName::ObjectOriginOffsetY   = UILowerString ("ObjectOriginOffsetY");
const UILowerString CuiWidget3dObjectViewer::PropertyName::ObjectYaw             = UILowerString ("ObjectYaw");
const UILowerString CuiWidget3dObjectViewer::PropertyName::ObjectPitch           = UILowerString ("ObjectPitch");
const UILowerString CuiWidget3dObjectViewer::PropertyName::LightYaw              = UILowerString ("LightYaw");
const UILowerString CuiWidget3dObjectViewer::PropertyName::LightPitch            = UILowerString ("LightPitch");
const UILowerString CuiWidget3dObjectViewer::PropertyName::LightAmbientColor     = UILowerString ("LightAmbientColor");
const UILowerString CuiWidget3dObjectViewer::PropertyName::DefaultRotateSpeed    = UILowerString ("DefaultRotateSpeed");
const UILowerString CuiWidget3dObjectViewer::PropertyName::MouseDraggable        = UILowerString ("MouseDraggable");
const UILowerString CuiWidget3dObjectViewer::PropertyName::AutoComputeView       = UILowerString ("AutoComputeView");
const UILowerString CuiWidget3dObjectViewer::PropertyName::DragRemoveObject      = UILowerString ("DragRemoveObject");
const UILowerString CuiWidget3dObjectViewer::PropertyName::TextStyle             = UILowerString ("TextStyle");
const UILowerString CuiWidget3dObjectViewer::PropertyName::HeadShot              = UILowerString ("HeadShot");
const UILowerString CuiWidget3dObjectViewer::PropertyName::RStyleOverlay         = UILowerString ("RStyleOverlay");
const UILowerString CuiWidget3dObjectViewer::PropertyName::RStyleOverlayColor    = UILowerString ("RStyleOverlayColor");
const UILowerString CuiWidget3dObjectViewer::PropertyName::RStyleOverlayOpacity  = UILowerString ("RStyleOverlayOpacity");

const char * const CuiWidget3dObjectViewer::TypeName = "CuiWidget3dObjectViewer";

//-----------------------------------------------------------------

CuiWidget3dObjectViewer::CuiWidget3dObjectViewer () :
UIWidget              (),
UIEventCallback       (), //lint !e1769
UINotification        (), //lint !e1769
m_object              (0),
m_renderObject        (0),
m_camera              (0),
m_objectOriginOffsetY (0.0f),
m_objectTransform     (),
m_cameraTranslateMax  (),
m_cameraTranslateMin  (),
m_allowMouseWheelHACK (true),
//-- private
m_objectList          (0),
m_light               (0),
m_light_2             (0),
m_cameraTranslate     (),
m_objectYaw           (0),
m_objectPitch         (0),
m_lightPitch          (0.3f),
m_lightYaw            (1.8f),
m_objectPitchDefault  (0.0f),
m_objectYawDefault    (1.1f),
m_lightAmbientColor   (200, 200, 200, 255),
m_rotateSpeed         (0.02f),
m_defaultRotateSpeed  (0.02f),
m_zoom                (0.0f),
m_targetZoom          (0.0f),
m_lastDragPoint       (),
m_lastDragTime        (0),
m_autoComputeMinimumVectorFromExtentDirty (false),
m_objectSphereExtent  (),
m_objectNetworkId     (NetworkId::cms_invalid),
m_flags               (F_alterObject | F_mouseDraggable | F_paused),
m_textStyle           (0),
m_text                (),
m_lineWidth           (0L),
m_overlay             (0),
m_overlayColor        (UIColor::white),
m_overlayOpacity      (1.0f),
m_margin              (4L,4L,4L,4L)
{

	m_camera     = new ObjectListCamera (1);
	m_camera->setNearPlane (0.01f);
	m_objectList = new ObjectList (10);
	m_camera->addObjectList (m_objectList);

	m_light = new Light (Light::T_parallel, VectorArgb::solidWhite);

	m_light->setSpecularColor (VectorArgb::solidWhite);

	m_objectList->addObject (m_light);

	m_light_2 = new Light (Light::T_ambient, VectorArgb (1.0f, 0.2f, 0.2f, 0.2f));
		
	m_objectList->addObject (m_light_2);

	AddCallback (this);
}

//-----------------------------------------------------------------
	
CuiWidget3dObjectViewer::~CuiWidget3dObjectViewer ()
{
	setOverlay (0);

	m_objectList->removeObject (m_light);
	delete m_light;
	m_light = 0;

	m_objectList->removeObject (m_light_2);
	delete m_light_2;
	m_light_2 = 0;

	m_objectList->removeAll (false);

	m_camera->removeObjectList (m_objectList);
	delete m_objectList;
	delete m_camera;
	m_objectList = 0;
	m_camera = 0;

	if (hasFlags (F_ownsObject))
		delete m_object.getPointer ();

	m_object = 0;
	m_renderObject = 0;

	setTextStyle (0);
}

//----------------------------------------------------------------------

void  CuiWidget3dObjectViewer::Destroy( void )
{
	setPaused (true);
	RemoveCallback (this);
	UIWidget::Destroy ();
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::GetLinkPropertyNames (UIPropertyNameVector & In) const
{
	In.push_back( PropertyName::TextStyle);
	In.push_back (PropertyName::RStyleOverlay);
	In.push_back (PropertyName::RStyleOverlayColor);
	In.push_back (PropertyName::RStyleOverlayOpacity);

	UIWidget::GetLinkPropertyNames (In);
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{	
	In.push_back( PropertyName::ObjectOriginOffsetY );
	In.push_back( PropertyName::ObjectYaw );
	In.push_back( PropertyName::ObjectPitch );
	In.push_back( PropertyName::LightYaw );
	In.push_back( PropertyName::LightPitch );
	In.push_back( PropertyName::DefaultRotateSpeed );
	In.push_back( PropertyName::LightAmbientColor);
	In.push_back( PropertyName::MouseDraggable);
	In.push_back( PropertyName::AutoComputeView);
	In.push_back( PropertyName::DragRemoveObject);
	In.push_back( PropertyName::TextStyle);
	In.push_back( PropertyName::HeadShot);
	In.push_back (PropertyName::RStyleOverlay);
	In.push_back (PropertyName::RStyleOverlayColor);
	In.push_back (PropertyName::RStyleOverlayOpacity);

	UIWidget::GetPropertyNames (In, forCopy);
}

//-----------------------------------------------------------------

bool CuiWidget3dObjectViewer::SetProperty( const UILowerString & Name, const UIString &Value )
{

	if (Name == PropertyName::ObjectOriginOffsetY)
	{
		return UIUtils::ParseFloat (Value, m_objectOriginOffsetY);
	}
	else if (Name == PropertyName::DefaultRotateSpeed)
	{
		return UIUtils::ParseFloat (Value, m_defaultRotateSpeed);
	}
	else if (Name == PropertyName::ObjectYaw)
	{
		return UIUtils::ParseFloat (Value, m_objectYawDefault);
	}
	else if (Name == PropertyName::ObjectPitch)
	{
		return UIUtils::ParseFloat (Value, m_objectPitchDefault);
	}
	else if (Name == PropertyName::MouseDraggable)
	{
		bool b = false;
		if (!UIUtils::ParseBoolean (Value, b))
			return false;
		setMouseDraggable (b);
		return true;
	}
	else if (Name == PropertyName::AutoComputeView)
	{
		bool b = false;
		if (!UIUtils::ParseBoolean (Value, b))
			return false;
		setAutoComputeMinimumVectorFromExtent (b);
		return true;
	}
	else if (Name == PropertyName::DragRemoveObject)
	{
		bool b = false;
		if (!UIUtils::ParseBoolean (Value, b))
			return false;
		setDragRemoveObject (b);
		return true;
	}
	else if (Name == PropertyName::LightYaw)
	{
		if (UIUtils::ParseFloat (Value, m_lightYaw))
		{
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::LightPitch)
	{
		if (UIUtils::ParseFloat (Value, m_lightPitch))
		{
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::LightAmbientColor)
	{
		if (UIUtils::ParseColor (Value, m_lightAmbientColor))
		{
			VectorArgb theColor (
				static_cast<real>(m_lightAmbientColor.a) / 255.0f,
				static_cast<real>(m_lightAmbientColor.r) / 255.0f,	
				static_cast<real>(m_lightAmbientColor.g) / 255.0f,
				static_cast<real>(m_lightAmbientColor.b) / 255.0f);

			m_light_2->setDiffuseColor (theColor);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::TextStyle)
	{
		UITextStyle * const style = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);
		
		if( style || Value.empty() )
		{
			setTextStyle( static_cast<UITextStyle *>( style ) );
			return true;
		}
	}
	else if (Name == PropertyName::HeadShot)
	{
		bool b = false;
		if (!UIUtils::ParseBoolean (Value, b))
			return false;
		setHeadShot (b);
		return true;
	}
	else if (Name == PropertyName::RStyleOverlay)
	{
		UIRectangleStyle  * const rs = static_cast<UIRectangleStyle *>(GetObjectFromPath( Value, TUIRectangleStyle));

		if (rs || Value.empty ())
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

	return UIWidget::SetProperty (Name, Value);
}

//-----------------------------------------------------------------

bool CuiWidget3dObjectViewer::GetProperty( const UILowerString & Name, UIString &Value ) const
{

	if (Name == PropertyName::ObjectOriginOffsetY)
	{
		return UIUtils::FormatFloat (Value, m_objectOriginOffsetY);
	}
	else if (Name == PropertyName::DefaultRotateSpeed)
	{
		return UIUtils::FormatFloat (Value, m_defaultRotateSpeed);
	}
	else if (Name == PropertyName::ObjectYaw)
	{
		return UIUtils::FormatFloat (Value, m_objectYawDefault);
	}
	else if (Name == PropertyName::ObjectPitch)
	{
		return UIUtils::FormatFloat (Value, m_objectPitchDefault);
	}
	else if (Name == PropertyName::LightYaw)
	{
		return UIUtils::FormatFloat (Value, m_lightYaw);
	}
	else if (Name == PropertyName::LightPitch)
	{
		return UIUtils::FormatFloat (Value, m_lightPitch);
	}
	else if (Name == PropertyName::LightAmbientColor)
	{
		return UIUtils::FormatColor (Value, m_lightAmbientColor);
	}
	else if (Name == PropertyName::MouseDraggable)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_mouseDraggable));
	}
	else if (Name == PropertyName::AutoComputeView)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_autoComputeView));
	}
	else if (Name == PropertyName::DragRemoveObject)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_dragRemoveObject));
	}
	else if (Name == PropertyName::TextStyle)
	{
		if (m_textStyle)
		{
			Value = m_textStyle->GetLogicalName();
			return true;
		}
	}
	else if (Name == PropertyName::HeadShot)
	{
		return UIUtils::FormatBoolean (Value, hasFlags (F_headShot));
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
	return UIWidget::GetProperty (Name, Value);
}


//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::RenderStart   (UICanvas & canvas) const
{
	UIWidget::Render (canvas);
	
	if (m_renderObject == 0)
		return;
	
	CuiLayerRenderer::flushRenderQueue (); //-- do not use UICanvas::flush (), it is ignored by the Cui
	
	const UIPoint point (canvas.GetTranslation ());
	
	UISize normalSize (GetSize ());

	normalSize.x -= m_margin.left + m_margin.right;
	normalSize.y -= m_margin.top + m_margin.bottom;

	UIRect rect (m_margin.Location (), normalSize);
	
	const Appearance * const app = m_renderObject->getAppearance ();
	if (app)
	{		
		SkeletalAppearance2 * const skelApp = const_cast<SkeletalAppearance2 *>(dynamic_cast<const SkeletalAppearance2 *> (app));
		if (skelApp)
			skelApp->setShowMesh (true);			

		if (m_autoComputeMinimumVectorFromExtentDirty && hasFlags (F_autoComputeView))
		{
			
			const CreatureObject * const creature = dynamic_cast<const CreatureObject *>(m_renderObject.getPointer ());
			
			const UISize visibleSize (std::min (static_cast<int>(normalSize.x), Graphics::getCurrentRenderTargetWidth ()),
				std::min (static_cast<int>(normalSize.y), Graphics::getCurrentRenderTargetHeight ()));
			
			if (visibleSize.x <= 0 || visibleSize.y <= 0)
				return;
			
			m_camera->setViewport (0, 0, visibleSize.x, visibleSize.y);

			float horizontalFOV = PI_OVER_8;
			
			if (visibleSize.x < visibleSize.y)
			{
				horizontalFOV *= static_cast<float>(visibleSize.x) / static_cast<float>(visibleSize.y);
			}
			
			m_camera->setHorizontalFieldOfView (horizontalFOV);
	
			CuiWidget3dObjectViewer * const nonconst_this = const_cast<CuiWidget3dObjectViewer *>(this);
			
			const Extent * extent = 0;
			if (skelApp)
			{
				extent = &(skelApp->getExactMeshExtent ());
			}
			else
				extent = app->getExtent ();
			
			if (!extent)
			{
				WARNING (true, ("null extent for appearance: %s", NON_NULL (app->getAppearanceTemplate ())->getName ()));
				return;
			}
			
			nonconst_this->m_cameraTranslateMin = computeMinimumVectorFromExtent (*extent);
			
			if (hasFlags (F_headShot) && creature)
			{
				const BoxExtent * const box = dynamic_cast<const BoxExtent *>(extent);
				if (box)
				{
					const Sphere & oldSphere = box->getSphere ();
					Extent newExtent;
					
					float t = std::min (1.0f, 2.0f - static_cast<float>(visibleSize.y) / static_cast<float>(visibleSize.x));
					const float radius = linearInterpolate (0.15f, 0.15f, t);
					t = linearInterpolate (0.7f, 0.80f, t);

					newExtent.setSphere (
						Sphere (
						oldSphere.getCenter () + (Vector::unitY * oldSphere.getRadius () * t),
						oldSphere.getRadius () * radius));//(1.0f - t)));
										
					//-- todo: pass the extent of the head from the skeletal appearance
					nonconst_this->m_cameraTranslateMax = computeMinimumVectorFromExtent (newExtent);
				}
			}
			
			else
			{
				nonconst_this->m_cameraTranslateMax = nonconst_this->m_cameraTranslateMin;
				nonconst_this->m_cameraTranslateMax.z *= 0.5f;
				//				nonconst_this->m_cameraTranslateMax.y *= 1.8f;
			}
			
			//- force the zoom parameters to reset
			if (creature && hasFlags (F_headShot) && hasFlags (F_headZoom))
			{
				nonconst_this->setZoom (1.0f);
			}
			else
			{
				nonconst_this->setZoom (m_zoom);
			}
			
			m_autoComputeMinimumVectorFromExtentDirty = false;
		}
	}

	UIRect clip;
	canvas.GetClip (clip);
	
	if (!UIUtils::ClipRect (rect, clip))
		return;
	
	DEBUG_FATAL (rect.Width () <= 0 || rect.Height () <= 0, ("cliprect fubar'd\n"));
	
	rect += point;
	
	if (!UIUtils::ClipRect (rect, UIRect (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ())))
		return;
	
	DEBUG_FATAL (rect.Width () <= 0 || rect.Height () <= 0, ("cliprect fubar'd\n"));
	
	m_camera->setViewport (rect.left, rect.top, rect.Width (), rect.Height ());
	
	const UISize visibleSize (rect.Size ());
	
	float horizontalFOV = PI_OVER_8;

	if (visibleSize.x < visibleSize.y)
	{
		horizontalFOV *= static_cast<float>(visibleSize.x) / static_cast<float>(visibleSize.y);
	}

	m_camera->setHorizontalFieldOfView (horizontalFOV);

	//-----------------------------------------------------------------
	
	m_camera->resetRotateTranslate_o2p ();
	
	m_camera->setTransform_o2p (m_renderObject->getTransform_o2w ());

	m_camera->yaw_o   (m_objectYaw);
	m_camera->pitch_o (m_objectPitch);

	m_light->setTransform_o2p (m_renderObject->getTransform_o2w ());
	m_light->yaw_o   (m_lightYaw * PI   + m_objectYaw);
	m_light->pitch_o (m_lightPitch * PI + m_objectPitch);

	m_camera->move_o  (m_cameraTranslate);

	m_camera->move_o (-Vector (0.0f, m_objectSphereExtent.getSphere ().getRadius () * m_objectOriginOffsetY, 0.0f));

	m_objectTransform = m_renderObject->getTransform_o2w ();

	Graphics::clearViewport (false, 0, true, 1.0f, true, 0);

	m_objectList->addObject (m_renderObject);

	const GlFillMode oldFillMode = Graphics::getFillMode();

	if (hasFlags (F_wireFrame))
		Graphics::setFillMode (GFM_wire);

	m_camera->renderScene ();

	if (hasFlags (F_wireFrame))
		Graphics::setFillMode (oldFillMode);

	m_objectList->removeObject (m_renderObject);

	if (hasFlags (F_drawExtent))
	{
		Graphics::setObjectToWorldTransformAndScale (m_objectTransform, Vector::xyz111);
		Graphics::setStaticShader (ShaderTemplateList::get3dVertexColorZStaticShader ());
		Graphics::drawExtent (m_renderObject->getAppearance ()->getExtent (), VectorArgb::solidWhite);
	}

	if (hasFlags (F_drawName) && m_textStyle && !m_text.empty ())
	{
		const UIPoint pos (0, GetHeight () - m_textStyle->GetLeading ());

		UITextStyle::RenderLineData rld (&canvas, UITextStyle::Center, pos, GetWidth (), 0, 0, UIColor::white);
		rld.truncateElipsis = true;
		rld.begin           = m_text.begin ();
		rld.nextLine        = m_text.end   ();
		rld.lineWidth       = m_lineWidth;

		m_textStyle->RenderPrewrappedLine (rld);
	}

	//----------------------------------------------------------------------
	//-- render overlay
	
	if (m_overlay)
	{
		const UIColor preBackgroundColor (canvas.GetColor ());
		const float OldOpacity   = canvas.GetOpacity();
		
		canvas.ModifyColor (m_overlayColor);
		canvas.ModifyOpacity( m_overlayOpacity);
				
		UISize scrollExtent;
		GetScrollExtent (scrollExtent);
		m_overlay->Render (GetAnimationState (), canvas, scrollExtent);

		canvas.SetColor (preBackgroundColor);
		canvas.SetOpacity( OldOpacity );
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::RenderStop () const
{
	if (m_renderObject == 0)
		return;
	
	Graphics::setViewport (0, 0, Graphics::getCurrentRenderTargetWidth (), Graphics::getCurrentRenderTargetHeight ());

}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::Render (UICanvas & canvas) const
{
	RenderStart   (canvas);
	RenderStop    ();
}

//----------------------------------------------------------------------

/**
* Force an unpause.
*/

void CuiWidget3dObjectViewer::OnShow( UIWidget *Context )
{
	if (Context == this)
		setPaused (false);
} //lint !e818 //stfu noob

//----------------------------------------------------------------------

/**
* Force a pause.
*/

void CuiWidget3dObjectViewer::OnHide( UIWidget *Context )
{
	if (Context == this)
		setPaused (true);
} //lint !e818 //stfu noob

//-----------------------------------------------------------------

bool CuiWidget3dObjectViewer::OnMessage( UIWidget *Context, const UIMessage & msg )
{
	if (m_renderObject == 0)
		return true;

	UNREF (Context);

	if (msg.Type == UIMessage::MouseWheel)
	{
		if (hasFlags (F_mouseDraggable) || m_allowMouseWheelHACK)
		{
			setTargetZoom (getTargetZoom () - 0.1f * msg.Data);
			setZoomInterpolating (true);
			return false;
		}
	}

	else if (msg.Type == UIMessage::MouseMove && hasFlags (F_dragging))
	{
		Vector delta;
		delta.x = static_cast<real>(msg.MouseCoords.x - m_lastDragPoint.x);
		delta.y = static_cast<real>(msg.MouseCoords.y - m_lastDragPoint.y);

		const float frameTime = Clock::frameTime ();
		static const float SENSITIVITY_ROTATE_YAW = 0.01f;
		static const float SENSITIVITY_ZOOM       = 0.001f;

		// rotate 
		if (msg.Modifiers.LeftMouseDown)
		{ 
			if (delta.x)
			{	
				//-- reverse the delta to match yaw angle
				delta.x *= SENSITIVITY_ROTATE_YAW;

				m_rotateSpeed = frameTime != 0.0f ? delta.x / frameTime : delta.x;
				m_objectYaw  += delta.x;
			}

			if (hasFlags (F_pitchEnabled))
				m_objectPitch = std::min (0.5f, std::max (-0.5f, m_objectPitch + delta.y));

			m_lastDragPoint = msg.MouseCoords;
			m_lastDragTime  = UIClock::gUIClock ().GetTime ();
			return true;			
		}

		// zoom camera
		else if (msg.Modifiers.RightMouseDown)
		{
			//-- zoom
			if (delta.x)
			{
				//-- zoom ranges from 0.0 to 1.0, 0.0 being the most distant view
				delta.x *= SENSITIVITY_ZOOM;
				real zoom  = m_zoom - delta.x;
				setZoom (zoom);
				setZoomInterpolating (false);
			}

			m_lastDragPoint = msg.MouseCoords;
			m_lastDragTime = UIClock::gUIClock ().GetTime ();
			return true;
		}
		//-----------------------------------------------------------------
		//-- free controls (testing purposes)

		else if (msg.Modifiers.MiddleMouseDown)
		{

		}

		else
		{
			unsetFlags (F_dragging);
		}

	}
	
	else if (msg.Type == UIMessage::LeftMouseDown || msg.Type == UIMessage::RightMouseDown || msg.Type == UIMessage::MiddleMouseDown)
	{
		if (hasFlags (F_mouseDraggable))
		{
			if (msg.Type == UIMessage::LeftMouseDown)
				m_rotateSpeed = 0.0f;
			
			m_lastDragPoint = msg.MouseCoords;
			setFlags (F_dragging);
			return false;
		}
	}
	else if (msg.Type == UIMessage::LeftMouseUp || msg.Type == UIMessage::RightMouseUp || msg.Type == UIMessage::MiddleMouseUp)
	{
		if (hasFlags (F_dragging))
		{
			
			//-----------------------------------------------------------------
			//-- stop rotating if the user pauses dragging
			
			if ((UIClock::gUIClock ().GetTime () - m_lastDragTime) > 10)
				m_rotateSpeed = 0.0f;
			
			unsetFlags (F_dragging);
		}
	}
	
	return true;
}

//-----------------------------------------------------------------

ClientObject * CuiWidget3dObjectViewer::setObject (ClientObject * object, const Vector & cameraOffsetMin, const Vector & cameraOffsetMax)
{
	ClientObject * const oldObject = m_object;

	m_object = object;
	m_renderObject = object;

	const Appearance * const app = m_renderObject ? m_renderObject->getAppearance () : 0;

	if (m_renderObject && !app)
	{
		WARNING (true, ("Attempt to set CuiWidget3dObjectViewer object with no appearance."));
		m_renderObject = 0;
	}
	
	if (dynamic_cast<ManufactureSchematicObject *>(object))
		setFlags (F_wireFrame);
	else
		unsetFlags (F_wireFrame);

	if (m_renderObject)
	{
		m_objectNetworkId = m_renderObject->getNetworkId ();

		m_objectSphereExtent.setSphere (app->getSphere ());
		
		//----------------------------------------------------------------------
		//-- hack to force an update
		if (hasFlags (F_alterObject))
		{
			if (dynamic_cast<CreatureObject *> (object))
			{		
				if (m_renderObject->alter    (1.0f))
					m_renderObject->conclude ();
				else
					m_renderObject = 0;
			}			
		}
		
		m_objectYaw       = m_objectYawDefault * PI;
		m_objectPitch     = m_objectPitchDefault * PI;
				
		if (hasFlags (F_autoComputeView))
			m_autoComputeMinimumVectorFromExtentDirty = true;
		else
		{
			m_cameraTranslateMin.x = 0.0f;
			m_cameraTranslateMin.y = cameraOffsetMin.y * m_objectSphereExtent.getSphere ().getRadius ();
			m_cameraTranslateMin.z = cameraOffsetMin.z * m_objectSphereExtent.getSphere ().getRadius ();
		}
				
		m_cameraTranslateMax.x = 0.0f;
		m_cameraTranslateMax.y = cameraOffsetMax.y * m_objectSphereExtent.getSphere ().getRadius ();
		m_cameraTranslateMax.z = cameraOffsetMax.z * m_objectSphereExtent.getSphere ().getRadius ();

		//-- set the camera zoom all the way out
		m_cameraTranslate = m_cameraTranslateMin;
		m_zoom = 0.0f;
		m_rotateSpeed = m_defaultRotateSpeed;
	}

	else
	{
		m_objectNetworkId = NetworkId::cms_invalid;
	}

	if (m_object)
		m_text = m_object->getLocalizedName ();
	else
		m_text.clear ();

	recomputeLineWidth ();

	SendNotification( UINotification::ObjectChanged, this);

	return oldObject;
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::Notify( UINotificationServer *notifyingObject, UIBaseObject *contextObject, Code notificationCode )
{
	UNREF (notifyingObject);
	UNREF (contextObject);
	UNREF (notificationCode);

	if (m_renderObject)
	{ 
		Attach (0);

		const float frameTime = Clock::frameTime ();

		if (!hasFlags (F_dragging))
		{
			static const real MAX_ROTATE_SPEED = 5.00f;

			if (fabs (m_rotateSpeed) > MAX_ROTATE_SPEED)
				m_rotateSpeed *= 0.9f;
				
			m_objectYaw += m_rotateSpeed * frameTime;
		}


		//----------------------------------------------------------------------
		//-- smoothly zoom

		if (hasFlags (F_zoomInterpolating))
		{
			static const real ZOOM_INTERPOLATING_THRESHOLD = 0.001f;

			const real diff = m_zoom - m_targetZoom;
			if (fabs (diff) < ZOOM_INTERPOLATING_THRESHOLD)
			{
				m_zoom = m_targetZoom;
				unsetFlags (F_zoomInterpolating);
			}
			else
			{
				setZoom (m_zoom - diff * frameTime * 5.0f);
			}
		}
		
		if (hasFlags (F_alterObject))
		{
			if (m_renderObject->alter    (frameTime))
				m_renderObject->conclude ();
			else
				m_renderObject = 0;
		}
		
		Detach (0);
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setZoom (real z)
{
	m_zoom = std::min (std::max (0.0f, z), 1.0f);
	m_cameraTranslate.z = m_cameraTranslateMin.z + (m_cameraTranslateMax.z - m_cameraTranslateMin.z) * m_zoom;
	m_cameraTranslate.y = m_cameraTranslateMin.y + (m_cameraTranslateMax.y - m_cameraTranslateMin.y) * m_zoom;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setTargetZoom (real z)
{
	m_targetZoom = std::min (std::max (0.0f, z), 1.0f);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setPitchEnabled (bool b)
{
	setUnsetFlags (F_pitchEnabled, b);
	m_objectPitch = m_objectPitchDefault * PI;
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::setPaused (bool b)
{
	if (b == hasFlags (F_paused)) //lint !e731
		return;

	setUnsetFlags (F_paused, b);

	if (b)
		UIClock::gUIClock().StopListening( this );
	else
		UIClock::gUIClock().ListenPerFrame ( this );
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::setObjectYaw (float y)
{
	m_objectYaw = y;
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::setRotateSpeed (float r)
{
	m_rotateSpeed = r;
}

//----------------------------------------------------------------------

const Vector CuiWidget3dObjectViewer::computeMinimumVectorFromExtent (const Extent & extent) const
{
	const_cast<Extent &>(m_objectSphereExtent).setSphere (extent.getSphere ());

	const BoxExtent * const box = dynamic_cast<const BoxExtent *>(&extent);

	float radius = 0.0f;
	Vector2d rect;

	if (box)
	{
		//-- contruct a cylinder

		const Vector bmin (box->getMin ());
		const Vector bmax (box->getMax ());

		radius = std::max (Vector2d (bmin.x, bmin.z).magnitude (), Vector2d (bmax.x, bmax.z).magnitude ());
		rect.y    = (bmax.y - bmin.y);
		rect.x    = radius * 2;
	}
	else
	{
		radius = extent.getSphere ().getRadius ();
		rect.x = rect.y = radius * 2.0f;
	}

	//-- x,y of the vector should be at the center of the object.
	//-- now determine z using similar triangles
	
	//-- NOTE: these calculations assume that the frustum is not tilted
	const real screenHeight = (m_camera->getFrustumVertex (Camera::FV_NearUpperLeft).y - m_camera->getFrustumVertex (Camera::FV_NearLowerLeft).y);
	const real height_z     = radius + (rect.y * m_camera->getNearPlane () / screenHeight);
	
	const real screenWidth = m_camera->getFrustumVertex (Camera::FV_NearUpperRight).x - m_camera->getFrustumVertex (Camera::FV_NearUpperLeft).x;
	const real width_z     = radius + (rect.x * m_camera->getNearPlane () / screenWidth);
	
	Vector result (extent.getSphere ().getCenter ());
	result.z -= std::max (width_z, height_z);
	
	return result;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setAutoComputeMinimumVectorFromExtent (bool b)
{
	setUnsetFlags (F_autoComputeView, b);
	m_autoComputeMinimumVectorFromExtentDirty = true;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setMouseDraggable (bool b)
{
	setUnsetFlags (F_mouseDraggable, b);
	m_allowMouseWheelHACK = b;
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::SetSize( const UISize & size)
{
	UIWidget::SetSize (size);
	m_autoComputeMinimumVectorFromExtentDirty = true;
}

//----------------------------------------------------------------------

UIWidget * CuiWidget3dObjectViewer::GetCustomDragWidget (const UIPoint & point, UIPoint & offset)
{
	UNREF (point);
	
	if (!m_renderObject)
		return 0;

	if (hasFlags (F_dragRemoveObject))
	{
		CuiWidget3dObjectViewer * const dupe = safe_cast<CuiWidget3dObjectViewer *>(DuplicateObject ());
		NOT_NULL (dupe);
		dupe->SetLocation (point);
		dupe->SetParent (GetParent ());
		dupe->Link ();
		dupe->SetParent (0);
		dupe->setObject (getObject (), Vector (), Vector ());
		setObject (0, Vector (), Vector ());
		return dupe;
	}
	else
	{
		return UIWidget::GetCustomDragWidget(point, offset);
	}
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setFlags (Flags f)
{
	m_flags |= f;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::unsetFlags (Flags f)
{
	m_flags &= ~f;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setUnsetFlags (Flags f, bool set)
{
	if (set)
		setFlags (f);
	else
		unsetFlags (f);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setZoomInterpolating (bool b)
{
	setUnsetFlags (F_zoomInterpolating, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setOwnsObject (bool b)
{
	setUnsetFlags (F_ownsObject, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setAlterObject (bool b)
{
	setUnsetFlags (F_alterObject, b);
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setDragRemoveObject (bool b)
{
	setUnsetFlags (F_dragRemoveObject, b);
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::setDrawName (bool b)
{
	setUnsetFlags (F_drawName, b);
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::setTextStyle (UITextStyle * style)
{
	if (AttachMember (m_textStyle, style))
	{
		recomputeLineWidth ();
	}
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::setHeadShot (bool b)
{
	setUnsetFlags (F_headShot, b);
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::setHeadZoom (bool b)
{
	setUnsetFlags (F_headZoom, b);
}

//-----------------------------------------------------------------

void CuiWidget3dObjectViewer::recomputeLineWidth ()
{
	if (m_textStyle && !m_text.empty ())
	{
		long dummyHeight = 0L;
		m_textStyle->MeasureText (m_text, m_lineWidth, dummyHeight);
	}
	else
		m_lineWidth = 0;
}

//----------------------------------------------------------------------

void CuiWidget3dObjectViewer::setOverlay (UIRectangleStyle * rs)
{
	AttachMember (m_overlay, rs);
}

// ======================================================================
