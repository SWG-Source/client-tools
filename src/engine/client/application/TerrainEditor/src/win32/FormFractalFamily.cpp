//
// FormFractalFamily.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormFractalFamily.h"

#include "CDib.h"
#include "FractalPreviewFrame.h"
#include "MainFrame.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"
#include "sharedFractal/MultiFractal.h"
#include "sharedTerrain/FractalGroup.h"

//-------------------------------------------------------------------

FormFractalFamily::FormFractalFamilyViewData::FormFractalFamilyViewData (void) :
	ViewData (),
	familyId (0),
	fractalGroup (0)
{
}

//-------------------------------------------------------------------

FormFractalFamily::FormFractalFamilyViewData::~FormFractalFamilyViewData (void)
{
	fractalGroup = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormFractalFamily, PropertyView)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormFractalFamily::FormFractalFamily() : 
	PropertyView(FormFractalFamily::IDD),
	data (),
	m_multiFractal (0),
	m_tooltip (),

	//-- widgets
	m_editSeed (false, true),
	m_spinSeed (),
	m_offsetZ (true),
	m_offsetX (true),
	m_editOctaves (this),
	m_editScaleZ (this),
	m_editScaleX (this),
	m_editGain (this),
	m_editBias (this),
	m_editAmplitude (this),
	m_editFrequency (this),
	m_sliderOctaves (),
	m_sliderScaleZ (),
	m_sliderScaleX (),
	m_sliderGain (),
	m_sliderFrequency (),
	m_sliderBias (),
	m_sliderAmplitude (),
	m_combinationRule (),
	m_name (),
	m_useBias (FALSE),
	m_useGain (FALSE)
{
	//{{AFX_DATA_INIT(FormFractalFamily)
	m_name = _T("");
	m_useBias = FALSE;
	m_useGain = FALSE;
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormFractalFamily::~FormFractalFamily()
{
	m_multiFractal = 0;
}

//-------------------------------------------------------------------

void FormFractalFamily::DoDataExchange(CDataExchange* pDX)
{
	PropertyView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormFractalFamily)
	DDX_Control(pDX, IDC_SEED, m_editSeed);
	DDX_Control(pDX, IDC_SPIN_SEED, m_spinSeed);
	DDX_Control(pDX, IDC_OFFSETZ, m_offsetZ);
	DDX_Control(pDX, IDC_OFFSETX, m_offsetX);
	DDX_Control(pDX, IDC_OCTAVES, m_editOctaves);
	DDX_Control(pDX, IDC_SCALEZ, m_editScaleZ);
	DDX_Control(pDX, IDC_SCALEX, m_editScaleX);
	DDX_Control(pDX, IDC_GAIN, m_editGain);
	DDX_Control(pDX, IDC_BIAS, m_editBias);
	DDX_Control(pDX, IDC_AMPLITUDE, m_editAmplitude);
	DDX_Control(pDX, IDC_FREQUENCY, m_editFrequency);
	DDX_Control(pDX, IDC_SLIDER_OCTAVES, m_sliderOctaves);
	DDX_Control(pDX, IDC_SLIDER_SCALEZ, m_sliderScaleZ);
	DDX_Control(pDX, IDC_SLIDER_SCALEX, m_sliderScaleX);
	DDX_Control(pDX, IDC_SLIDER_GAIN, m_sliderGain);
	DDX_Control(pDX, IDC_SLIDER_FREQUENCY, m_sliderFrequency);
	DDX_Control(pDX, IDC_SLIDER_BIAS, m_sliderBias);
	DDX_Control(pDX, IDC_SLIDER_AMPLITUDE, m_sliderAmplitude);
	DDX_Control(pDX, IDC_COMBINATIONRULE, m_combinationRule);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Check(pDX, IDC_CHECK_BIAS, m_useBias);
	DDX_Check(pDX, IDC_CHECK_GAIN, m_useGain);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormFractalFamily, PropertyView)
	//{{AFX_MSG_MAP(FormFractalFamily)
	ON_CBN_SELCHANGE(IDC_COMBINATIONRULE, OnSelchangeCombinationrule)
	ON_BN_CLICKED(IDC_CHECK_BIAS, OnCheckBias)
	ON_EN_CHANGE(IDC_SEED, OnChangeSeed)
	ON_BN_CLICKED(IDC_CHECK_GAIN, OnCheckGain)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_OFFSETX, OnChangeOffsetx)
	ON_EN_CHANGE(IDC_OFFSETZ, OnChangeOffsetz)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormFractalFamily::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormFractalFamily::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormFractalFamily::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormFractalFamilyViewData* fffvd = dynamic_cast<FormFractalFamilyViewData*> (vd);
	NOT_NULL (fffvd);

	data = *fffvd;
}

//-------------------------------------------------------------------

