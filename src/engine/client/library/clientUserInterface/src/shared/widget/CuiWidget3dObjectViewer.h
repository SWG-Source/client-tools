// ======================================================================
//
// CuiWidget3dObjectViewer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiWidget3dObjectViewer_H
#define INCLUDED_CuiWidget3dObjectViewer_H

#include "UIEventCallback.h"
#include "UINotification.h"
#include "UIWidget.h"
#include "sharedCollision/Extent.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientGame/ClientObject.h"

class ObjectListCamera;
class ObjectList;
class Light;
class ClientObject;
struct UIColor;
class Extent;
class UITextStyle;

// ======================================================================

/**
* CuiWidget3dObjectViewer is a widget that allows viewing, rotation, and zooming of
* an Object.
*
* Currently the zooming code is hackish and tends to cut off the heads of the models.
*
*/
class CuiWidget3dObjectViewer : 
public UIWidget, 
public UIEventCallback, 
public UINotification,
public UINotificationServer
{
public:
	static const char * const TypeName;

	struct PropertyName
	{
		static const UILowerString ObjectOriginOffsetY;
		static const UILowerString ObjectYaw;
		static const UILowerString ObjectPitch;
		static const UILowerString LightYaw;
		static const UILowerString LightPitch;
		static const UILowerString LightAmbientColor;
		static const UILowerString DefaultRotateSpeed;
		static const UILowerString MouseDraggable;
		static const UILowerString AutoComputeView;
		static const UILowerString DragRemoveObject;
		static const UILowerString TextStyle;
		static const UILowerString HeadShot;
		static const UILowerString RStyleOverlay;
		static const UILowerString RStyleOverlayColor;
		static const UILowerString RStyleOverlayOpacity;
	};

	enum Flags
	{
		F_mouseDraggable    = 0x0001,
		F_autoComputeView   = 0x0002,
		F_dragRemoveObject  = 0x0004,
		F_dragging          = 0x0008,
		F_zoomInterpolating = 0x0010,
		F_drawExtent        = 0x0020,
		F_alterObject       = 0x0040,
		F_ownsObject        = 0x0080,
		F_wireFrame         = 0x0100,
		F_paused            = 0x0200,
		F_pitchEnabled      = 0x0400,
		F_drawName          = 0x0800,
		F_headShot          = 0x1000,
		F_headZoom          = 0x2000
	};

	                          CuiWidget3dObjectViewer (); 
	virtual                  ~CuiWidget3dObjectViewer ();
	virtual UIBaseObject *    Clone( void ) const { return new CuiWidget3dObjectViewer; }
	virtual	UIStyle *         GetStyle( void ) const;
	virtual void              Destroy( void );


	virtual void              Render( UICanvas & ) const;
	virtual bool			  OnMessage( UIWidget *context, const UIMessage & msg );
	virtual void              OnShow( UIWidget *Context );
	virtual void              OnHide( UIWidget *Context );
	virtual void              SetSize( const UISize & );
	
	virtual const char        *GetTypeName( void ) const { return TypeName; }

	ClientObject *            getObject ();
	const ClientObject *      getObject () const;
	ClientObject *            setObject (ClientObject * object, const Vector& cameraOffsetMin, const Vector& cameraOffsetMax);

	virtual void              Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );
	virtual bool              SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool              GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual void              GetLinkPropertyNames (UIPropertyNameVector & ) const;
	virtual void              GetPropertyNames     (UIPropertyNameVector &, bool forCopy ) const;
	virtual UIWidget *        GetCustomDragWidget  (const UIPoint & point, UIPoint & offset);

	const Light &             getLight ();
	const Light &             getLight2 ();

	real                      getZoom () const;
	void                      setZoom (real z);

	real                      getTargetZoom () const;
	void                      setTargetZoom (real z);

	void                      setZoomInterpolating (bool b);
	void                      setPitchEnabled (bool b);
	void                      setPaused (bool b);

	float                     getObjectPitch () const;
	float                     getObjectYaw () const;
	void                      setObjectYaw (float y);
	void                      setRotateSpeed (float y);
	float                     getRotateSpeed () const;

	const Vector              computeMinimumVectorFromExtent (const Extent & extent) const;

	void                      setAutoComputeMinimumVectorFromExtent (bool b);
	void                      setOwnsObject (bool b);
	void                      setAlterObject (bool b);
	void                      setDragRemoveObject (bool b);
	void                      setHeadShot (bool b);
	void                      setHeadZoom (bool b);

	void                      setMouseDraggable (bool b);
	void                      setDrawName (bool b);

	const NetworkId &         getObjectNetworkId () const;

	bool                      hasFlags (Flags f) const;

	void                      setTextStyle (UITextStyle * style);

	void                      setOverlay (UIRectangleStyle * rs);

