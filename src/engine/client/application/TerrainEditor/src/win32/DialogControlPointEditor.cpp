//===================================================================
//
// DialogControlPointEditor.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstTerrainEditor.h"
#include "DialogControlPointEditor.h"

#include "DialogControlPointEdit.h"

//===================================================================

namespace
{
	const char* const numericFormatString = "%1.2f";
}

//===================================================================

DialogControlPointEditor::DialogControlPointEditor (const ArrayList<Vector2d>& pointList) : 
	CDialog(DialogControlPointEditor::IDD, 0),
	m_pointList (pointList.size ()),
	m_heightList (0),
	m_hasHeights(false)
{
	//{{AFX_DATA_INIT(DialogControlPointEditor)
	//}}AFX_DATA_INIT

	m_pointList = pointList;
}

//-------------------------------------------------------------------

DialogControlPointEditor::DialogControlPointEditor (const ArrayList<Vector2d>& pointList, const ArrayList<float>& heightList) : 
	CDialog(DialogControlPointEditor::IDD, 0),
	m_pointList (pointList.size ()),
	m_heightList (heightList.size()),
	m_hasHeights(true)
{
	//{{AFX_DATA_INIT(DialogControlPointEditor)
	//}}AFX_DATA_INIT

	m_pointList = pointList;
	m_heightList = heightList;
}

//-------------------------------------------------------------------

void DialogControlPointEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogControlPointEditor)
	DDX_Control(pDX, IDC_LIST_CONTROL_POINTS, m_listCtrl);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogControlPointEditor, CDialog)
	//{{AFX_MSG_MAP(DialogControlPointEditor)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, OnButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_UP, OnButtonUp)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CONTROL_POINTS, OnDblclkListControlPoints)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT_APPEND, OnButtonImportAppend)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT_CLEAR, OnButtonImportClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

BOOL DialogControlPointEditor::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	IGNORE_RETURN (m_listCtrl.DeleteAllItems ());
	IGNORE_RETURN (m_listCtrl.InsertColumn (0, "x", LVCFMT_LEFT, 80, 0));
	IGNORE_RETURN (m_listCtrl.InsertColumn (1, "z", LVCFMT_LEFT, 80, 1));
	if(m_hasHeights)
	{
		IGNORE_RETURN (m_listCtrl.InsertColumn (2,"height", LVCFMT_LEFT,80,2));
	}

	CString buffer;

	DEBUG_FATAL(m_hasHeights && (m_pointList.size() != m_heightList.size()),("DialogControlPointEditor::OnInitDialog() - height and pointlist not the same size"));
	int i;
	for (i = 0; i < m_pointList.size (); ++i)
	{
		buffer.Format (numericFormatString, m_pointList [i].x);
		IGNORE_RETURN (m_listCtrl.InsertItem (i, buffer));

		buffer.Format (numericFormatString, m_pointList [i].y);
		IGNORE_RETURN (m_listCtrl.SetItemText (i, 1, buffer));

		if(m_hasHeights)
		{
			buffer.Format (numericFormatString, m_heightList[i]);
			IGNORE_RETURN (m_listCtrl.SetItemText (i,2,buffer));
		}
	}

	

	m_listCtrl.SetFocus ();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-------------------------------------------------------------------

