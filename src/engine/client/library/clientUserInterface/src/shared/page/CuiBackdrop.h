// ======================================================================
//
// CuiBackdrop.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiBackdrop_H
#define INCLUDED_CuiBackdrop_H

#include "UINotification.h"
#include "UIWidget.h"
#include "clientUserInterface/CuiMediator.h"

class Camera;
class ObjectList;
class UIPage;

// ======================================================================

/**
* CuiBackdrop just keeps a scene render around and also paints the background.
*/

class CuiBackdrop :
public CuiMediator
{
public:

	class SceneRenderer;

	explicit                 CuiBackdrop (UIPage & thePage);

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

	void                     setRenderData (Camera * camera, ObjectList * objectList);

	static void              staticSetBackdrop (int backdropIndex);
	void                     setBackdrop       (int backdropIndex);

private:
	                        ~CuiBackdrop ();
	                         CuiBackdrop ();
	                         CuiBackdrop (const CuiBackdrop & rhs);
	CuiBackdrop &            operator=   (const CuiBackdrop & rhs);

private:

	SceneRenderer *          m_sceneRenderer;

	int                      m_numBackdrops;
	UIWidget **              m_backdrops;
};

//-----------------------------------------------------------------

/**
* The SceneRenderer is simply a widget that displays an ObjectList through a Camera
* whenever the widget is visible.
*
* @todo: move to separate class
*/
class CuiBackdrop::SceneRenderer : public UIWidget, public UINotification

{
public:
	                          SceneRenderer ();
	                         ~SceneRenderer ();
	virtual UIBaseObject *    Clone( void ) const { return 0; }
	virtual	UIStyle *         GetStyle( void ) const { return 0; }
	
	virtual void              Render( UICanvas & ) const;

	void                      setRenderData (Camera * camera, ObjectList * objectList);
	virtual void              Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

private:

	SceneRenderer (const SceneRenderer &);
	SceneRenderer& operator= (const SceneRenderer &);

private:

	Camera *                  m_camera;
	ObjectList *              m_objectList;
};

// ======================================================================

#endif
