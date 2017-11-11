//
// FloraMeshView.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FloraMeshView.h"

#include "sharedObject/AppearanceTemplateList.h"
#include "sharedUtility/FileName.h"
#include "clientObject/ObjectListCamera.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Light.h"
#include "sharedDebug/Profiler.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectList.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/VectorArgb.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FloraMeshView, CView)

//-------------------------------------------------------------------

FloraMeshView::FloraMeshView (void) :
	CView (),
	camera (0),
	objectList (0),
	object (0),
	yaw (0),
	pitch (0),
	zoom (0),
	hasName (false)
{
}  

//-------------------------------------------------------------------
	
FloraMeshView::~FloraMeshView()
{
	camera     = 0;
	objectList = 0;
	object     = 0;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FloraMeshView, CView)
	//{{AFX_MSG_MAP(FloraMeshView)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void FloraMeshView::loadObject (const char* newName)
{
	if (!object)
		return;

	NOT_NULL (object);
	NOT_NULL (camera);

	hasName = newName != 0;

	if (newName)
	{
		FileName fileName (FileName::P_appearance, newName);

		if (TreeFile::exists (fileName))
		{
			yaw   = PI_OVER_4;
			pitch = PI_OVER_4 * CONST_REAL (0.5);
			zoom  = CONST_REAL (2);

			object->setAppearance (AppearanceTemplateList::createAppearance (fileName));

			camera->resetRotateTranslate_o2p ();
			camera->setPosition_p (object->getAppearanceSphereCenter_w ());
			camera->yaw_o (yaw);
			camera->pitch_o (pitch);
			camera->move_o (Vector::negativeUnitZ * zoom * object->getAppearanceSphereRadius ());
		}
		else
			object->setAppearance (0);
	}
	else
		object->setAppearance (0);

	Invalidate ();
}

//-------------------------------------------------------------------

void FloraMeshView::OnDraw(CDC* pDC)
{
	NOT_NULL (object);
	NOT_NULL (camera);

	if (object->getAppearance ())
	{
		objectList->prepareToAlter ();
		objectList->alter (0.f);
		objectList->conclude ();

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
	else
	{
		IGNORE_RETURN (pDC->TextOut (0, 0, hasName ? "Could not find appearance" : "No appearance selected"));
	}
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void FloraMeshView::AssertValid() const
{
	CView::AssertValid();
}

void FloraMeshView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FloraMeshView::OnSize(UINT nType, int cx, int cy) 
{
	// TODO: Add your message handler code here
	if (camera && cx && cy)
		camera->setViewport(0, 0, cx, cy);

	CView::OnSize(nType, cx, cy);

	Invalidate ();
}

//-------------------------------------------------------------------

BOOL FloraMeshView::OnEraseBkgnd(CDC* pDC) 
{
	NOT_NULL (object);

	if (object->getAppearance ())
		return TRUE; 

	return CView::OnEraseBkgnd (pDC);	
}

//-------------------------------------------------------------------

void FloraMeshView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	//-- create camera
	camera     = new ObjectListCamera (2);
	camera->setNearPlane (CONST_REAL (0.1));

	//-- create objectlist
	objectList = new ObjectList (3);
	camera->addObjectList (objectList);

	//-- create lights (full ambient)
	Light* const ambientLight = new Light (Light::T_ambient, VectorArgb::solidWhite);
	objectList->addObject (ambientLight);

	Light* const parallelLight = new Light (Light::T_parallel, VectorArgb (CONST_REAL (1.0), CONST_REAL (1), CONST_REAL (1), CONST_REAL (1)));
	parallelLight->yaw_o (PI_OVER_4);
	parallelLight->pitch_o (-PI_OVER_4);
	objectList->addObject (parallelLight);

	object = new Object ();
	objectList->addObject (object);

	yaw   = PI_OVER_4;
	pitch = PI_OVER_4 * CONST_REAL (0.5);
	zoom  = CONST_REAL (2);
}  //lint !e429  //-- ambientLight/parallelLight has not been freed or returned

//-------------------------------------------------------------------

void FloraMeshView::OnDestroy() 
{
	CView::OnDestroy();
	
	// TODO: Add your message handler code here
	delete camera;
	camera = 0;

	if (objectList)
	{
		objectList->removeAll (true);

		delete objectList;
		objectList = 0;
	}
}

//-------------------------------------------------------------------

inline bool keyDown (int key)
{
	return (GetKeyState (key) & 0x8000) != 0;
}

//-------------------------------------------------------------------

void FloraMeshView::OnMouseMove(UINT nFlags, CPoint point) 
{
	NOT_NULL (object);

	if (object->getAppearance ())
	{
		static bool   first     = true;
		static CPoint lastPoint = point;

		if (keyDown (VK_MENU))
		{
			bool redraw = false;

			SetCapture ();

			if (nFlags & MK_LBUTTON && (nFlags & MK_RBUTTON || nFlags & MK_MBUTTON))
			{
				first = true;

				CRect rect;
				GetClientRect (&rect);

				//-- calculate how far to move
				zoom -= object->getAppearanceSphereRadius () * static_cast<real> (point.x - lastPoint.x) / rect.Width ();
				zoom = clamp (CONST_REAL (1), zoom, CONST_REAL (3));

				redraw = true;
			}
			else
				if (nFlags & MK_LBUTTON)
				{
					//-- panning
					if (first)
					{
						first     = false;
						lastPoint = point;
					}
					else
					{
						CRect rect;
						GetClientRect (&rect);

						real yawMod = PI_TIMES_2 * static_cast<real> (point.x - lastPoint.x) / rect.Width ();
						yaw += yawMod;

						real pitchMod = PI_TIMES_2 * static_cast<real> (point.y - lastPoint.y) / rect.Height ();
						pitch += pitchMod;
						pitch = clamp (-PI_OVER_2, pitch, PI_OVER_2);

						redraw = true;
					}
				}
				else
				{
					ReleaseCapture ();

					first = true;
				}

			if (redraw)
			{
				NOT_NULL (camera);

				camera->resetRotateTranslate_o2p ();
				camera->setPosition_p (object->getAppearanceSphereCenter_w ());
				camera->yaw_o (yaw);
				camera->pitch_o (pitch);
				camera->move_o (Vector::negativeUnitZ * zoom * object->getAppearanceSphereRadius ());

				Invalidate (false);
			}
		}

		lastPoint = point;
	}

	CView::OnMouseMove(nFlags, point);
}  //lint !e1746  //-- point could have been made a const reference

//-------------------------------------------------------------------
