//
// FormRadialFamily.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormRadialFamily.h"

#include "sharedTerrain/RadialGroup.h"
#include "PropertyView.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

FormRadialFamily::FormRadialFamilyViewData::FormRadialFamilyViewData (void) :
	ViewData (),
	familyId (0),
	radialGroup (0)
{
}

//-------------------------------------------------------------------

FormRadialFamily::FormRadialFamilyViewData::~FormRadialFamilyViewData (void)
{
	radialGroup = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormRadialFamily,  PropertyView)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormRadialFamily::FormRadialFamily() : 
	PropertyView(FormRadialFamily::IDD),
	data (),

	//-- widgets
	m_editDensity (this),
	m_sliderDensity (),
	m_childList (),
	m_windowColor (),
	m_name ()
{
	//{{AFX_DATA_INIT(FormRadialFamily)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormRadialFamily::~FormRadialFamily()
{
}

//-------------------------------------------------------------------

void FormRadialFamily::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormRadialFamily)
	DDX_Control(pDX, IDC_SLIDER_DENSITY, m_sliderDensity);
	DDX_Control(pDX, IDC_EDIT_DENSITY, m_editDensity);
	DDX_Control(pDX, IDC_CHILDRENLIST, m_childList);
	DDX_Control(pDX, IDC_STATIC_WINDOW_COLOR, m_windowColor);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormRadialFamily, PropertyView)
	//{{AFX_MSG_MAP(FormRadialFamily)
	ON_BN_CLICKED(IDC_CHOOSECOLOR, OnChoosecolor)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormRadialFamily::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormRadialFamily::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormRadialFamily::OnDraw(CDC* pDC) 
{
	PropertyView::OnDraw (pDC);

	// TODO: Add your specialized code here and/or call the base class
	PackedRgb color = data.radialGroup->getFamilyColor (data.familyId);
	
	CRect rect;
	m_windowColor.GetWindowRect (&rect);
	ScreenToClient (&rect);

	CBrush brush;
	IGNORE_RETURN (brush.CreateSolidBrush (RGB (color.r, color.g, color.b)));
	IGNORE_RETURN (pDC->SelectObject (&brush));
	IGNORE_RETURN (pDC->Rectangle (&rect));
}

//-------------------------------------------------------------------

void FormRadialFamily::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormRadialFamilyViewData* frfvd = dynamic_cast<FormRadialFamilyViewData*> (vd);
	NOT_NULL (frfvd);

	data = *frfvd;
}

//-------------------------------------------------------------------

void FormRadialFamily::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	m_name    = data.radialGroup->getFamilyName (data.familyId);
	m_editDensity.LinkSmartSliderCtrl (&m_sliderDensity);
	m_editDensity.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editDensity.SetValue (data.radialGroup->getFamilyDensity (data.familyId)));

	IGNORE_RETURN (m_childList.DeleteAllItems ());
	IGNORE_RETURN (m_childList.InsertColumn (0, "Name",         LVCFMT_LEFT, 60, 0));
	IGNORE_RETURN (m_childList.InsertColumn (1, "Weight",       LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (2, "Distance",     LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (3, "Min Width",    LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (4, "Max Width",    LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (5, "Min Height",   LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (6, "Max Height",   LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (7, "Maintain Aspect Ratio",LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (8, "Sway",         LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (9, "Displacement", LVCFMT_LEFT, 40, 1));
	IGNORE_RETURN (m_childList.InsertColumn (10, "Period",       LVCFMT_LEFT, 40, 1));

	CString buffer;

	int i;
	for (i = 0; i < data.radialGroup->getFamilyNumberOfChildren (data.familyId); i++)
	{
		const RadialGroup::FamilyChildData fcd = data.radialGroup->getFamilyChild (data.familyId, i);

		IGNORE_RETURN (m_childList.InsertItem (i, fcd.shaderTemplateName));

		buffer.Format ("%1.2f", fcd.weight);
		IGNORE_RETURN (m_childList.SetItemText (i, 1, buffer));

		buffer.Format ("%1.2f", fcd.distance);
		IGNORE_RETURN (m_childList.SetItemText (i, 2, buffer));

		buffer.Format ("%1.2f", fcd.minWidth);
		IGNORE_RETURN (m_childList.SetItemText (i, 3, buffer));

		buffer.Format ("%1.2f", fcd.maxWidth);
		IGNORE_RETURN (m_childList.SetItemText (i, 4, buffer));

		buffer.Format ("%1.2f", fcd.minHeight);
		IGNORE_RETURN (m_childList.SetItemText (i, 5, buffer));

		buffer.Format ("%1.2f", fcd.maxHeight);
		IGNORE_RETURN (m_childList.SetItemText (i, 6, buffer));

		buffer.Format ("%s", fcd.maintainAspectRatio ? "yes" : "no");
		IGNORE_RETURN (m_childList.SetItemText (i, 7, buffer));

		buffer.Format ("%s", fcd.shouldSway ? "yes" : "no");
		IGNORE_RETURN (m_childList.SetItemText (i, 8, buffer));

		buffer.Format ("%1.2f", fcd.displacement);
		IGNORE_RETURN (m_childList.SetItemText (i, 9, buffer));

		buffer.Format ("%1.2f", fcd.period);
		IGNORE_RETURN (m_childList.SetItemText (i, 10, buffer));	
	}

	IGNORE_RETURN (UpdateData(false));

	m_initialized = true;
}

//---------------------------------------------------------------------------

void FormRadialFamily::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormRadialFamily::HasChanged () const
{
	return 
		(m_editDensity.GetValueFloat () != data.radialGroup->getFamilyDensity (data.familyId));
}

//-------------------------------------------------------------------

void FormRadialFamily::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData(TRUE));

	if (HasChanged ())
	{
		NOT_NULL (data.radialGroup);

		data.radialGroup->setFamilyDensity (data.familyId, m_editDensity.GetValueFloat ());

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormRadialFamily::OnChoosecolor() 
{
	PackedRgb prgb = data.radialGroup->getFamilyColor (data.familyId);
	CColorDialog dlg (RGB (prgb.r, prgb.g, prgb.b));  //lint !e1924  //-- c-style cast
	if (dlg.DoModal () == IDOK)
	{
		prgb.r = GetRValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style cast
		prgb.g = GetGValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style cast
		prgb.b = GetBValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style cast
		data.radialGroup->setFamilyColor (data.familyId, prgb);

		TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*>(GetDocument());
		doc->UpdateAllViews (this);

		Invalidate ();
	}
}

//-------------------------------------------------------------------
