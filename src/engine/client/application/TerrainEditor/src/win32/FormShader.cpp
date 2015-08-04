//
// FormShader.cpp
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormShader.h"

#include "sharedTerrain/ShaderGroup.h"

//-------------------------------------------------------------------

FormShader::FormShaderViewData::FormShaderViewData (void) :
	ViewData (),
	childName (),
	familyId (0),
	shaderGroup (0)
{
}

//-------------------------------------------------------------------

FormShader::FormShaderViewData::~FormShaderViewData (void)
{
	shaderGroup = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormShader, PropertyView)

//-------------------------------------------------------------------

FormShader::FormShader() :
	PropertyView(FormShader::IDD),
	data (),

	//-- widgets
	m_familyColorWindow (),
	m_name (),
	m_weight (0.f),
	m_familyName ()
{
	//{{AFX_DATA_INIT(FormShader)
	m_name = _T("");
	m_familyName = _T("");
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

FormShader::~FormShader()
{
}

//-------------------------------------------------------------------

void FormShader::DoDataExchange(CDataExchange* pDX)
{
	PropertyView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormShader)
	DDX_Control(pDX, IDC_WEIGHT, m_weight);
	DDX_Control(pDX, IDC_STATIC_FAMILY_COLOR, m_familyColorWindow);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_EDIT_FAMILY_NAME, m_familyName);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormShader, PropertyView)
	//{{AFX_MSG_MAP(FormShader)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormShader::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormShader::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormShader::OnDraw(CDC* pDC) 
{
	// TODO: Add your specialized code here and/or call the base class
	PropertyView::OnDraw (pDC);

	// TODO: Add your specialized code here and/or call the base class
	PackedRgb color = data.shaderGroup->getFamilyColor (data.familyId);
	
	CRect rect;
	m_familyColorWindow.GetWindowRect (&rect);
	ScreenToClient (&rect);

	CBrush brush;
	IGNORE_RETURN (brush.CreateSolidBrush (RGB (color.r, color.g, color.b)));
	IGNORE_RETURN (pDC->SelectObject (&brush));
	IGNORE_RETURN (pDC->Rectangle (&rect));
}

//-------------------------------------------------------------------

void FormShader::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormShaderViewData* fsvd = dynamic_cast<FormShaderViewData*> (vd);
	NOT_NULL (fsvd);

	data = *fsvd;
}

//-------------------------------------------------------------------------

void FormShader::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();
	m_familyName = data.shaderGroup->getFamilyName (data.familyId);
	m_name       = data.childName;

	const ShaderGroup::FamilyChildData fcd = data.shaderGroup->getFamilyChild (data.familyId, data.childName);
	m_weight     = fcd.weight;

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormShader::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormShader::HasChanged () const
{
	const ShaderGroup::FamilyChildData fcd = data.shaderGroup->getFamilyChild (data.familyId, data.childName);

	return 
		(m_weight != fcd.weight);
}

//-------------------------------------------------------------------

void FormShader::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (data.shaderGroup);

		ShaderGroup::FamilyChildData fcd;
		fcd.familyId           = data.familyId;
		fcd.shaderTemplateName = data.childName;
		fcd.weight             = m_weight;

		data.shaderGroup->setFamilyChild (data.familyId, fcd);

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