private:
	                          CuiWidget3dObjectViewer (const CuiWidget3dObjectViewer & rhs);
	                          CuiWidget3dObjectViewer & operator= (const CuiWidget3dObjectViewer & rhs);

	void                      setFlags (Flags f);
	void                      unsetFlags (Flags f);
	void                      setUnsetFlags (Flags f, bool set);
	void                      recomputeLineWidth ();

protected:

	Watcher<ClientObject>     m_object;
	Watcher<ClientObject>     m_renderObject;

	ObjectListCamera *        m_camera;
	float                     m_objectOriginOffsetY;
	mutable Transform         m_objectTransform;
	Vector                    m_cameraTranslateMax;
	Vector                    m_cameraTranslateMin;
	mutable bool              m_allowMouseWheelHACK;

	void                      RenderStart   (UICanvas & canvas) const;
	void                      RenderStop    () const;

private:
	ObjectList *              m_objectList;
	Light *                   m_light;
	Light *                   m_light_2;
	Vector                    m_cameraTranslate;
	float                     m_objectYaw;
	float                     m_objectPitch;

	float                     m_lightPitch;
	float                     m_lightYaw;
	float                     m_objectPitchDefault;
	float                     m_objectYawDefault;

	UIColor                   m_lightAmbientColor;

	float                     m_rotateSpeed;
	float                     m_defaultRotateSpeed;

	float                     m_zoom;
	float                     m_targetZoom;

	UIPoint                   m_lastDragPoint;

	unsigned long             m_lastDragTime;

	mutable bool              m_autoComputeMinimumVectorFromExtentDirty;

	Extent                    m_objectSphereExtent;

	NetworkId                 m_objectNetworkId;

	uint32                    m_flags;

	UITextStyle *             m_textStyle;

	Unicode::String           m_text;

	long                      m_lineWidth;

	UIRectangleStyle *        m_overlay;
	UIColor                   m_overlayColor;
	float                     m_overlayOpacity;

	UIRect                    m_margin;
};

// ======================================================================

inline UIStyle * CuiWidget3dObjectViewer::GetStyle( void ) const
{
	return 0;
}
//-----------------------------------------------------------------

inline const Light & CuiWidget3dObjectViewer::getLight ()
{
	return *m_light;
}

//-----------------------------------------------------------------

inline const Light & CuiWidget3dObjectViewer::getLight2 ()
{
	return *m_light_2;
}

//----------------------------------------------------------------------
inline real CuiWidget3dObjectViewer::getZoom () const
{
	return m_zoom;
}
//----------------------------------------------------------------------
inline real CuiWidget3dObjectViewer::getTargetZoom () const
{
	return m_targetZoom;
}

//-----------------------------------------------------------------

inline float CuiWidget3dObjectViewer::getObjectPitch () const
{
	return m_objectPitch;
}

//-----------------------------------------------------------------

inline float CuiWidget3dObjectViewer::getObjectYaw () const
{
	return m_objectYaw;
}

//----------------------------------------------------------------------

inline ClientObject * CuiWidget3dObjectViewer::getObject ()
{
	return m_object;
}

//----------------------------------------------------------------------

inline const ClientObject * CuiWidget3dObjectViewer::getObject () const
{
	return m_object;
}

//----------------------------------------------------------------------

inline float CuiWidget3dObjectViewer::getRotateSpeed () const
{
	return m_rotateSpeed;
}

//----------------------------------------------------------------------

/**
* Return the NetworkId of the object.  This should still be valid even if
* the object pointer is dangling.  This should be used to cleanup the object
* pointer in the same frame it is deleted, before any alter or render is called.
*
*/

inline const NetworkId & CuiWidget3dObjectViewer::getObjectNetworkId () const
{
	return m_objectNetworkId;
}

//----------------------------------------------------------------------

inline bool CuiWidget3dObjectViewer::hasFlags (Flags f) const
{
	return (m_flags & f) != 0;
}

//-----------------------------------------------------------------

#endif

