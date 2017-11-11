// ======================================================================
//
// SpaceZoneMapView.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "SpaceZoneMapView.h"

#include "ChildFrame.h"
#include "GdiHelper.h"
#include "MainFrame.h"
#include "Resource.h"
#include "sharedMath/Transform.h"
#include "SpaceZoneTreeView.h"
#include "SwgSpaceZoneEditorDoc.h"

// ======================================================================

namespace
{
	bool isCloseTo(CPoint const & center, int const radius, CPoint const & point)
	{
		CRect rect(center, CSize(radius * 2, radius * 2));
		rect.OffsetRect(-radius, -radius);

		return rect.PtInRect(point) != 0;
	}
}

// ======================================================================

SpaceZoneMapView::Grid::Grid() :
	m_mapView(0),
	m_gridDistance(0)
{
}

// ----------------------------------------------------------------------
	
SpaceZoneMapView::Grid::~Grid()
{
}

// ----------------------------------------------------------------------
	
void SpaceZoneMapView::Grid::setMapView(SpaceZoneMapView const * const mapView)
{
	m_mapView = mapView;
}

// ----------------------------------------------------------------------
	
void SpaceZoneMapView::Grid::setGridDistance(int const gridDistance)
{
	m_gridDistance = abs(gridDistance);
}

// ----------------------------------------------------------------------
	
void SpaceZoneMapView::Grid::draw(CDC * const pDC) const
{
	int const gridDistanceScreen = m_mapView->convertWorldToScreen(static_cast<float>(m_gridDistance));
	if (gridDistanceScreen > 0)
	{
		GdiHelper helper(pDC);
		helper.gray1();

		CRect rect;
		m_mapView->GetClientRect(&rect);

		//-- find where 0 is
		CPoint const centerScreen = m_mapView->convertWorldToScreen(Vector::zero);

		//-- draw vertical lines
		int x = rect.left / gridDistanceScreen;

		x += centerScreen.x % gridDistanceScreen;

		while (x < rect.right)
		{
			pDC->MoveTo(x, rect.top);
			pDC->LineTo(x, rect.bottom);

			x += gridDistanceScreen;
		}

		//-- draw horizontal lines
		int y = rect.top / gridDistanceScreen;

		y += centerScreen.y % gridDistanceScreen;

		while (y < rect.bottom)
		{
			pDC->MoveTo(rect.left, y);
			pDC->LineTo(rect.right, y);

			y += gridDistanceScreen;
		}
	}
}

// ======================================================================

IMPLEMENT_DYNCREATE(SpaceZoneMapView, CView)

BEGIN_MESSAGE_MAP(SpaceZoneMapView, CView)
	//{{AFX_MSG_MAP(SpaceZoneMapView)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// ======================================================================

SpaceZoneMapView::SpaceZoneMapView() :
	m_viewMode(VM_xz),
	m_dragging(false),
	m_draggingSelection(false),
	m_hideNavPoints(false),
	m_hideSpawners(false),
	m_hideMisc(false),
	m_hidePaths(false),
	m_hideGrid(false),
	m_zoomLevel(1.f),
	m_minimumZoomLevel(RECIP(64.f)),
	m_maximumZoomLevel(32.f),
	m_centerOffset(0, 0),
	m_keyStatePanMap(),
	m_grid(),
	m_upperLeft(),
	m_lowerRight(),
	m_selectionList()
{
	m_grid.setMapView(this);
	m_grid.setGridDistance(500);

	m_keyStatePanMap.bind(VK_MENU);
}

// ----------------------------------------------------------------------

SpaceZoneMapView::~SpaceZoneMapView()
{
}

// ----------------------------------------------------------------------

float SpaceZoneMapView::convertScreenToWorld(int const distance) const
{
	return static_cast<float>(distance) / m_zoomLevel;
}

//-------------------------------------------------------------------

