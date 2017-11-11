//======================================================================
//
// CuiWidget3dObjectListViewer.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiWidget3dObjectListViewer_H
#define INCLUDED_CuiWidget3dObjectListViewer_H

//======================================================================

#include "UINotification.h"
#include "UIWidget.h"
#include "clientGame/ClientObject.h"
#include "sharedCollision/Extent.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/MemoryBlockManagerMacros.h"

class ObjectWatcherListCamera;
class ObjectWatcherList;
class Light;
class ClientObject;
class Extent;
class UITextStyle;
class BoxExtent;
class SkeletalAppearance2;
class ShaderTemplate;
class CuiObjectDataSource;
class UIDataSource;
class MemoryBlockManager;
class Texture;

// ======================================================================

/**
* CuiWidget3dObjectListViewer is a widget that allows viewing of some Objects.
*
*/

class CuiWidget3dObjectListViewer : 
public UIWidget, 
public UINotification
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:
	static const char * const TypeName;

	struct PropertyName
	{
		static const UILowerString AutoZoomOutOnly;
		static const UILowerString CameraAutoZoom;
		static const UILowerString CameraCompensateScale;
		static const UILowerString CameraFitBone;
		static const UILowerString CameraLodBias;
		static const UILowerString CameraLodBiasOverride;
		static const UILowerString CameraLookAt;
		static const UILowerString CameraLookAtBone;
		static const UILowerString CameraLookAtCenter;
		static const UILowerString CameraPitch;
		static const UILowerString CameraRoll;
		static const UILowerString CameraTransformToObj;
		static const UILowerString CameraYaw;
		static const UILowerString CameraForceTarget;
		static const UILowerString DragPitchMax;
		static const UILowerString DragPitchMin;
		static const UILowerString DragPitchOk;
		static const UILowerString DragYawOk;
		static const UILowerString DrawName;
		static const UILowerString FieldOfView;
		static const UILowerString FitDistanceFactor;
		static const UILowerString FitRect;
		static const UILowerString LightAmbientColor;
		static const UILowerString LightColor;
		static const UILowerString LightColor2;
		static const UILowerString LightLockToCamera;
		static const UILowerString LightPitch;
		static const UILowerString LightPitch2;
		static const UILowerString LightYaw;
		static const UILowerString LightYaw2;
		static const UILowerString LookAtBoneOnlyY;
		static const UILowerString ObjectDataSource;
		static const UILowerString OverrideShader;
		static const UILowerString OverrideShaderTexture;
		static const UILowerString Paused;
		static const UILowerString RStyleOverlay;
		static const UILowerString RStyleOverlayColor;
		static const UILowerString RStyleOverlayOpacity;
		static const UILowerString RenderObjectEffects;
		static const UILowerString Rotation;
		static const UILowerString ScaleMaxOverride;
		static const UILowerString Shadows;
		static const UILowerString TextStyleBottom;
		static const UILowerString TextStyleTop;
	};

	struct MethodName
	{
		static const UILowerString SetObject;
		static const UILowerString ClearObjects;
	};

	enum Flags
	{
		F_alterObjects           = 0x000001,
		F_autoZoomOutOnly        = 0x000002,
		F_cameraCompensateScale  = 0x000004,
		F_cameraFitBone          = 0x000008,
		F_cameraForceTarget      = 0x000010,
		F_cameraInterpolate      = 0x000020,
		F_cameraLookAtBoneDirty  = 0x000040,
		F_dragYawOk              = 0x000080,
		F_dragging               = 0x000100,
		F_draggingDebug          = 0x000200,
		F_drawExtent             = 0x000400,
		F_paused                 = 0x000800,
		F_viewDirty              = 0x001000,
		F_wireFrame              = 0x002000,
		F_lookAtBoneOnlyY        = 0x004000,
		F_cameraLookAtCenter     = 0x008000,
		F_useOverrideShader      = 0x010000,
		F_useShadows             = 0x020000,
		F_drawName               = 0x040000,
		F_cameraTransformToObj   = 0x080000,
		F_dragPitchOk            = 0x100000,
		F_lightLockToCamera      = 0x200000,
		F_sizeChanged            = 0x400000,
		F_cameraLodBiasOverride  = 0x800000,
		F_cameraAutoZoom         = 0x1000000,
		F_cameraZoomInWhileTurn  = 0x2000000,
		F_useOverrideShaderTexture = 0x4000000, // Renders the object once with the override shader.
		F_rotateNeedsControlKey = 0x8000000
	};

	class TextOverlay;

	enum TextOverlayTypes
	{
		TOT_topRight,
		TOT_topLeft,
		TOT_bottom,
		TOT_numTextOverlays,
	};

	                          CuiWidget3dObjectListViewer (); 
	virtual                  ~CuiWidget3dObjectListViewer ();
	virtual UIBaseObject *    Clone                       () const { return new CuiWidget3dObjectListViewer; }
	virtual	UIStyle *         GetStyle                    () const { return 0; }
	virtual void              Destroy                     ();

	virtual void              Render                      (UICanvas & ) const;
	virtual bool              ProcessMessage              (const UIMessage & );
	virtual void              SetSize                     (const UISize & );
	
	const char *              GetTypeName                 () const { return TypeName; }

	int                       getRenderObjectCount        () const;
	Object *                  getRenderObjectByIndex      (int index);
	const Object *            getRenderObjectByIndex      (int index) const;

	const ObjectWatcherList & getObjectWatcherList        () const;

	virtual void              Notify                      (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );
	virtual bool              SetProperty                 (const UILowerString & Name, const UIString &Value );
	virtual bool              GetProperty                 (const UILowerString & Name, UIString &Value ) const;

	virtual void              SetVisible                  (bool b);
	virtual void              GetPropertyNames            (UIPropertyNameVector &, bool forCopy ) const;
	virtual void              GetLinkPropertyNames        (UIPropertyNameVector & in) const;

	virtual UIWidget *        GetCustomDragWidget         (const UIPoint & point, UIPoint & offset);

	const Light &             getLight                 ();

	void                      setPaused                (bool b);
	void                      setAlterObjects          (bool b);
	void                      setViewDirty             (bool b);
	void                      setCameraForceTarget     (bool b);
	void                      setCameraLookAtBoneDirty (bool b);
	void                      setCameraCompensateScale (bool b);
	void                      setAutoZoomOutOnly       (bool b);
	void                      setCameraFitBone         (bool b);
	void                      setCameraZoomInWhileTurn (bool b);

	void                      setRenderObjectEffects   (bool b);
	bool                      getRenderObjectEffects   () const;

	void                      setRotationSlowsToStop(bool b);
	void                      setForceDefaultClippingPlanes(bool b);
	void                      setIgnoreMouseWheel(bool b);

	bool                      hasFlags                 (Flags f) const;

	bool                      checkAppearancesReady    () const;

	const float               computeFitDistance       (bool & appearancesReady) const;

	void setCameraLookAt(const Vector & v, bool stopRotating = true);
	Vector getCameraFrame_k() const;
	bool findWorldLocation(UIPoint const & pointLocalToControl, Vector & begin_w, Vector & end_w) const;

	void                      setCameraYaw             (float f, bool stopRotating = true);
	void                      setCameraYawTarget       (float f, bool stopRotating = true);
	float                     getCameraYaw             () const;
	void                      setCameraPitch           (float f);
	float                     getCameraPitch           () const;

	void                      setCameraRoll            (float f);
	float                     getCameraRoll            () const;

	void                      setLightYaw              (float radians);
	void                      setLightYaw2             (float radians);

	const ClientObject *      getObjectAt              (const UIPoint & pt) const;

	void                      computeFitPointCloud     (stdvector<Vector>::fwd & vv, bool & appearancesReady) const;

	void                      forceUpdate              ();

	bool                      findScreenLocation       (const ClientObject & obj, const Vector & objectLocation, UIPoint & screenLocation);

	void                      clearObjects             ();
	void                      addObject                (Object & obj);
	void                      removeObject             (Object & obj);
	void                      setObject                (Object * obj);

	void                      setFitRect               (const UIRect & rect);

	Object *                  getLastObject            ();
	const Object *            getLastObject            () const;

	Object *                  getLastRenderObject      ();
	const Object *            getLastRenderObject      () const;

	void                      setCameraLookAtBone      (const std::string & bone);
	const std::string &       getCameraLookAtBone      () const;
	void                      setCameraZoomLookAtBone  (const std::string & bone);

	void                      setCameraLookAtCenter    (bool b);

	void                      recomputeZoom            ();

	float                     getScaleMaxOverride      () const;
	void                      setScaleMaxOverride      (float f);

	void                      setUseOverrideShader     (const std::string & name, bool b);

	void                      setObjectDataSource      (UIDataSource * ds);

	typedef std::pair<Watcher<Object>, Watcher<Object> > ObjectPair;
	typedef stdvector<ObjectPair>::fwd ObjectRenderVector;

	void                      setRotateSpeed           (float f);
	float					  getRotateSpeed		   () const;
	void                      setDragYawOk             (bool b);
	void                      setDragPitchOk           (bool b);

	void                      setText                  (TextOverlayTypes tot, const Unicode::String & str, const UIColor & color);
	void                      setTextStyle             (TextOverlayTypes tot, UITextStyle * style);
	void                      setTextFadeOut           (TextOverlayTypes tot, bool b);
	void                      setOverlay               (UIRectangleStyle * rs);
	void                      setDrawName              (bool b);
	void addText3d(Vector const & location, UIString const & text, UIColor const & color = UIColor::white, float backgroundOpacity = 1.0f);
	void clearText3d();

	void                      setCameraAutoZoom        (bool b);

	void                      setCameraTransformToObj  (bool b);

	float                     getFitDistanceFactor     () const;
	void                      setFitDistanceFactor     (float f);
	void                      setCameraLodBias         (float f);
	void                      setCameraLodBiasOverride (bool b);
	void                      setRotateNeedsControlKey(bool b);

	void setEnvironmentTexture(Texture const * texture);

	void                      computeTransformedBoxExtent (const Object & obj, BoxExtent & boxExtent, bool debugRender) const;

	virtual bool              CanSelect() const;
	
	static void               install ();
	static void               remove  ();

	virtual void OnSizeChanged(UISize const & newSize, UISize const & oldSize);
	virtual bool IsA(UITypeID const Type ) const { return (Type == TUI3DObjectListViewer) || UIWidget::IsA( Type ); }

	void					  setNoCameraClamp   (bool forceCamera);

