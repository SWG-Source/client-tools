//
// FormShaderFamily.cpp
// asommers
// 
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormShaderFamily.h"

#include "sharedTerrain/ShaderGroup.h"
#include "PropertyView.h"
#include "TerrainEditorDoc.h"

//-------------------------------------------------------------------

FormShaderFamily::FormShaderFamilyViewData::FormShaderFamilyViewData (void) :
	ViewData (),
	familyId (0),
	shaderGroup (0)
{
}

//-------------------------------------------------------------------

FormShaderFamily::FormShaderFamilyViewData::~FormShaderFamilyViewData (void)
{
	shaderGroup = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormShaderFamily,  PropertyView)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormShaderFamily::FormShaderFamily() : 
	PropertyView(FormShaderFamily::IDD),
	data (),

	//-- widgets
	m_editFeatherClamp (this),
	m_sliderFeatherClamp (),
	m_childList (),
	m_windowColor (),
	m_name ()
{
	//{{AFX_DATA_INIT(FormShaderFamily)
	m_name = _T("");
	m_surfacePropertiesName = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormShaderFamily::~FormShaderFamily()
{
}

//-------------------------------------------------------------------

void FormShaderFamily::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormShaderFamily)
	DDX_Control(pDX, IDC_EDIT_FEATHERCLAMP, m_editFeatherClamp);
	DDX_Control(pDX, IDC_SLIDER_FEATHERCLAMP, m_sliderFeatherClamp);
	DDX_Control(pDX, IDC_CHILDRENLIST, m_childList);
	DDX_Control(pDX, IDC_STATIC_WINDOW_COLOR, m_windowColor);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_EDIT_SURFACEPROPERTIES, m_surfacePropertiesName);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormShaderFamily, PropertyView)
	//{{AFX_MSG_MAP(FormShaderFamily)
	ON_BN_CLICKED(IDC_CHOOSECOLOR, OnChoosecolor)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormShaderFamily::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormShaderFamily::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormShaderFamily::OnDraw(CDC* pDC) 
{
	PropertyView::OnDraw (pDC);

	// TODO: Add your specialized code here and/or call the base class
	PackedRgb color = data.shaderGroup->getFamilyColor (data.familyId);
	
	CRect rect;
	m_windowColor.GetWindowRect (&rect);
	ScreenToClient (&rect);

	CBrush brush;
	IGNORE_RETURN (brush.CreateSolidBrush (RGB (color.r, color.g, color.b)));
	IGNORE_RETURN (pDC->SelectObject (&brush));
	IGNORE_RETURN (pDC->Rectangle (&rect));
}

//-------------------------------------------------------------------

void FormShaderFamily::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormShaderFamilyViewData* fsfvd = dynamic_cast<FormShaderFamilyViewData*> (vd);
	NOT_NULL (fsfvd);

	data = *fsfvd;
}

//-------------------------------------------------------------------

void FormShaderFamily::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	m_name        = data.shaderGroup->getFamilyName (data.familyId);
//	m_shaderSizeM = data.shaderGroup->getFamilyShaderSize (data.familyId);

	CRect rect;
	m_childList.GetWindowRect (&rect);
	ScreenToClient (&rect);
	int listWidth = rect.right - rect.left-4;

	IGNORE_RETURN (m_childList.InsertColumn (0, "Name", LVCFMT_LEFT, listWidth-75, 0));
	IGNORE_RETURN (m_childList.InsertColumn (1, "Weight", LVCFMT_LEFT, 75, 1));
	IGNORE_RETURN (m_childList.DeleteAllItems ());

	// Populate child list.
	for (int i=0; i<data.shaderGroup->getFamilyNumberOfChildren (data.familyId); i++)
	{
		const ShaderGroup::FamilyChildData fcd = data.shaderGroup->getFamilyChild (data.familyId, i);
		IGNORE_RETURN (m_childList.InsertItem (i, fcd.shaderTemplateName));
		CString s;
		s.Format ("%1.2f", fcd.weight);
		IGNORE_RETURN (m_childList.SetItemText (i, 1, s));
	}

	m_editFeatherClamp.LinkSmartSliderCtrl (&m_sliderFeatherClamp);
	m_editFeatherClamp.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherClamp.SetValue (data.shaderGroup->getFamilyFeatherClamp (data.familyId)));

	m_surfacePropertiesName = data.shaderGroup->getFamilySurfacePropertiesName (data.familyId);

	IGNORE_RETURN (UpdateData(false));

	m_initialized = true;
}

//---------------------------------------------------------------------------

void FormShaderFamily::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormShaderFamily::HasChanged (void) const
{
	return true;
}

//-------------------------------------------------------------------

void FormShaderFamily::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (TRUE));

	if (HasChanged ())
	{
		NOT_NULL (data.shaderGroup);

		data.shaderGroup->setFamilySurfacePropertiesName (data.familyId, m_surfacePropertiesName);
		data.shaderGroup->setFamilyFeatherClamp (data.familyId, m_editFeatherClamp.GetValueFloat ());

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormShaderFamily::OnChoosecolor() 
{
	PackedRgb prgb = data.shaderGroup->getFamilyColor (data.familyId);
	CColorDialog dlg (RGB (prgb.r, prgb.g, prgb.b));  //lint !e1924  //-- c-style case
	if (dlg.DoModal () == IDOK)
	{
		prgb.r = GetRValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style case
		prgb.g = GetGValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style case
		prgb.b = GetBValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style case
		data.shaderGroup->setFamilyColor (data.familyId, prgb);

		TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*>(GetDocument());
		doc->UpdateAllViews (this);

		Invalidate ();
	}
}

//-------------------------------------------------------------------
