// ======================================================================
//
// HardpointDialog.cpp
//
// Copyright 2005, Sony Online Entertainment
//
// ======================================================================

#include "FirstViewer.h"
#include "HardpointDialog.h"

#include "ChildFrm.h"
#include "Viewer.h"
#include "ViewerDoc.h"

#include "clientObject/HardpointObject.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/Hardpoint.h"
#include "sharedObject/Object.h"

#include <map>
#include <string>

// ======================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

BEGIN_MESSAGE_MAP(HardpointDialog, CDialog)
	//{{AFX_MSG_MAP(HardpointDialog)
	ON_NOTIFY(NM_RCLICK, IDC_HARDPOINT_TREE, OnRclickHardpointTree)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

namespace HardpointDialogNamespace
{
	enum HardpointIconList
	{
		HIL_object,
		HIL_hardpoint
	};

	typedef std::map<std::string, int> HardpointNameIndexMap;
};

using namespace HardpointDialogNamespace;

// ======================================================================

HardpointDialog::HardpointDialog(CWnd* pParent /*=NULL*/)
	: CDialog(HardpointDialog::IDD, pParent)
	, m_viewerDoc(0)
{
	DEBUG_REPORT_LOG(true, ("IDD = %d\n", IDD));

	//{{AFX_DATA_INIT(HardpointDialog)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void HardpointDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DEBUG_REPORT_LOG(true, ("IDC_HARDPOINT_TREE = %d\n", IDC_HARDPOINT_TREE));

	//{{AFX_DATA_MAP(HardpointDialog)
	DDX_Control(pDX, IDC_HARDPOINT_TREE, m_hardpointTree);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

void HardpointDialog::updateHardpointTree(Object const * const object)
{
	m_hardpointTree.DeleteAllItems();

	if (!object)
		return;

	addObjectToTree(object, TVI_ROOT);
}

// ----------------------------------------------------------------------

void HardpointDialog::addObjectToTree(Object const * const object, HTREEITEM parent)
{
	if (object)
	{
		AppearanceTemplate const * const appearanceTemplate = object ? (object->getAppearance () ? object->getAppearance ()->getAppearanceTemplate () : 0) : 0;

		if (appearanceTemplate)
		{
			HardpointNameIndexMap hardpointNameIndexMap;

			HTREEITEM item = m_hardpointTree.InsertItem(appearanceTemplate->getName(), HIL_object, HIL_object, parent);
			m_hardpointTree.SetItemState(item, TVIS_EXPANDED, TVIS_EXPANDED);
			m_hardpointTree.SetItemData(item, reinterpret_cast<unsigned long>(object));

			int const numberOfHardpoints = appearanceTemplate->getHardpointCount();

			for (int i = 0; i < numberOfHardpoints; ++i)
			{
				const Hardpoint& hardpoint = appearanceTemplate->getHardpoint(i);
				hardpointNameIndexMap[hardpoint.getName().getString()] = i;
			}

			HardpointNameIndexMap::iterator hardpointNameIndex = hardpointNameIndexMap.begin();

			for (; hardpointNameIndex != hardpointNameIndexMap.end(); ++hardpointNameIndex)
			{
				HTREEITEM hardpointItem = m_hardpointTree.InsertItem(hardpointNameIndex->first.c_str(), HIL_hardpoint, HIL_hardpoint, item);
				m_hardpointTree.SetItemState(hardpointItem, TVIS_EXPANDED, TVIS_EXPANDED);
				m_hardpointTree.SetItemData(hardpointItem, reinterpret_cast<unsigned long>(object));

				int numberOfChildObjects = object->getNumberOfChildObjects();
				Hardpoint const & hardpoint = appearanceTemplate->getHardpoint(hardpointNameIndex->second);

				for (int j=0; j<numberOfChildObjects; ++j)
				{
					Object const * const child = object->getChildObject(j);
					HardpointObject const * const hardpointObject = dynamic_cast<HardpointObject const *>(child);

					if (hardpointObject && hardpointObject->getHardpointName() == hardpoint.getName())
					{
						addObjectToTree(hardpointObject, hardpointItem);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void HardpointDialog::setViewerDoc(CViewerDoc * viewerDoc)
{
	ASSERT_VALID(viewerDoc);
	m_viewerDoc = viewerDoc;
}

// ----------------------------------------------------------------------

BOOL HardpointDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_imageList.Create(IDB_BITMAP_HARDPOINT, 20, 1, RGB(255,255,255));
	m_hardpointTree.SetImageList(&m_imageList, TVSIL_NORMAL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void HardpointDialog::OnRclickHardpointTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	UNREF(pNMHDR);
	UNREF(pResult);
	
	*pResult = 0;
}

// ----------------------------------------------------------------------

void HardpointDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	if (IsWindow(m_hardpointTree.GetSafeHwnd()))
	{
		CRect sz;
		GetClientRect(sz);
		sz.DeflateRect(5,5);

		m_hardpointTree.MoveWindow(sz);
	}
}

//-------------------------------------------------------------------

CString HardpointDialog::getSelectedHardpoint() const
{
	HTREEITEM selection = m_hardpointTree.GetSelectedItem();

	if (selection == NULL)
		return "";

	return m_hardpointTree.GetItemText(selection);
}

//-------------------------------------------------------------------

Object * HardpointDialog::getParentObject() const
{
	HTREEITEM selection = m_hardpointTree.GetSelectedItem();

	if (selection == NULL)
		return 0;

	return reinterpret_cast<Object *>(m_hardpointTree.GetItemData(selection));
}

// ======================================================================
