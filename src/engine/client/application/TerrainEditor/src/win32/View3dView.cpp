//
// View3dView.cpp 
// aommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "View3dView.h"

#include "MapFrame.h"
#include "Resource.h"
#include "TerrainEditorDoc.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/GameCamera.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedTerrain/TerrainObject.h"

//-------------------------------------------------------------------

static inline bool keyDown (int key)
{
	return (GetKeyState (key) & 0x8000) != 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(View3dView, CView)

//-------------------------------------------------------------------

View3dView::View3dView() :
	CView (),
	camera (0),
	terrain (0),
	yaw (0),
	pitch (0),
	timer (0),
	milliseconds (50),
	elapsedTime (0.f),
	render (false)
{
}

//-------------------------------------------------------------------

View3dView::~View3dView()
{
	delete camera;
	camera = 0;

	NOT_NULL (terrain);
	terrain->removeFromWorld ();
	delete terrain;
	terrain = 0;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(View3dView, CView)
	//{{AFX_MSG_MAP(View3dView)
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_COMMAND(ID_REFRESH, OnRefresh)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void View3dView::OnDraw(CDC* pDC)
{
	UNREF (pDC);

	if (!render || !(terrain && terrain->getAppearance ()))
		return;

	NOT_NULL (camera);

	NOT_NULL (terrain);
	IGNORE_RETURN (terrain->alter (elapsedTime));

	elapsedTime = 0;

	if (keyDown (VK_UP))
		camera->move_o (Vector::unitZ);

	if (keyDown (VK_DOWN))
		camera->move_o (Vector::negativeUnitZ);

	if (keyDown (VK_LEFT))
		camera->move_o (Vector::negativeUnitX);

	if (keyDown (VK_RIGHT))
		camera->move_o (Vector::unitX);

	real height = 0.f;
	if (terrain->getHeight (camera->getPosition_w (), height))
	{
		const Vector oldPosition = camera->getPosition_w ();
		const Vector newPosition (oldPosition.x, height + 2.f, oldPosition.z);

		camera->setPosition_p (newPosition);
	}

	//-- point the Gl at this window
	Graphics::setViewport (0, 0, camera->getViewportWidth (), camera->getViewportHeight ());

	//-- render a frame
	Graphics::beginScene ();

		Graphics::clearViewport(true, 0xffa1a1a1, true, 1.0f, true, 0);

		//-- render the scene
		camera->renderScene ();

	//-- done rendering the frame
	Graphics::endScene ();
	Graphics::present (m_hWnd, camera->getViewportWidth (), camera->getViewportHeight ());
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void View3dView::AssertValid() const
{
	CView::AssertValid();
}

void View3dView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void View3dView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	timer = SetTimer (1, milliseconds, 0);

	//-- create camera
	camera = new GameCamera ();
	camera->setHorizontalFieldOfView (PI_OVER_3);
	camera->setNearPlane (0.1f);
	camera->setFarPlane (1000.f);
	camera->addToWorld ();

	terrain = new TerrainObject ();
	RenderWorld::addObjectNotifications (*terrain);
	terrain->addToWorld ();

	yaw   = 0;
	pitch = 0;
}  //lint !e429  //-- ambientLight/parallelLight has not been freed or returned

//-------------------------------------------------------------------

void View3dView::OnMouseMove(UINT nFlags, CPoint point) 
{
	NOT_NULL (terrain);

	if (terrain->getAppearance ())
	{
		static CPoint lastPoint = point;

		SetCapture ();

		if (nFlags & MK_LBUTTON)
		{
			CRect rect;
			GetClientRect (&rect);

			real yawMod = PI_TIMES_2 * static_cast<real> (point.x - lastPoint.x) / rect.Width ();
			yaw += yawMod;

			real pitchMod = PI_TIMES_2 * static_cast<real> (point.y - lastPoint.y) / rect.Height ();
			pitch += pitchMod;
			pitch = clamp (-PI_OVER_2, pitch, PI_OVER_2);

			NOT_NULL (camera);

			camera->resetRotate_o2p ();
			camera->yaw_o (yaw);
			camera->pitch_o (pitch);

			Invalidate ();
		}
		else
		{
			ReleaseCapture ();
		}
		
		lastPoint = point;
	}

	CView::OnMouseMove(nFlags, point);
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------

void View3dView::OnSize(UINT nType, int cx, int cy) 
{
	if (camera && cx && cy)
		camera->setViewport(0, 0, cx, cy);

	CView::OnSize(nType, cx, cy);

	Invalidate ();
}

//-------------------------------------------------------------------

BOOL View3dView::OnEraseBkgnd(CDC* pDC) 
{
	NOT_NULL (terrain);

	if (terrain->getAppearance ())
		return TRUE; 

	return CView::OnEraseBkgnd (pDC);	
}

//-------------------------------------------------------------------

void View3dView::OnDestroy() 
{
	CView::OnDestroy();

	IGNORE_RETURN (KillTimer (static_cast<int> (timer)));

	// TODO: Add your message handler code here
	delete camera;
	camera = 0;
}

//-------------------------------------------------------------------

void View3dView::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == timer)
		elapsedTime += RECIP (static_cast<float> (milliseconds));

	CView::OnTimer(nIDEvent);

	Invalidate ();
}

//-------------------------------------------------------------------

void View3dView::OnRefresh() 
{
	const TerrainEditorDoc* doc = dynamic_cast<const TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);

	Iff iff (10000);
	doc->save (iff, 0);
	iff.allowNonlinearFunctions ();
	iff.goToTopOfForm ();

	NOT_NULL (terrain);
	terrain->setAppearance (0);

	const AppearanceTemplate* at = AppearanceTemplateList::fetch (&iff);
	terrain->setAppearance (at->createAppearance ());
	AppearanceTemplateList::release (at);

	Vector2d center;
	center.makeZero ();
	if (doc->getMapFrame ())
	{
		center = doc->getMapFrame ()->getCenter ();
		NON_NULL (camera)->setPosition_p (Vector (center.x, 0.f, center.y));
	}

	Invalidate ();
}

//-------------------------------------------------------------------

void View3dView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	render = true;	
}

//-------------------------------------------------------------------

void View3dView::OnKillFocus(CWnd* pNewWnd) 
{
	CView::OnKillFocus(pNewWnd);
	
	render = false;	
}

//-------------------------------------------------------------------

