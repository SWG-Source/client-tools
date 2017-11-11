//
// FormBitmapFamily.cpp
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormBitmapFamily.h"

#include "CDib.h"
#include "BitmapPreviewFrame.h"
#include "MainFrame.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"
#include "sharedTerrain/BitmapGroup.h"

//-------------------------------------------------------------------

FormBitmapFamily::FormBitmapFamilyViewData::FormBitmapFamilyViewData (void) :
	ViewData (),
	familyId (0),
	bitmapGroup (0)
{
}

//-------------------------------------------------------------------

FormBitmapFamily::FormBitmapFamilyViewData::~FormBitmapFamilyViewData (void)
{
	bitmapGroup = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormBitmapFamily, PropertyView)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormBitmapFamily::FormBitmapFamily() : 
	PropertyView(FormBitmapFamily::IDD),
	data (),
	m_image (0),
	m_tooltip (),
	//-- widgets
	m_name ()
{
	//{{AFX_DATA_INIT(FormBitmapFamily)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormBitmapFamily::~FormBitmapFamily()
{
	// just a pointer, doesn't own it
	m_image = 0;
}

//-------------------------------------------------------------------

void FormBitmapFamily::DoDataExchange(CDataExchange* pDX)
{
	PropertyView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormBitmapFamily)
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormBitmapFamily, PropertyView)
	//{{AFX_MSG_MAP(FormBitmapFamily)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormBitmapFamily::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormBitmapFamily::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormBitmapFamily::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormBitmapFamilyViewData* fffvd = dynamic_cast<FormBitmapFamilyViewData*> (vd);
	NOT_NULL (fffvd);

	data = *fffvd;
}

//-------------------------------------------------------------------

void FormBitmapFamily::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	{
		IGNORE_RETURN (m_tooltip.Create(this));
		m_tooltip.Activate(TRUE);
		IGNORE_RETURN (m_tooltip.SetMaxTipWidth (150));
	}

	m_name = data.bitmapGroup->getFamilyName (data.familyId);

	//-- update settings
	m_image = data.bitmapGroup->getFamilyBitmap(data.familyId);


	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;

	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormBitmapFamily::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormBitmapFamily::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormBitmapFamily::ApplyChanges ()
{
	if (!m_initialized || !m_image)
	{
		return;
	}

	//-- update the controls/data
	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (m_image);

		CString buffer;

		//-- tell the document we've modified data
		GetDocument ()->SetModifiedFlag (true);

		//-- update the bitmap
		safe_cast<TerrainEditorApp*> (AfxGetApp ())->showBitmapPreview ();
		GetParentFrame ()->ActivateFrame ();

		TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);
		
		if (doc->getBitmapPreviewFrame ())
		{
			doc->getBitmapPreviewFrame ()->updateBitmap (*m_image);
		}

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}



//-------------------------------------------------------------------

BOOL FormBitmapFamily::PreTranslateMessage(MSG* pMsg) 
{
	m_tooltip.RelayEvent(pMsg);
	
	return PropertyView::PreTranslateMessage(pMsg);
}


//-------------------------------------------------------------------