Vector const SpaceZoneMapView::convertScreenToWorld(Vector const & position_w, CPoint const & point) const
{
	Vector result;

	switch(m_viewMode)
	{
	case VM_xz:
		{
			result.x = static_cast<float>(point.x - m_centerOffset.x) / m_zoomLevel;
			result.y = position_w.y;
			result.z = static_cast<float>(m_centerOffset.y - point.y) / m_zoomLevel;
		}
		break;

	case VM_xy:
		{
			result.x = static_cast<float>(point.x - m_centerOffset.x) / m_zoomLevel;
			result.y = static_cast<float>(m_centerOffset.y - point.y) / m_zoomLevel;
			result.z = position_w.z;
		}
		break;

	case VM_zy:
		{
			result.x = position_w.x;
			result.y = static_cast<float>(m_centerOffset.y - point.y) / m_zoomLevel;
			result.z = static_cast<float>(point.x - m_centerOffset.x) / m_zoomLevel;
		}
		break;
	}

	return result;
}

// ----------------------------------------------------------------------

int SpaceZoneMapView::convertWorldToScreen(float const distance) const
{
	return static_cast<int> (distance * m_zoomLevel);
}

//-------------------------------------------------------------------

CPoint const SpaceZoneMapView::convertWorldToScreen(Vector const & position_w) const
{
	CPoint result;

	switch(m_viewMode)
	{
	case VM_xz:
		{
			result.x = static_cast<int>(position_w.x * m_zoomLevel + static_cast<float>(m_centerOffset.x));
			result.y = static_cast<int>(static_cast<float>(m_centerOffset.y) - position_w.z * m_zoomLevel);
		}
		break;

	case VM_xy:
		{
			result.x = static_cast<int>(position_w.x * m_zoomLevel + static_cast<float>(m_centerOffset.x));
			result.y = static_cast<int>(static_cast<float>(m_centerOffset.y) - position_w.y * m_zoomLevel);
		}
		break;

	case VM_zy:
		{
			result.x = static_cast<int>(position_w.z * m_zoomLevel + static_cast<float>(m_centerOffset.x));
			result.y = static_cast<int>(static_cast<float>(m_centerOffset.y) - position_w.y * m_zoomLevel);
		}
		break;
	}

	return result;
}

// ----------------------------------------------------------------------