private:
	                          CuiWidget3dObjectListViewer (const CuiWidget3dObjectListViewer & rhs);
	                          CuiWidget3dObjectListViewer & operator= (const CuiWidget3dObjectListViewer & rhs);

	void                      setFlags                   (Flags f);
	void                      unsetFlags                 (Flags f);
	void                      setUnsetFlags              (Flags f, bool set);
	void                      debugRender                () const;
	bool                      computeSkeletalBoundingBox (const SkeletalAppearance2 & skelApp, BoxExtent & box) const;
	void                      gotoCameraPivotPoint       (bool useTarget) const;
	void                      updateObjectName           ();
	void                      checkRenderLimits          ();
	void recomputeNearAndFarPlanes();

protected:

	ObjectWatcherListCamera * m_camera;

	Vector                    m_cameraLookAt;
	mutable Vector            m_cameraLookAtTarget;
	mutable Vector            m_cameraYawPitchZoom;
	mutable float             m_cameraRoll;
	mutable Vector            m_cameraYawPitchZoomLast;
	mutable Vector            m_cameraYawPitchZoomTarget;

	void                      RenderStart   (UICanvas & canvas) const;
	void                      RenderStop    () const;
	void					  RenderText	(UICanvas & canvas) const;

private:

	ObjectRenderVector * m_objectVector;

	ObjectWatcherList *       m_objectWatcherList;
	Light *                   m_light;
	Light *                   m_light2;
	Light *                   m_lightAmbient;

	float                     m_lightPitch;
	float                     m_lightPitch2;
	float                     m_lightYaw;
	float                     m_lightYaw2;
	VectorArgb                m_lightAmbientColor;
	VectorArgb                m_lightColor;
	VectorArgb                m_lightColor2;

	mutable uint32            m_flags;
	UIPoint                   m_lastMousePoint;

	float                     m_fieldOfView;

	mutable UIRect            m_viewport;

	UIRect                    m_fitRect;

	std::string               m_cameraLookAtBone;
	std::string               m_cameraZoomLookAtBone;
	float                     m_zoomBoneInterpFactor;

	Vector                    m_cameraDeflectionAngle;

	mutable bool              m_cameraLookAtBoneOk;

	float                     m_rotateSpeed;
	uint32                    m_lastDragTime;

	float                     m_scaleMaxOverride;

	int                       m_viewDirtyFrame;

	ShaderTemplate const * m_overrideShaderTemplate;

	float                     m_fitDistanceFactor;

	CuiObjectDataSource *     m_objectDataSource;

	class MyObjectDataSourceCallback;
	MyObjectDataSourceCallback * m_objectDataSourceCallback;

	UIRectangleStyle *        m_overlay;
	UIColor                   m_overlayColor;
	float                     m_overlayOpacity;

	UIRect                    m_margin;

	float                     m_dragPitchMax;
	float                     m_dragPitchMin;
	float                     m_dragPitchMinActual;

	mutable UISize            m_lastViewportSize;

	TextOverlay *             m_textOverlays [TOT_numTextOverlays];

	float                     m_cameraLodBias;

	int                       m_numCreatures;
	float                     m_zoomInterpolationRate;

	float                     m_lastNameUpdateSecs;

	int                       m_maximumDesiredDetailLevel;

	mutable Vector m_fitScale;

	struct Text3d;
	typedef std::vector<Text3d *> Text3dVector;
	Text3dVector * m_text3d;
	bool m_renderObjectEffects : 1;
	bool m_rotationSlowsToStop : 1;
	bool m_forceDefaultClippingPlanes : 1;
	bool m_ignoreMouseWheel : 1;
	bool m_noCameraClamp;
};

