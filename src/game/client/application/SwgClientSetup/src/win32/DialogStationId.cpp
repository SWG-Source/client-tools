// ======================================================================
//
// DialogStationId.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "DialogStationId.h"

#include "Options.h"
#include "SwgClientSetup.h"

// ======================================================================

DialogStationId::DialogStationId(CWnd* pParent /*=NULL*/)
	: CDialog(DialogStationId::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogStationId)
	m_radio = -1;

	m_lblSendStationIdInfo = _T("");
	m_lblSendStationIdQuery = _T("");
	m_lblPleaseInclude = _T("");
	m_lblDoNotInclude = _T("");

	m_lblBtnCancel = _T("");
	m_lblBtnNext = _T("");

	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogStationId::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogStationId)
	DDX_Radio(pDX, IDC_RADIO1, m_radio);

	DDX_Text(pDX, IDC_LBL_STATIONID_INFO, m_lblSendStationIdInfo);
	DDX_Text(pDX, IDC_LBL_STATIONID_QUERY, m_lblSendStationIdQuery);
	DDX_Text(pDX, IDC_RADIO1, m_lblPleaseInclude);
	DDX_Text(pDX, IDC_RADIO2, m_lblDoNotInclude);

	DDX_Text(pDX, ID_CANCEL, m_lblBtnCancel);
	DDX_Text(pDX, IDC_BUTTON_PROCEED, m_lblBtnNext);

	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogStationId, CDialog)
	//{{AFX_MSG_MAP(DialogStationId)
	ON_BN_CLICKED(IDC_BUTTON_PROCEED, OnButtonProceed)
	ON_BN_CLICKED(ID_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogStationId::OnInitDialog() 
{
	CDialog::OnInitDialog();

	VERIFY(m_lblSendStationIdInfo.LoadString(IDS_SENDSTATION_INFO));
	VERIFY(m_lblSendStationIdQuery.LoadString(IDS_SENDSTATION_QUESTION));
	VERIFY(m_lblPleaseInclude.LoadString(IDS_SENDSTATION_RADIO_PLEASE_INCLUDE));
	VERIFY(m_lblDoNotInclude.LoadString(IDS_SENDSTATION_RADIO_DO_NOT_INCLUDE));
	VERIFY(m_lblBtnCancel.LoadString(IDS_BTN_CANCEL));
	VERIFY(m_lblBtnNext.LoadString(IDS_BTN_NEXT));

	m_radio = Options::getSendStationId () ? 0 : 1;

	UpdateData (false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogStationId::OnButtonProceed() 
{
	UpdateData (true);

	Options::setSendStationId (m_radio == 0);

	// TODO: Add your control notification handler code here
	CDialog::OnOK ();	
}

// ======================================================================