bool SpaceZoneMapView::filterObject(SwgSpaceZoneEditorDoc::Object const * const object) const
{
	if (!object)
		return true;

	if (m_hideNavPoints && object->getType() == SwgSpaceZoneEditorDoc::Object::T_navPoint)
		return true;

	if (m_hideSpawners && object->getType() == SwgSpaceZoneEditorDoc::Object::T_spawner)
		return true;

	if (m_hideSpawners && object->getType() == SwgSpaceZoneEditorDoc::Object::T_patrolPoint)
		return true;

	if (m_hideMisc && object->getType() == SwgSpaceZoneEditorDoc::Object::T_misc)
		return true;

	return false;
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::OnDraw(CDC * const pDC)
{
	SwgSpaceZoneEditorDoc const * const document = safe_cast<SwgSpaceZoneEditorDoc const *>(GetDocument());

	CRect rect;
	GetClientRect(&rect);

	GdiHelper helper(pDC);

	helper.black1();
	CBrush brush(RGB(0,0,0));
	pDC->FillRect(&rect, &brush);

	if (!m_hideGrid)
		m_grid.draw(pDC);

	//-- Zone boundary
	helper.white1();
	{
		Vector const minimum(Vector::xyz111 * (-8192.f + 512.f));
		Vector const maximum(Vector::xyz111 * (8192.f - 512.f));

		CPoint const upperLeft = convertWorldToScreen(minimum);
		CPoint const lowerRight = convertWorldToScreen(maximum);
		pDC->MoveTo(upperLeft.x, upperLeft.y);
		pDC->LineTo(lowerRight.x, upperLeft.y);
		pDC->LineTo(lowerRight.x, lowerRight.y);
		pDC->LineTo(upperLeft.x, lowerRight.y);
		pDC->LineTo(upperLeft.x, upperLeft.y);
	}

	//-- Center
	helper.gray1();
	{
		int const radius = 2;

		CPoint center = convertWorldToScreen(Vector::zero);
		pDC->Ellipse(center.x - radius, center.y - radius, center.x + radius, center.y + radius);
	}

	//-- Lines
	helper.gray1();
	{
		CString positiveX;
		CString positiveY;
		CString negativeX;
		CString negativeY;

		switch(m_viewMode)
		{
		case VM_xz:
			{
				positiveX = "+X";
				negativeX = "-X";
				positiveY = "+Z";
				negativeY = "-Z";
			}
			break;

		case VM_xy:
			{
				positiveX = "+X";
				negativeX = "-X";
				positiveY = "+Y";
				negativeY = "-Y";
			}
			break;

		case VM_zy:
			{
				positiveX = "+Z";
				negativeX = "-Z";
				positiveY = "+Y";
				negativeY = "-Y";
			}
			break;
		}

		pDC->TextOut(rect.Width() - 20, rect.CenterPoint().y - 8, positiveX);
		pDC->TextOut(rect.CenterPoint().x - 8, 1, positiveY);
		pDC->TextOut(1, rect.CenterPoint().y - 8, negativeX);
		pDC->TextOut(rect.CenterPoint().x - 8, rect.bottom - 16, negativeY);
	}

	//-- Selected object
	helper.red1();
	{
		if (m_dragging)
		{
			CPoint const upperLeft = convertWorldToScreen(m_upperLeft);
			CPoint const lowerRight = convertWorldToScreen(m_lowerRight);
			
			pDC->MoveTo(upperLeft.x, upperLeft.y);
			pDC->LineTo(lowerRight.x, upperLeft.y);
			pDC->LineTo(lowerRight.x, lowerRight.y);
			pDC->LineTo(upperLeft.x, lowerRight.y);
			pDC->LineTo(upperLeft.x, upperLeft.y);
		}

		//-- Render selection list
		helper.red3();
		for (size_t i = 0; i < m_selectionList.size(); ++i)
		{
			SwgSpaceZoneEditorDoc::Object const * const object = m_selectionList[i];
			object->draw(this, pDC);
		}
	}

	//-- Render selected spawner path(if applicable)
	helper.dottedWhite1();
	{
		if (!m_hideSpawners && !m_hidePaths)
		{
			SwgSpaceZoneEditorDoc::Object const * const object = document->getSelectedSpawner();
			if (object && object->isPatrolPointSpawner())
			{
				StringList patrolPointList;
				Configuration::unpackString(object->getPatrolPoints(), patrolPointList, ':');
				for (int i = 0; i < static_cast<int>(patrolPointList.size()) - 1; ++i)
				{
					SwgSpaceZoneEditorDoc::Object const * const patrolPoint1 = document->findPatrolPoint(patrolPointList[i]);
					SwgSpaceZoneEditorDoc::Object const * const patrolPoint2 = document->findPatrolPoint(patrolPointList[i + 1]);
					if (patrolPoint1 && patrolPoint2)
					{
						CPoint const point1 = convertWorldToScreen(patrolPoint1->getTransform_o2w().getPosition_p());
						pDC->MoveTo(point1);

						CPoint const point2 = convertWorldToScreen(patrolPoint2->getTransform_o2w().getPosition_p());
						pDC->LineTo(point2);
					}
				}
			}
		}
	}

	//-- Objects
	helper.white1();
	{
		int i;
		for (i = 0; i < document->getNumberOfObjects(); ++i)
		{
			SwgSpaceZoneEditorDoc::Object const * const object = document->getObject(i);
			if (filterObject(object))
				continue;

			Transform const & transform = object->getTransform_o2w();
			int const radius = 1;
			CPoint const point = convertWorldToScreen(transform.getPosition_p());

			if (object == document->getSelectedObject())
				helper.red3();
			else
			{
				switch(object->getType())
				{
				case SwgSpaceZoneEditorDoc::Object::T_navPoint:
					helper.blue2();
					break;

				case SwgSpaceZoneEditorDoc::Object::T_spawner:
					helper.yellow2();
					break;

				case SwgSpaceZoneEditorDoc::Object::T_patrolPoint:
					helper.green2();
					break;

				default:
					helper.white1();
					break;
				}
			}

			pDC->Ellipse(point.x - radius, point.y - radius, point.x + radius, point.y + radius);
		}
	}
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::OnInitialUpdate()
{
	GetWindowDC()->SetBkColor(RGB(0,0,0));
}

// ----------------------------------------------------------------------

BOOL SpaceZoneMapView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void SpaceZoneMapView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void SpaceZoneMapView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void SpaceZoneMapView::AssertValid() const
{
	CView::AssertValid();
}

void SpaceZoneMapView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

#endif //_DEBUG

// ======================================================================

void SpaceZoneMapView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_keyStatePanMap.isDown())
		return;

	m_dragging = false;
	m_draggingSelection = false;

	CRect rect;
	GetClientRect(&rect);

	//-- See if we've clicked on an object
	{
		if (!m_selectionList.empty())
		{
			for (size_t i = 0; i < m_selectionList.size(); ++i)
			{
				SwgSpaceZoneEditorDoc::Object const * object = m_selectionList[i];
				if (filterObject(object))
					continue;

				if (isCloseTo(convertWorldToScreen(object->getTransform_o2w().getPosition_p()), convertWorldToScreen(32.f), point))
				{
					m_draggingSelection = true;
					m_dragStart = convertScreenToWorld(Vector::zero, point);
				}
			}
		}

		if (!m_draggingSelection)
		{
			//-- see if we're within our rectangle
			m_upperLeft = convertScreenToWorld(Vector::zero, point);
			m_lowerRight = m_upperLeft;
			m_dragging = true;
		}

		Invalidate();
	}

	CView::OnLButtonDown(nFlags, point);
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::OnMouseMove(UINT nFlags, CPoint point) 
{
	//-- handle panning the map
	{
		if (m_keyStatePanMap.isDown())
		{
			SetCapture();

			if (nFlags & MK_LBUTTON)
			{
				//-- panning
				if (m_keyStatePanMap.first)
				{
					m_keyStatePanMap.first     = false;
					m_keyStatePanMap.lastPoint = point;
				}
				else
				{
					m_centerOffset += point - m_keyStatePanMap.lastPoint;
				}

				Invalidate(false);
			}
		}
		else
		{
			m_keyStatePanMap.first = true;

			if (m_draggingSelection)
			{
				SetCapture();

				Vector const dragEnd = convertScreenToWorld(Vector::zero, point);
				Vector offset = dragEnd - m_dragStart;
				m_dragStart = dragEnd;

				for (size_t i = 0; i < m_selectionList.size(); ++i)
				{
					SwgSpaceZoneEditorDoc::Object * const object = m_selectionList[i];

					Vector offset_p = offset;
					if (m_viewMode != VM_xz)
					{
						offset_p.x = 0;
						offset_p.z = 0;
					}

					Transform transform_o2w = object->getTransform_o2w();
					transform_o2w.move_p(offset_p);
					object->setTransform_o2w(transform_o2w);
				}

				Invalidate();
			}
			else
			{
				if (m_dragging)
				{
					SetCapture();

					CRect rect;
					GetClientRect(&rect);
					CPoint clampedPoint = point;
					clampedPoint.x = clamp(rect.left, clampedPoint.x, rect.right);
					clampedPoint.y = clamp(rect.top, clampedPoint.y, rect.bottom);
					
					m_lowerRight = convertScreenToWorld(Vector::zero, clampedPoint);

					Invalidate();
				}
				else
					ReleaseCapture();
			}
		}

		m_keyStatePanMap.lastPoint = point;
	}

	{
		Vector const position2d = convertScreenToWorld(Vector::zero, point);
		safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument())->setMapPosition(position2d);
		GetDocument()->UpdateAllViews(this, SwgSpaceZoneEditorDoc::H_updateRulers);
	}

	CView::OnMouseMove(nFlags, point);
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	SwgSpaceZoneEditorDoc * const document = safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument());

	//-- Find out which objects need to go into the selection list
	{
		if (m_dragging)
		{
			int objectsSelected = 0;
			SwgSpaceZoneEditorDoc::Object * selectedObject = 0;
			for (int i = 0; i < document->getNumberOfObjects(); ++i)
			{
				CPoint const upperLeft = convertWorldToScreen(m_upperLeft);
				CPoint const lowerRight = convertWorldToScreen(m_lowerRight);

				SwgSpaceZoneEditorDoc::Object * const object = document->getObject(i);
				if (filterObject(object))
					continue;

				CRect rect(std::min(upperLeft.x, lowerRight.x), std::min(upperLeft.y, lowerRight.y), std::max(upperLeft.x, lowerRight.x), std::max(upperLeft.y, lowerRight.y));
				if (rect.PtInRect(convertWorldToScreen(object->getTransform_o2w().getPosition_p())))
				{
					SelectionList::iterator iter = std::find(m_selectionList.begin(), m_selectionList.end(), object);
					if (iter == m_selectionList.end())
						m_selectionList.push_back(object);
					else
						m_selectionList.erase(iter);

					++objectsSelected;
					selectedObject = object;
				}
			}

			if (objectsSelected == 1 && selectedObject)
				document->selectObject(selectedObject);
		}
	}

	m_dragging = false;
	m_draggingSelection = false;

	Invalidate();

	CView::OnLButtonUp(nFlags, point);
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::setViewMode(ViewMode const viewMode)
{
	m_viewMode = viewMode;

	SwgSpaceZoneEditorDoc const * const document = safe_cast<SwgSpaceZoneEditorDoc const *>(GetDocument());
	SwgSpaceZoneEditorDoc::Object const * const object = document->getSelectedObject();
	if (object)
		recenter(object->getTransform_o2w().getPosition_p());
	else
		Invalidate();
}

