//===================================================================
//
// FormEnvironmentFamily.cpp
// asommers
// 
// copyright 2001, sony online entertainment
//
//===================================================================

#include "FirstTerrainEditor.h"
#include "FormEnvironmentFamily.h"

#include "CDib.h"
#include "DialogPalette.h"
#include "PropertyView.h"
#include "RecentDirectory.h"
#include "TerrainEditorDoc.h"
#include "sharedTerrain/EnvironmentGroup.h"
#include "sharedUtility/FileName.h"

//===================================================================

FormEnvironmentFamily::FormEnvironmentFamilyViewData::FormEnvironmentFamilyViewData (void) :
	ViewData (),
	familyId (0),
	environmentGroup (0)
{
}

//-------------------------------------------------------------------

FormEnvironmentFamily::FormEnvironmentFamilyViewData::~FormEnvironmentFamilyViewData (void)
{
	environmentGroup = 0;
}

//===================================================================

IMPLEMENT_DYNCREATE(FormEnvironmentFamily,  PropertyView)

//===================================================================

#pragma warning(push)
#pragma warning(disable:4355)

FormEnvironmentFamily::FormEnvironmentFamily() : 
	PropertyView(FormEnvironmentFamily::IDD),
	data (),

	//-- widgets
	m_editFeatherClamp (this),
	m_sliderFeatherClamp (),
	m_windowColor (),
	m_name ()
{
	//{{AFX_DATA_INIT(FormEnvironmentFamily)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormEnvironmentFamily::~FormEnvironmentFamily()
{
}

//-------------------------------------------------------------------

void FormEnvironmentFamily::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormEnvironmentFamily)
	DDX_Control(pDX, IDC_EDIT_FEATHERCLAMP, m_editFeatherClamp);
	DDX_Control(pDX, IDC_SLIDER_FEATHERCLAMP, m_sliderFeatherClamp);
	DDX_Control(pDX, IDC_STATIC_WINDOW_COLOR, m_windowColor);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormEnvironmentFamily, PropertyView)
	//{{AFX_MSG_MAP(FormEnvironmentFamily)
	ON_BN_CLICKED(IDC_CHOOSECOLOR, OnChoosecolor)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormEnvironmentFamily::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormEnvironmentFamily::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormEnvironmentFamily::OnDraw(CDC* pDC) 
{
	PropertyView::OnDraw (pDC);

	// TODO: Add your specialized code here and/or call the base class
	PackedRgb color = data.environmentGroup->getFamilyColor (data.familyId);
	
	CRect rect;
	m_windowColor.GetWindowRect (&rect);
	ScreenToClient (&rect);

	CBrush brush;
	IGNORE_RETURN (brush.CreateSolidBrush (RGB (color.r, color.g, color.b)));
	IGNORE_RETURN (pDC->SelectObject (&brush));
	IGNORE_RETURN (pDC->Rectangle (&rect));
}

//-------------------------------------------------------------------

void FormEnvironmentFamily::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormEnvironmentFamilyViewData* fsfvd = dynamic_cast<FormEnvironmentFamilyViewData*> (vd);
	NOT_NULL (fsfvd);

	data = *fsfvd;
}

//-------------------------------------------------------------------

void FormEnvironmentFamily::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	m_name      = data.environmentGroup->getFamilyName (data.familyId);

	m_editFeatherClamp.LinkSmartSliderCtrl (&m_sliderFeatherClamp);
	m_editFeatherClamp.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherClamp.SetValue (data.environmentGroup->getFamilyFeatherClamp (data.familyId)));

	IGNORE_RETURN (UpdateData(false));

	m_initialized = true;
}

//---------------------------------------------------------------------------

void FormEnvironmentFamily::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormEnvironmentFamily::HasChanged (void) const
{
	return true;
}

//-------------------------------------------------------------------

void FormEnvironmentFamily::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (TRUE));

	if (HasChanged ())
	{
		NOT_NULL (data.environmentGroup);

		data.environmentGroup->setFamilyFeatherClamp (data.familyId, m_editFeatherClamp.GetValueFloat ());

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormEnvironmentFamily::OnChoosecolor() 
{
	PackedRgb prgb = data.environmentGroup->getFamilyColor (data.familyId);
	CColorDialog dlg (RGB (prgb.r, prgb.g, prgb.b));  //lint !e1924  //-- c-style case
	if (dlg.DoModal () == IDOK)
	{
		prgb.r = GetRValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style case
		prgb.g = GetGValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style case
		prgb.b = GetBValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style case
		data.environmentGroup->setFamilyColor (data.familyId, prgb);

		TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*>(GetDocument());
		doc->UpdateAllViews (this);

		Invalidate ();
	}
}

//-------------------------------------------------------------------