//-----------------------------------------------------------------

inline const Light & CuiWidget3dObjectListViewer::getLight ()
{
	return *NON_NULL (m_light);
}

//----------------------------------------------------------------------

inline bool CuiWidget3dObjectListViewer::hasFlags (Flags f) const
{
	return (m_flags & f) != 0;
}

//----------------------------------------------------------------------

inline const ObjectWatcherList & CuiWidget3dObjectListViewer::getObjectWatcherList () const
{
	return *NON_NULL (m_objectWatcherList);
}

//----------------------------------------------------------------------

inline float CuiWidget3dObjectListViewer::getCameraYaw () const
{
	return m_cameraYawPitchZoom.x;
}

//----------------------------------------------------------------------

inline float CuiWidget3dObjectListViewer::getScaleMaxOverride () const
{
	return m_scaleMaxOverride;
}

//----------------------------------------------------------------------

inline const std::string & CuiWidget3dObjectListViewer::getCameraLookAtBone      () const
{
	return m_cameraLookAtBone;
}

//----------------------------------------------------------------------

inline float CuiWidget3dObjectListViewer::getCameraPitch () const
{
	return m_cameraYawPitchZoom.y;
}

//----------------------------------------------------------------------

inline float CuiWidget3dObjectListViewer::getCameraRoll () const
{
	return m_cameraRoll;
}

//----------------------------------------------------------------------

inline void CuiWidget3dObjectListViewer::setRenderObjectEffects(bool b)
{
	m_renderObjectEffects = b;
}

//----------------------------------------------------------------------

inline bool CuiWidget3dObjectListViewer::getRenderObjectEffects() const
{
	return m_renderObjectEffects;
}

//----------------------------------------------------------------------

inline void CuiWidget3dObjectListViewer::setRotationSlowsToStop(bool b)
{
	m_rotationSlowsToStop = b;
}

//----------------------------------------------------------------------

inline void CuiWidget3dObjectListViewer::setForceDefaultClippingPlanes(bool b)
{
	m_forceDefaultClippingPlanes = b;
}

//----------------------------------------------------------------------

inline void CuiWidget3dObjectListViewer::setIgnoreMouseWheel(bool b)
{
	m_ignoreMouseWheel = b;
}

//----------------------------------------------------------------------

inline void CuiWidget3dObjectListViewer::setNoCameraClamp (bool forceCamera)
{
  m_noCameraClamp = forceCamera;
}

//----------------------------------------------------------------------

inline float CuiWidget3dObjectListViewer::getRotateSpeed() const
{
	return m_rotateSpeed;
};

//======================================================================

#endif
