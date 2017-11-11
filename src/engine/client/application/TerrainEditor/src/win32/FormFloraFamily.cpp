//
// FormFloraFamily.cpp
// asommers
//
// copyright 2001, sony online entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormFloraFamily.h"

#include "sharedTerrain/Affector.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

FormFloraFamily::FormFloraFamilyViewData::FormFloraFamilyViewData (void) :
	ViewData (),
	familyId (0),
	floraGroup (0)
{
}

//-------------------------------------------------------------------
	
FormFloraFamily::FormFloraFamilyViewData::~FormFloraFamilyViewData (void)
{
	floraGroup = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormFloraFamily, PropertyView)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormFloraFamily::FormFloraFamily() : 
	PropertyView (FormFloraFamily::IDD),
	data (),

	//-- widgets
	m_editDensity (this),
	m_sliderDensity (),
	m_childList (),
	m_windowColor (),
	m_name (),
	m_floats (FALSE)
{
	//{{AFX_DATA_INIT(FormFloraFamily)
	m_name = _T("");
	m_floats = FALSE;
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormFloraFamily::~FormFloraFamily()
{
}

//-------------------------------------------------------------------

void FormFloraFamily::DoDataExchange(CDataExchange* pDX)
{
	PropertyView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormFloraFamily)
	DDX_Control(pDX, IDC_SLIDER_DENSITY, m_sliderDensity);
	DDX_Control(pDX, IDC_EDIT_DENSITY, m_editDensity);
	DDX_Control(pDX, IDC_CHILDRENLIST, m_childList);
	DDX_Control(pDX, IDC_STATIC_WINDOW_COLOR, m_windowColor);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Check(pDX, IDC_CHECK_FLOATS, m_floats);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormFloraFamily, PropertyView)
	//{{AFX_MSG_MAP(FormFloraFamily)
	ON_BN_CLICKED(IDC_CHOOSECOLOR, OnChoosecolor)
	ON_BN_CLICKED(IDC_CHECK_FLOATS, OnCheckFloats)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormFloraFamily::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormFloraFamily::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormFloraFamily::Initialize (PropertyView::ViewData* vd)
{
	FormFloraFamilyViewData* fffvd = dynamic_cast<FormFloraFamilyViewData*>(vd);
	NOT_NULL (fffvd);

	data = *fffvd;
}

//-------------------------------------------------------------------------

void FormFloraFamily::OnChoosecolor() 
{
	PackedRgb prgb = data.floraGroup->getFamilyColor (data.familyId);
	CColorDialog dlg (RGB (prgb.r, prgb.g, prgb.b));  //lint !e1924  //-- c-style cast
	if (dlg.DoModal () == IDOK)
	{
		prgb.r = GetRValue (dlg.m_cc.rgbResult);      //lint !e1924  //-- c-style cast
		prgb.g = GetGValue (dlg.m_cc.rgbResult);      //lint !e1924  //-- c-style cast
		prgb.b = GetBValue (dlg.m_cc.rgbResult);      //lint !e1924  //-- c-style cast
		data.floraGroup->setFamilyColor (data.familyId, prgb);

		TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument());

		doc->UpdateAllViews (this);
		Invalidate ();
	}
}

//-------------------------------------------------------------------

void FormFloraFamily::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	m_name    = data.floraGroup->getFamilyName (data.familyId);
	m_editDensity.LinkSmartSliderCtrl (&m_sliderDensity);
	m_editDensity.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editDensity.SetValue (data.floraGroup->getFamilyDensity (data.familyId)));
	m_floats  = data.floraGroup->getFamilyFloats (data.familyId);

	IGNORE_RETURN (m_childList.DeleteAllItems ());
	IGNORE_RETURN (m_childList.InsertColumn (0, "Name",         LVCFMT_LEFT, 60, 0));
	IGNORE_RETURN (m_childList.InsertColumn (1, "Weight",       LVCFMT_LEFT, 75, 1));
	IGNORE_RETURN (m_childList.InsertColumn (2, "Sway",         LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (3, "Displacement", LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (4, "Period",       LVCFMT_LEFT, 40, 1));

	CString buffer;

	int i;
	for (i = 0; i < data.floraGroup->getFamilyNumberOfChildren (data.familyId); i++)
	{
		const FloraGroup::FamilyChildData fcd = data.floraGroup->getFamilyChild (data.familyId, i);

		IGNORE_RETURN (m_childList.InsertItem (i, fcd.appearanceTemplateName));

		buffer.Format ("%1.2f", fcd.weight);
		IGNORE_RETURN (m_childList.SetItemText (i, 1, buffer));

		buffer.Format ("%s", fcd.shouldSway ? "yes" : "no");
		IGNORE_RETURN (m_childList.SetItemText (i, 2, buffer));

		buffer.Format ("%1.2f", fcd.displacement);
		IGNORE_RETURN (m_childList.SetItemText (i, 3, buffer));

		buffer.Format ("%1.2f", fcd.period);
		IGNORE_RETURN (m_childList.SetItemText (i, 4, buffer));
	}

	IGNORE_RETURN (UpdateData(false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormFloraFamily::OnDraw(CDC* pDC) 
{
	PropertyView::OnDraw (pDC);
	PackedRgb color = data.floraGroup->getFamilyColor (data.familyId);
	
	CRect rect;
	m_windowColor.GetWindowRect (&rect);
	ScreenToClient (&rect);

	CBrush brush;
	IGNORE_RETURN (brush.CreateSolidBrush (RGB (color.r, color.g, color.b)));
	IGNORE_RETURN (pDC->SelectObject (&brush));
	IGNORE_RETURN (pDC->Rectangle (&rect));
}

//-------------------------------------------------------------------

void FormFloraFamily::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormFloraFamily::HasChanged () const
{
	return 
		(m_editDensity.GetValueFloat () != data.floraGroup->getFamilyDensity (data.familyId)) ||
		((m_floats == TRUE)             != data.floraGroup->getFamilyFloats (data.familyId));
}

//-------------------------------------------------------------------

void FormFloraFamily::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (data.floraGroup);

		data.floraGroup->setFamilyDensity (data.familyId, m_editDensity.GetValueFloat ());
		data.floraGroup->setFamilyFloats (data.familyId, m_floats == TRUE);

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormFloraFamily::OnCheckFloats() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

