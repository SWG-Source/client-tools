//
// FormFlora.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormFlora.h"

#include "sharedTerrain/FloraGroup.h"

//-------------------------------------------------------------------

FormFlora::FormFloraViewData::FormFloraViewData () : 
	ViewData (),
	childName (),
	familyId (0),
	floraGroup (0)
{
}

//-------------------------------------------------------------------
	
FormFlora::FormFloraViewData::~FormFloraViewData ()
{
	floraGroup = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormFlora, PropertyView)

//-------------------------------------------------------------------

FormFlora::FormFlora() : 
	PropertyView (FormFlora::IDD),
	data (),

	//-- widgets
	m_familyColorWindow (),
	m_period (false),
	m_displacement (false),
	m_name (),
	m_weight (false),
	m_shouldSway (FALSE),
	m_familyName (),
	m_alignToTerrain (FALSE),
	m_shouldScale (FALSE),
	m_minimumScale (false),
	m_maximumScale (false)
{
	//{{AFX_DATA_INIT(FormFlora)
	m_name = _T("");
	m_shouldSway = FALSE;
	m_familyName = _T("");
	m_alignToTerrain = FALSE;
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

FormFlora::~FormFlora()
{
}

//-------------------------------------------------------------------

void FormFlora::DoDataExchange(CDataExchange* pDX)
{
	PropertyView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormFlora)
	DDX_Control(pDX, IDC_WEIGHT, m_weight);
	DDX_Control(pDX, IDC_EDIT_FREQUENCY, m_period);
	DDX_Control(pDX, IDC_EDIT_DISPLACEMENT, m_displacement);
	DDX_Control(pDX, IDC_STATIC_FAMILY_COLOR, m_familyColorWindow);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Check(pDX, IDC_CHECK_SWAY, m_shouldSway);
	DDX_Text(pDX, IDC_EDIT_FAMILY_NAME, m_familyName);
	DDX_Check(pDX, IDC_ALIGN_TO_TERRAIN, m_alignToTerrain);
	DDX_Check(pDX, IDC_CHECK_SCALE, m_shouldScale);
	DDX_Control(pDX, IDC_EDIT_MINIMUM_SCALE, m_minimumScale);
	DDX_Control(pDX, IDC_EDIT_MAXIMUM_SCALE, m_maximumScale);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormFlora, PropertyView)
	//{{AFX_MSG_MAP(FormFlora)
	ON_BN_CLICKED(IDC_CHECK_SWAY, OnCheckSway)
	ON_BN_CLICKED(IDC_ALIGN_TO_TERRAIN, OnAlignToTerrain)
	ON_EN_CHANGE(IDC_EDIT_DISPLACEMENT, OnChangeEditDisplacement)
	ON_EN_CHANGE(IDC_EDIT_FREQUENCY, OnChangeEditFrequency)
	ON_EN_CHANGE(IDC_WEIGHT, OnChangeWeight)
	ON_BN_CLICKED(IDC_CHECK_SCALE, OnCheckScale)
	ON_EN_CHANGE(IDC_EDIT_MINIMUM_SCALE, OnChangeEditMinimumScale)
	ON_EN_CHANGE(IDC_EDIT_MAXIMUM_SCALE, OnChangeEditMaximumScale)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormFlora::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormFlora::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormFlora::OnDraw(CDC* pDC) 
{
	// TODO: Add your specialized code here and/or call the base class
	PropertyView::OnDraw (pDC);

	// TODO: Add your specialized code here and/or call the base class
	PackedRgb color = data.floraGroup->getFamilyColor (data.familyId);
	
	CRect rect;
	m_familyColorWindow.GetWindowRect (&rect);
	ScreenToClient (&rect);

	CBrush brush;
	IGNORE_RETURN (brush.CreateSolidBrush (RGB (color.r, color.g, color.b)));
	IGNORE_RETURN (pDC->SelectObject (&brush));	
	IGNORE_RETURN (pDC->Rectangle (&rect));
}

//-------------------------------------------------------------------

void FormFlora::Initialize (PropertyView::ViewData* vd)
{
	FormFloraViewData* ffvd = dynamic_cast<FormFloraViewData*>(vd);
	NOT_NULL (ffvd);

	data = *ffvd;
}

//-------------------------------------------------------------------------

void FormFlora::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	m_name       = data.childName;
	m_familyName = data.floraGroup->getFamilyName (data.familyId);

	const FloraGroup::FamilyChildData fcd = data.floraGroup->getFamilyChild (data.familyId, data.childName);
	m_weight         = fcd.weight;
	m_displacement   = fcd.displacement;
	m_period         = fcd.period;
	m_shouldSway     = fcd.shouldSway;
	m_alignToTerrain = fcd.alignToTerrain;
	m_shouldScale    = fcd.shouldScale;
	m_minimumScale   = fcd.minimumScale;
	m_maximumScale   = fcd.maximumScale;

	IGNORE_RETURN (UpdateData (false));

	IGNORE_RETURN (m_displacement.EnableWindow (m_shouldSway));
	IGNORE_RETURN (m_period.EnableWindow (m_shouldSway));
	IGNORE_RETURN (m_minimumScale.EnableWindow (m_shouldScale));
	IGNORE_RETURN (m_maximumScale.EnableWindow (m_shouldScale));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormFlora::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormFlora::HasChanged () const
{
	const FloraGroup::FamilyChildData fcd = data.floraGroup->getFamilyChild (data.familyId, data.childName);

	return 
		(m_weight                   != fcd.weight) ||
		(m_displacement             != fcd.displacement) ||
		(m_period                   != fcd.period) ||
		((m_shouldSway == TRUE)     != fcd.shouldSway) ||
		((m_alignToTerrain == TRUE) != fcd.alignToTerrain) ||
		((m_shouldScale == TRUE)    != fcd.shouldScale) ||
		(m_minimumScale             != fcd.minimumScale) ||
		(m_maximumScale             != fcd.maximumScale);
}

//-------------------------------------------------------------------

void FormFlora::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (data.floraGroup);

		FloraGroup::FamilyChildData fcd;
		fcd.familyId               = data.familyId;
		fcd.weight                 = m_weight;
		fcd.appearanceTemplateName = data.childName;
		fcd.shouldSway             = m_shouldSway == TRUE;
		fcd.period                 = m_period;
		fcd.displacement           = m_displacement;
		fcd.alignToTerrain         = m_alignToTerrain == TRUE;
		fcd.shouldScale            = m_shouldScale == TRUE;
		fcd.minimumScale           = m_minimumScale;
		fcd.maximumScale           = m_maximumScale;

		data.floraGroup->setFamilyChild (data.familyId, fcd);

		IGNORE_RETURN (m_displacement.EnableWindow (m_shouldSway));
		IGNORE_RETURN (m_period.EnableWindow (m_shouldSway));
		IGNORE_RETURN (m_minimumScale.EnableWindow (m_shouldScale));
		IGNORE_RETURN (m_maximumScale.EnableWindow (m_shouldScale));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormFlora::OnCheckSway() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFlora::OnAlignToTerrain() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFlora::OnChangeEditDisplacement() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFlora::OnChangeEditFrequency() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFlora::OnChangeWeight() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFlora::OnCheckScale() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFlora::OnChangeEditMinimumScale() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFlora::OnChangeEditMaximumScale() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

