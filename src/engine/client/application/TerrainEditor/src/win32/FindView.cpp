//
// FindView.cpp
// asommers 10-9-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FindView.h"

#include "DialogFind.h"
#include "FindFrame.h"
#include "LayerFrame.h"
#include "LayerView.h"
#include "Resource.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FindView, CListView)

//-------------------------------------------------------------------

FindView::FindView() :
	CListView (),
	imageListSet (false),
	imageList ()
{
}

//-------------------------------------------------------------------

FindView::~FindView()
{
}

//-------------------------------------------------------------------

//lint -save -e1924 -e648

BEGIN_MESSAGE_MAP(FindView, CListView)
	//{{AFX_MSG_MAP(FindView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_FINDVIEW_CLEAR, OnFindviewClear)
	ON_COMMAND(ID_FINDVIEW_FINDBYNAME, OnFindviewFindbyname)
	ON_COMMAND(ID_FINDVIEW_FINDBYTYPE, OnFindviewFindbytype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

void FindView::OnDraw(CDC* pDC)
{
	UNREF(pDC);
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void FindView::AssertValid() const
{
	CListView::AssertValid();
}

void FindView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FindView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	if (!imageListSet)
	{
		IGNORE_RETURN (imageList.Create (IDB_BITMAP_LAYER, 16, 1, RGB (255,255,255)));
		IGNORE_RETURN (GetListCtrl ().SetImageList (&imageList, LVSIL_SMALL));

		imageListSet = true;
	}

	IGNORE_RETURN (GetListCtrl ().InsertColumn (0, "Description", LVCFMT_LEFT, 600, 0));
}

//-------------------------------------------------------------------

void FindView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	UNREF (pHint);
	UNREF(lHint);
	UNREF(pSender);
}

//-------------------------------------------------------------------

BOOL FindView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style = cs.style | LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void FindView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNREF (pNMHDR);

	POSITION pos = GetListCtrl ().GetFirstSelectedItemPosition ();

	if (pos)
	{
		const int item = GetListCtrl ().GetNextSelectedItem (pos);

		const TerrainGenerator::LayerItem* layerItem = reinterpret_cast<const TerrainGenerator::LayerItem*> (GetListCtrl ().GetItemData (item));

		TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);

		GetApp ()->showLayers ();

		if (doc->getLayerFrame ())
			doc->getLayerFrame ()->selectLayerItem (layerItem);
	}
	
	*pResult = 0;
}

//-------------------------------------------------------------------

void FindView::update (const TerrainGeneratorHelper::OutputData& outputData) const
{
	const int index = GetListCtrl ().GetItemCount ();

	IGNORE_RETURN (GetListCtrl ().InsertItem (index, *outputData.message, LayerView::getIcon (outputData.layerItem)));
	IGNORE_RETURN (GetListCtrl ().SetItemData (index, reinterpret_cast<DWORD> (outputData.layerItem)));
}

//-------------------------------------------------------------------

void FindView::clear (void) const
{
	IGNORE_RETURN (GetListCtrl ().DeleteAllItems ());
}

//-------------------------------------------------------------------

void FindView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CListView::OnRButtonDown(nFlags, point);

	CPoint pt = point;
	ClientToScreen (&pt);

	CMenu menu;
	IGNORE_RETURN (menu.LoadMenu (IDR_FINDVIEW_MENU));

	CMenu* subMenu = menu.GetSubMenu (0);
	IGNORE_RETURN (subMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this));
}  //lint !e1746  //-- point could be made a const reference

//-------------------------------------------------------------------

void FindView::OnFindviewClear() 
{
	clear ();
}

//-------------------------------------------------------------------

void FindView::OnFindviewFindbyname() 
{
	ArrayList<TerrainGeneratorHelper::LayerItemQueryType> query;

	DialogFind dlg (query);
	if (dlg.DoModal () == IDOK)
	{
		GetApp ()->showFind (dlg.getClear ());

		const TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);

		ArrayList<TerrainGeneratorHelper::OutputData> output;

		TerrainGeneratorHelper::query (doc->getTerrainGenerator (), dlg.getName (), output);

		//--
		int i;
		for (i = 0; i < output.getNumberOfElements (); i++)
		{
			switch (output [i].mode)
			{
			case TerrainGeneratorHelper::OutputData::M_find:
				{
					update (output [i]);
				}
				break;

			case TerrainGeneratorHelper::OutputData::M_console:
				{
					CONSOLE_PRINT (*output [i].message);
					CONSOLE_PRINT ("\r\n");
				}
				break;

			case TerrainGeneratorHelper::OutputData::M_warning:
			default:
				break;
			}

			delete output [i].message;
		}
	}
}

//-------------------------------------------------------------------

void FindView::OnFindviewFindbytype() 
{
	ArrayList<TerrainGeneratorHelper::LayerItemQueryType> query;

	DialogFind dlg (query);
	if (dlg.DoModal () == IDOK)
	{
		GetApp ()->showFind (dlg.getClear ());

		const TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);

		ArrayList<TerrainGeneratorHelper::OutputData> output;

		TerrainGeneratorHelper::query (doc->getTerrainGenerator (), query, output);

		//--
		int i;
		for (i = 0; i < output.getNumberOfElements (); i++)
		{
			switch (output [i].mode)
			{
			case TerrainGeneratorHelper::OutputData::M_find:
				{
					update (output [i]);
				}
				break;

			case TerrainGeneratorHelper::OutputData::M_console:
				{
					CONSOLE_PRINT (*output [i].message);
					CONSOLE_PRINT ("\r\n");
				}
				break;

			case TerrainGeneratorHelper::OutputData::M_warning:
			default:
				break;
			}

			delete output [i].message;
		}
	}
}

//===================================================================

