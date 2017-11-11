//
// DialogFind.cpp
// asommers 
//
// copyright 2001, verant interactive
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "terraineditor.h"
#include "DialogFind.h"

//-------------------------------------------------------------------

DialogFind::DialogFind(ArrayList<TerrainGeneratorHelper::LayerItemQueryType>& newQuery, CWnd* pParent /*=NULL*/) : 
	CDialog(DialogFind::IDD, pParent),
	query (newQuery),

	//-- widgets
	m_source (),
	m_destination (),
	m_clear (FALSE),
	m_name ()
{
	//{{AFX_DATA_INIT(DialogFind)
	m_clear = FALSE;
	m_name = _T("");
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

void DialogFind::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogFind)
	DDX_Control(pDX, IDC_LIST_SOURCE, m_source);
	DDX_Control(pDX, IDC_LIST_DESTINATION, m_destination);
	DDX_Check(pDX, IDC_CHECK_CLEAR, m_clear);
	DDX_Text(pDX, IDC_EDIT_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(DialogFind, CDialog)
	//{{AFX_MSG_MAP(DialogFind)
	ON_LBN_DBLCLK(IDC_LIST_DESTINATION, OnDblclkListDestination)
	ON_LBN_DBLCLK(IDC_LIST_SOURCE, OnDblclkListSource)
	ON_BN_CLICKED(IDC_TODESTINATION, OnTodestination)
	ON_BN_CLICKED(IDC_TOSOURCE, OnTosource)
	ON_BN_CLICKED(IDC_CHECK_CLEAR, OnCheckClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore 

//-------------------------------------------------------------------

BOOL DialogFind::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int i;
	for (i = 0; i < TerrainGeneratorHelper::LIQT_COUNT; ++i)
	{
		const int item = m_source.AddString (TerrainGeneratorHelper::LayerItemQueries [i]);
		IGNORE_RETURN (m_source.SetItemData (item, static_cast<DWORD> (i)));
	}

	return TRUE;  
}

//-------------------------------------------------------------------

void DialogFind::OnDblclkListDestination() 
{
	int selected = m_destination.GetCurSel ();
	if (selected != LB_ERR)
	{
		const int itemDestination = static_cast<int> (m_destination.GetItemData (selected));

		IGNORE_RETURN (m_destination.DeleteString (static_cast<uint> (selected)));

		const int itemSource = m_source.AddString (TerrainGeneratorHelper::LayerItemQueries [itemDestination]);
		IGNORE_RETURN (m_source.SetItemData (itemSource, static_cast<DWORD> (itemDestination)));
	}
}

//-------------------------------------------------------------------

void DialogFind::OnDblclkListSource() 
{
	int selected = m_source.GetCurSel ();
	if (selected != LB_ERR)
	{
		const int itemSource = static_cast<int> (m_source.GetItemData (selected));

		IGNORE_RETURN (m_source.DeleteString (static_cast<uint> (selected)));

		const int itemDestination = m_destination.AddString (TerrainGeneratorHelper::LayerItemQueries [itemSource]);
		IGNORE_RETURN (m_destination.SetItemData (itemDestination, static_cast<DWORD> (itemSource)));
	}
}

//-------------------------------------------------------------------

void DialogFind::OnTodestination() 
{
	OnDblclkListSource ();
}

//-------------------------------------------------------------------

void DialogFind::OnTosource() 
{
	OnDblclkListDestination ();
}

//-------------------------------------------------------------------

void DialogFind::OnOK() 
{
	int i;
	for (i = 0; i < m_destination.GetCount (); ++i)
		query.add (static_cast<TerrainGeneratorHelper::LayerItemQueryType> (m_destination.GetItemData (i)));

	CDialog::OnOK();
}

//-------------------------------------------------------------------

void DialogFind::OnCheckClear() 
{
	IGNORE_RETURN (UpdateData (true));
}

//-------------------------------------------------------------------