void DialogControlPointEditor::OnButtonAdd() 
{
	Vector2d pointSum(0.0f,0.0f);
	float heightSum = 0.0f;

	if (m_pointList.size ())
	{
		int i;
		for (i = 0; i < m_pointList.size (); ++i)
		{
			pointSum += m_pointList [i];
			if(m_hasHeights)
			{
				heightSum += m_heightList[i];
			}
		}

		pointSum /= static_cast<float> (m_pointList.size ());
		if(m_hasHeights)
		{
			heightSum /= static_cast<float> (m_heightList.size ());
		}
	}

	const int i = m_listCtrl.GetItemCount ();

	CString buffer;
	buffer.Format (numericFormatString, pointSum.x);
	IGNORE_RETURN (m_listCtrl.InsertItem (i, buffer));

	buffer.Format (numericFormatString, pointSum.y);
	IGNORE_RETURN (m_listCtrl.SetItemText (i, 1, buffer));

	if(m_hasHeights)
	{
		buffer.Format (numericFormatString, heightSum);
		IGNORE_RETURN (m_listCtrl.SetItemText (i, 2, buffer));
	}

	m_listCtrl.SetItemState (i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_listCtrl.EnsureVisible (i, false);
}

//-------------------------------------------------------------------

void DialogControlPointEditor::OnButtonDelete() 
{
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition ();

	if (pos != NULL)
	{
		int item = m_listCtrl.GetNextSelectedItem (pos);
		m_listCtrl.DeleteItem (item);

		item = max (0, item - 1);
		m_listCtrl.SetItemState (item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_listCtrl.EnsureVisible (item, false);
	}
}

//-------------------------------------------------------------------

void DialogControlPointEditor::OnButtonDown() 
{
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition ();

	if (pos != NULL)
	{
		int item = m_listCtrl.GetNextSelectedItem (pos);

		if (item < m_listCtrl.GetItemCount () - 1)
		{
			CString temp = m_listCtrl.GetItemText (item, 0);
			m_listCtrl.SetItemText (item, 0, m_listCtrl.GetItemText (item + 1, 0));
			m_listCtrl.SetItemText (item + 1, 0, temp);

			temp = m_listCtrl.GetItemText (item, 1);
			m_listCtrl.SetItemText (item, 1, m_listCtrl.GetItemText (item + 1, 1));
			m_listCtrl.SetItemText (item + 1, 1, temp);

			if(m_hasHeights)
			{
				temp = m_listCtrl.GetItemText (item, 2);
				m_listCtrl.SetItemText (item, 2, m_listCtrl.GetItemText (item + 1, 2));
				m_listCtrl.SetItemText (item + 1, 2, temp);
			}

			m_listCtrl.SetItemState (item + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_listCtrl.EnsureVisible (item + 1, false);
		}
	}
}

//-------------------------------------------------------------------

void DialogControlPointEditor::OnButtonUp() 
{
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition ();

	if (pos != NULL)
	{
		int item = m_listCtrl.GetNextSelectedItem (pos);

		if (item > 0)
		{
			CString temp = m_listCtrl.GetItemText (item, 0);
			m_listCtrl.SetItemText (item, 0, m_listCtrl.GetItemText (item - 1, 0));
			m_listCtrl.SetItemText (item - 1, 0, temp);

			temp = m_listCtrl.GetItemText (item, 1);
			m_listCtrl.SetItemText (item, 1, m_listCtrl.GetItemText (item - 1, 1));
			m_listCtrl.SetItemText (item - 1, 1, temp);

			if(m_hasHeights)
			{
				temp = m_listCtrl.GetItemText (item, 2);
				m_listCtrl.SetItemText (item, 2, m_listCtrl.GetItemText (item - 1, 2));
				m_listCtrl.SetItemText (item - 1, 2, temp);
			}

			m_listCtrl.SetItemState (item - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			m_listCtrl.EnsureVisible (item - 1, false);
		}
	}
}

//-------------------------------------------------------------------

void DialogControlPointEditor::OnButtonImportAppend() 
{
}

//-------------------------------------------------------------------

void DialogControlPointEditor::OnButtonImportClear() 
{
}

//-------------------------------------------------------------------

void DialogControlPointEditor::OnDblclkListControlPoints(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	POSITION pos = m_listCtrl.GetFirstSelectedItemPosition ();

	if (pos != NULL)
	{
		const int item = m_listCtrl.GetNextSelectedItem (pos);
		CString sx = m_listCtrl.GetItemText (item, 0);
		CString sz = m_listCtrl.GetItemText (item, 1);
		float x = static_cast<float> (atof (sx));
		float z = static_cast<float> (atof (sz));

		if(m_hasHeights)
		{
			CString sHeight = m_listCtrl.GetItemText (item, 2);
			float height = static_cast<float> (atof (sHeight));
			DialogControlPointEdit dlg (Vector2d (x,z),height);
			if (dlg.DoModal ())
			{
				Vector2d point = dlg.getPoint ();

				sx.Format (numericFormatString, point.x);
				sz.Format (numericFormatString, point.y);

				m_listCtrl.SetItemText (item, 0, sx);
				m_listCtrl.SetItemText (item, 1, sz);

				CString sHeight;
				float height = dlg.getHeight();
				sHeight.Format(numericFormatString, height);
				m_listCtrl.SetItemText (item, 2, sHeight);
				
			}
		}
		else
		{
			DialogControlPointEdit dlg (Vector2d (x, z));
			if (dlg.DoModal ())
			{
				Vector2d point = dlg.getPoint ();

				sx.Format (numericFormatString, point.x);
				sz.Format (numericFormatString, point.y);

				m_listCtrl.SetItemText (item, 0, sx);
				m_listCtrl.SetItemText (item, 1, sz);
			}
		}

		
	}

	*pResult = 0;
}

//-------------------------------------------------------------------

void DialogControlPointEditor::OnOK() 
{
	//-- copy list ctrl to pointlist
	m_pointList.clear ();


	int i;
	for (i = 0; i < m_listCtrl.GetItemCount (); ++i)
	{
		const CString sx = m_listCtrl.GetItemText (i, 0);
		const CString sz = m_listCtrl.GetItemText (i, 1);

		const float x = static_cast<float> (atof (sx));
		const float z = static_cast<float> (atof (sz));

		m_pointList.add (Vector2d (x, z));
	}

	if(m_hasHeights)
	{
		m_heightList.clear();
		for (i = 0; i < m_listCtrl.GetItemCount (); ++i)
		{
			const CString sHeight = m_listCtrl.GetItemText (i, 2);
			const float height = static_cast<float> (atof (sHeight));
			m_heightList.add (height);
		}

	}

	CDialog::OnOK();
}

//===================================================================

