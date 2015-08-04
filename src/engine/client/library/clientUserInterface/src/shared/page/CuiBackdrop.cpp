// ======================================================================
//
// CuiBackdrop.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiBackdrop.h"

#include "UIClock.h"
#include "UIData.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UnicodeUtils.h"
#include "clientGraphics/Camera.h"
#include "clientGraphics/Graphics.h"
#include "clientUserInterface/CuiManager.h"
#include "sharedFoundation/Clock.h"
#include "sharedObject/ObjectList.h"

// ======================================================================

namespace CuiBackdropNamespace
{
	CuiBackdrop * s_backdrop = 0;
}

using namespace CuiBackdropNamespace;

//----------------------------------------------------------------------

CuiBackdrop::CuiBackdrop (UIPage & thePage) :
CuiMediator     ("CuiBackdrop", thePage),
m_sceneRenderer (0),
m_numBackdrops  (0),
m_backdrops     (0)
{

	m_sceneRenderer = new SceneRenderer ();
	IGNORE_RETURN (getPage ().AddChild (m_sceneRenderer));

	m_sceneRenderer->SetSize (getPage ().GetSize ());
	m_sceneRenderer->SetLocation (0,0);
	m_sceneRenderer->SetPropertyNarrow (UIWidget::PropertyName::PackSize, "1,1");

	s_backdrop = this;

	const UIData * const codeData = getCodeData ();

	if (codeData)
	{
		const static UILowerString prop_backdropCount ("BackdropCount");
		if (codeData->GetPropertyInteger (prop_backdropCount, m_numBackdrops))
		{
			m_backdrops = new UIWidget * [m_numBackdrops];

			char buf [64];
			const size_t buf_size = sizeof (buf);

			for (int i = 0; i < m_numBackdrops; ++i)
			{
				m_backdrops [i] = 0;
				snprintf (buf, buf_size, "Backdrop%02d", i);
				getCodeDataObject (TUIWidget, m_backdrops [i], buf);
			}
		}
	}

	// the scene renderer will always clear the viewport for us
	getPage ().SetBackgroundColor (UIColor::black);

}

//-----------------------------------------------------------------

CuiBackdrop::~CuiBackdrop ()
{
	// PS UI cleans up scene renderer
	m_sceneRenderer = 0; //lint !e423 //memleak
	
	if (s_backdrop == this)
		s_backdrop = 0;

	delete [] m_backdrops;
	m_backdrops = 0;
}

//-----------------------------------------------------------------

void CuiBackdrop::performActivate ()
{
	m_sceneRenderer->SetEnabled (true);
	m_sceneRenderer->SetVisible (true);
}

//-----------------------------------------------------------------

void CuiBackdrop::performDeactivate ()
{
	m_sceneRenderer->SetEnabled (false);
	m_sceneRenderer->SetVisible (false);
}

//-----------------------------------------------------------------

void  CuiBackdrop::setRenderData (Camera * camera, ObjectList * objectList)
{
	if (m_sceneRenderer)
	{
		m_sceneRenderer->setRenderData (camera, objectList);
	}
}

// ======================================================================

CuiBackdrop::SceneRenderer::SceneRenderer () :
UIWidget (),
UINotification (),
m_camera (0),
m_objectList (0)
{
	UIClock::gUIClock().Listen( this );

}

//-----------------------------------------------------------------

CuiBackdrop::SceneRenderer::~SceneRenderer ()
{
	UIClock::gUIClock().StopListening( this );

	setRenderData (0,0);
	m_camera     = 0;
	m_objectList = 0;
}

//-----------------------------------------------------------------

void CuiBackdrop::SceneRenderer::Render( UICanvas & ) const
{
	Graphics::clearViewport (true, 0, true, 1.0f, true, 0);

	if (m_camera)
		m_camera->renderScene ();
}

//-----------------------------------------------------------------

void CuiBackdrop::SceneRenderer::setRenderData (Camera * camera, ObjectList * objectList)
{

	if (m_camera)
	{
		delete m_camera;
		m_camera = 0;
	}

	if (m_objectList)
	{
		m_objectList->removeAll (true);
		delete m_objectList;
		m_objectList = 0;
	}

	m_camera = camera;
	MEM_OWN(m_camera);
	m_objectList = objectList;
	MEM_OWN(m_objectList);
}

//-----------------------------------------------------------------

void CuiBackdrop::SceneRenderer::Notify( UINotificationServer *notifyingObject, UIBaseObject *contextObject, Code notificationCode )
{
	UNREF (notifyingObject);
	UNREF (contextObject);
	UNREF (notificationCode);
	
	if (IsEnabled () && IsVisible () && m_objectList)
	{ 
		static int last_frame_number = 0;	
		const int cur_frame_number = Graphics::getFrameNumber ();
		
		if (last_frame_number == cur_frame_number)
			return;

		m_objectList->prepareToAlter ();
		m_objectList->alter (Clock::frameTime ());
		m_objectList->conclude ();

		last_frame_number = cur_frame_number;
	}
}

//----------------------------------------------------------------------

void CuiBackdrop::staticSetBackdrop (int backdropIndex)
{
	if (s_backdrop)
		s_backdrop->setBackdrop (backdropIndex);
}

//----------------------------------------------------------------------

void CuiBackdrop::setBackdrop      (int backdropIndex)
{
	if (backdropIndex >= 0 || backdropIndex < m_numBackdrops)
	{
		for (int i = 0; i < m_numBackdrops; ++i)
		{
			UIWidget * const bd = m_backdrops [i];
			if (bd)
				bd->SetVisible (i == backdropIndex);
		}
	}
}

//-----------------------------------------------------------------