// ----------------------------------------------------------------------

SpaceZoneMapView::ViewMode SpaceZoneMapView::getViewMode() const
{
	return m_viewMode;
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::setHideNavPoints(bool const hideNavPoints)
{
	m_hideNavPoints = hideNavPoints;
	Invalidate();
}

// ----------------------------------------------------------------------

bool SpaceZoneMapView::getHideNavPoints() const
{
	return m_hideNavPoints;
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::setHideSpawners(bool const hideSpawners)
{
	m_hideSpawners = hideSpawners;
	Invalidate();
}

// ----------------------------------------------------------------------

bool SpaceZoneMapView::getHideSpawners() const
{
	return m_hideSpawners;
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::setHideMisc(bool const hideMisc)
{
	m_hideMisc = hideMisc;
	Invalidate();
}

// ----------------------------------------------------------------------

bool SpaceZoneMapView::getHideMisc() const
{
	return m_hideMisc;
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::setHidePaths(bool const hidePaths)
{
	m_hidePaths = hidePaths;
	Invalidate();
}

// ----------------------------------------------------------------------

bool SpaceZoneMapView::getHidePaths() const
{
	return m_hidePaths;
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::setHideGrid(bool const hideGrid)
{
	m_hideGrid = hideGrid;
	Invalidate();
}

// ----------------------------------------------------------------------

bool SpaceZoneMapView::getHideGrid() const
{
	return m_hideGrid;
}

// ----------------------------------------------------------------------

BOOL SpaceZoneMapView::OnEraseBkgnd(CDC * /*pDC*/) 
{
	return TRUE;
}

// ----------------------------------------------------------------------

BOOL SpaceZoneMapView::OnMouseWheel(UINT const nFlags, short const zDelta, CPoint const pt) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetClientRect(&rect);

	CPoint point = pt;
	ScreenToClient(&point);

	Vector const center = convertScreenToWorld(Vector::zero, point);

	if (zDelta > 0)
		zoomIn(center);
	else
		zoomOut(center);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::zoomIn(Vector const & center)
{
	if (m_zoomLevel < m_maximumZoomLevel)
	{
		m_zoomLevel = clamp (m_minimumZoomLevel, m_zoomLevel * 2.f, m_maximumZoomLevel);

		recenter(center, true);
	}
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::zoomOut(Vector const & center)
{
	if (m_zoomLevel > m_minimumZoomLevel)
	{
		m_zoomLevel = clamp (m_minimumZoomLevel, m_zoomLevel * 0.5f, m_maximumZoomLevel);

		recenter (center, true);
	}
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::recenter(Vector const & center, bool const force)
{
	CRect rect;
	GetClientRect (&rect);

	CPoint centerScreen = convertWorldToScreen(center);
	if (!rect.PtInRect(centerScreen) || force)
	{
		m_centerOffset.x += rect.CenterPoint().x - centerScreen.x;
		m_centerOffset.y += rect.CenterPoint().y - centerScreen.y;

		GetDocument()->UpdateAllViews(this, SwgSpaceZoneEditorDoc::H_updateRulers);
	}

	Invalidate();
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::OnUpdate(CView * const pSender, LPARAM const lHint, CObject * const pHint) 
{
	if (pSender != this)
	{
		if (lHint == SwgSpaceZoneEditorDoc::H_selectObject)
		{
			SwgSpaceZoneEditorDoc::Object * const object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object *>(pHint);
			if (object)
			{
				recenter(object->getTransform_o2w().getPosition_p());
				m_selectionList.clear();
				m_selectionList.push_back(object);

				Invalidate();
			}
		}

		if (lHint == SwgSpaceZoneEditorDoc::H_deleteObject)
			Invalidate();

		if (lHint == SwgSpaceZoneEditorDoc::H_addPatrolPoint)
			Invalidate();

		if (lHint == SwgSpaceZoneEditorDoc::H_selectAllPatrolPoints)
		{
			SwgSpaceZoneEditorDoc * const document = safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument());
			SwgSpaceZoneEditorDoc::Object * object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object *>(pHint);
			if (object->isPatrolPoint())
				object = object->getParent();

			if (object->isPatrolPointSpawner())
			{
				m_selectionList.clear();

				StringList patrolPointList;
				Configuration::unpackString(object->getPatrolPoints(), patrolPointList, ':');
				for (size_t i = 0; i < patrolPointList.size(); ++i)
				{
					SwgSpaceZoneEditorDoc::Object * const patrolPoint = document->findPatrolPoint(patrolPointList[i]);
					m_selectionList.push_back(patrolPoint);
				}
				
				recenter(object->getTransform_o2w().getPosition_p());
			}

			Invalidate();
		}
	}
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_selectionList.clear();
	Invalidate();
	
	CView::OnRButtonDown(nFlags, point);
}

// ----------------------------------------------------------------------

void SpaceZoneMapView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CView::OnKeyDown(nChar, nRepCnt, nFlags);

	if (m_selectionList.empty())
		return;

	SwgSpaceZoneEditorDoc * const document = safe_cast<SwgSpaceZoneEditorDoc *>(GetDocument());

	switch (nChar)
	{
	case VK_DELETE:
		{
			if (m_selectionList.empty())
				MessageBox("No items selected");
			else
			{
				CString caption;
				caption.Format("Delete %d selected objects?", m_selectionList.size());

				CString message;
				{
					for (size_t i = 0; i < m_selectionList.size(); ++i)
						message += m_selectionList[i]->getName() + '\n';
				}

				if (MessageBox(message, caption, MB_YESNO) == IDYES)
				{
					//-- Delete all patrol points before spawners
					SelectionList patrolPointList;
					SelectionList otherList;
					{
						for (size_t i = 0; i < m_selectionList.size(); ++i)
						{
							SwgSpaceZoneEditorDoc::Object * const object = m_selectionList[i];
							if (object->isPatrolPoint())
								patrolPointList.push_back(object);
							else
								otherList.push_back(object);
						}
					}

					m_selectionList.clear();

					{
						for (size_t i = 0; i < patrolPointList.size(); ++i)
							document->deleteObject(patrolPointList[i]);
					}

					{
						for (size_t i = 0; i < otherList.size(); ++i)
							document->deleteObject(otherList[i]);
					}
				}

				Invalidate();
			}
		}
		break;

	case VK_INSERT:
		document->addPatrolPoint(document->getSelectedObject());
		break;

	default:
		break;
	}
}

// ======================================================================