void FormFractalFamily::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	{
		IGNORE_RETURN (m_tooltip.Create(this));
		m_tooltip.Activate(TRUE);
		IGNORE_RETURN (m_tooltip.SetMaxTipWidth (150));

		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_AMPLITUDE), IDS_AMPLITUDE));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_FREQUENCY), IDS_FREQUENCY));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_BIAS), IDS_BIAS));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_GAIN), IDS_GAIN));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_SCALEX), IDS_SCALEX));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_SCALEZ), IDS_SCALEZ));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_OCTAVES), IDS_NUMBEROFOCTAVES));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_COMBINATIONRULE), IDS_COMBINATIONRULE));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_SEED), IDS_SEED));
		// m_tooltip.AddTool(GetDlgItem(IDC_<name>), "<text>");
	}

	m_name = data.fractalGroup->getFamilyName (data.familyId);

	//-- update settings
	m_multiFractal = data.fractalGroup->getFamilyMultiFractal (data.familyId);

	m_editSeed.SetNumberInt (static_cast<int> (m_multiFractal->getSeed ()));
	m_spinSeed.SetBuddy (&m_editSeed);
	m_spinSeed.SetRange32 (0, 1 << 30);

	m_editScaleX.LinkSmartSliderCtrl (&m_sliderScaleX);
	m_editScaleX.SetParams (0.0f, 0.1f, 20, "%1.5f");
	IGNORE_RETURN (m_editScaleX.SetValue (m_multiFractal->getScaleX ()));

	m_editScaleZ.LinkSmartSliderCtrl (&m_sliderScaleZ);
	m_editScaleZ.SetParams (0.0f, 0.1f, 20, "%1.5f");
	IGNORE_RETURN (m_editScaleZ.SetValue (m_multiFractal->getScaleY ()));

	m_offsetX = m_multiFractal->getOffsetX ();
	m_offsetZ = m_multiFractal->getOffsetY ();

	m_useBias = m_multiFractal->getUseBias ();

	m_editBias.LinkSmartSliderCtrl (&m_sliderBias);
	m_editBias.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editBias.SetValue (m_multiFractal->getBias ()));

	m_useGain = m_multiFractal->getUseGain ();

	m_editGain.LinkSmartSliderCtrl (&m_sliderGain);
	m_editGain.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editGain.SetValue (m_multiFractal->getGain ()));

	m_editOctaves.LinkSmartSliderCtrl (&m_sliderOctaves);
	m_editOctaves.SetParams (1, 4, 1);
	IGNORE_RETURN (m_editOctaves.SetValue (m_multiFractal->getNumberOfOctaves ()));

	IGNORE_RETURN (m_combinationRule.SetCurSel (m_multiFractal->getCombinationRule ()));

	m_editFrequency.LinkSmartSliderCtrl (&m_sliderFrequency);
	m_editFrequency.SetParams (0.0f, 4.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFrequency.SetValue (m_multiFractal->getFrequency ()));

	m_editAmplitude.LinkSmartSliderCtrl (&m_sliderAmplitude);
	m_editAmplitude.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editAmplitude.SetValue (m_multiFractal->getAmplitude ()));

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;

	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFractalFamily::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormFractalFamily::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormFractalFamily::ApplyChanges ()
{
	if (!m_initialized)
		return;

	//-- update the controls/data
	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (m_multiFractal);

		CString buffer;

		//-- update seed
		m_multiFractal->setSeed (static_cast<uint32> (m_editSeed.GetNumberInt ()));

		//-- update scale
		m_editScaleX.GetWindowText (buffer);
		const real scaleX = static_cast<real> (atof (buffer));
		m_editScaleZ.GetWindowText (buffer);
		const real scaleZ = static_cast<real> (atof (buffer));

		m_multiFractal->setScale (scaleX, scaleZ);

		//-- update offset
		m_multiFractal->setOffset (m_offsetX, m_offsetZ);

		//-- update bias
		m_editBias.GetWindowText (buffer);
		m_multiFractal->setBias (m_useBias == TRUE, static_cast<real> (atof (buffer)));

		//-- update gain
		m_editGain.GetWindowText (buffer);
		m_multiFractal->setGain (m_useGain == TRUE, static_cast<real> (atof (buffer)));

		//-- update octaves
		m_editOctaves.GetWindowText (buffer);
		m_multiFractal->setNumberOfOctaves (atoi (buffer));

		IGNORE_RETURN (m_editFrequency.EnableWindow (m_multiFractal->getNumberOfOctaves () != 1 ? TRUE : FALSE));
		IGNORE_RETURN (m_sliderFrequency.EnableWindow (m_multiFractal->getNumberOfOctaves () != 1 ? TRUE : FALSE));
		IGNORE_RETURN (m_editAmplitude.EnableWindow (m_multiFractal->getNumberOfOctaves () != 1 ? TRUE : FALSE));
		IGNORE_RETURN (m_sliderAmplitude.EnableWindow (m_multiFractal->getNumberOfOctaves () != 1 ? TRUE : FALSE));

		//-- update combo rule
		m_multiFractal->setCombinationRule (static_cast<MultiFractal::CombinationRule> (m_combinationRule.GetCurSel ()));

		//-- update amplitude
		m_editAmplitude.GetWindowText (buffer);
		m_multiFractal->setAmplitude (static_cast<real> (atof (buffer)));

		//-- update frequency
		m_editFrequency.GetWindowText (buffer);
		m_multiFractal->setFrequency (static_cast<real> (atof (buffer)));

		//-- tell the document we've modified data
		GetDocument ()->SetModifiedFlag (true);

		//-- update the bitmap
		safe_cast<TerrainEditorApp*> (AfxGetApp ())->showFractalPreview ();
		GetParentFrame ()->ActivateFrame ();

		TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);
		
		if (doc->getFractalPreviewFrame ())
			doc->getFractalPreviewFrame ()->updateBitmap (*m_multiFractal);

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormFractalFamily::OnCheckBias() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFractalFamily::OnCheckGain() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFractalFamily::OnChangeSeed() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

BOOL FormFractalFamily::PreTranslateMessage(MSG* pMsg) 
{
	m_tooltip.RelayEvent(pMsg);
	
	return PropertyView::PreTranslateMessage(pMsg);
}

//-------------------------------------------------------------------

void FormFractalFamily::OnChangeOffsetx() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFractalFamily::OnChangeOffsetz() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFractalFamily::OnSelchangeCombinationrule() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

