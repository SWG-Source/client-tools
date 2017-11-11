// ======================================================================
//
// DialogExtra.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "DialogExtra.h"

#include "Configuration.h"
#include "DialogSelectNpc.h"
#include "Quest.h"
#include "RecentDirectory.h"
#include "sharedUtility/FileName.h"

// ======================================================================

DialogExtra::DialogExtra(Quest * const quest, CWnd* pParent /*=NULL*/)
	: CDialog(DialogExtra::IDD, pParent),
	m_quest (quest)
{
	//{{AFX_DATA_INIT(DialogExtra)
	m_encounterDataTableName = _T("");
	m_extraSpawnObjectTemplate = _T("");
	m_spawnDelay = 0;
	m_spawnFrequency = 0;
	m_spawnLimit = 0;
	m_spawnRadius = 0;
	m_spawnScript = _T("");
	m_extraNpc1Entry = _T("");
	m_extraNpc2Entry = _T("");
	m_extraNpc3Entry = _T("");
	m_extraNpc4Entry = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogExtra::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogExtra)
	DDX_Control(pDX, IDC_COMBO_SPAWNACTION4, m_comboSpawnAction4);
	DDX_Control(pDX, IDC_COMBO_SPAWNACTION3, m_comboSpawnAction3);
	DDX_Control(pDX, IDC_COMBO_SPAWNACTION2, m_comboSpawnAction2);
	DDX_Control(pDX, IDC_COMBO_SPAWNACTION1, m_comboSpawnAction1);
	DDX_Control(pDX, IDC_COMBO_NPC4DISPOSITION, m_comboNpc4Disposition);
	DDX_Control(pDX, IDC_COMBO_NPC3DISPOSITION, m_comboNpc3Disposition);
	DDX_Control(pDX, IDC_COMBO_NPC1DISPOSITION, m_comboNpc1Disposition);
	DDX_Control(pDX, IDC_COMBO_NPC2DISPOSITION, m_comboNpc2Disposition);
	DDX_Text(pDX, IDC_EDIT_ENCOUNTERDATATABLENAME, m_encounterDataTableName);
	DDX_Text(pDX, IDC_EDIT_EXTRASPAWNOBJECTTEMPLATE, m_extraSpawnObjectTemplate);
	DDX_Text(pDX, IDC_EDIT_SPAWNDELAY, m_spawnDelay);
	DDX_Text(pDX, IDC_EDIT_SPAWNFREQUENCY, m_spawnFrequency);
	DDX_Text(pDX, IDC_EDIT_SPAWNLIMIT, m_spawnLimit);
	DDX_Text(pDX, IDC_EDIT_SPAWNRADIUS, m_spawnRadius);
	DDX_Text(pDX, IDC_EDIT_SPAWNSCRIPT, m_spawnScript);
	DDX_Text(pDX, IDC_EDIT_EXTRANPC1TEMPLATE, m_extraNpc1Entry);
	DDX_Text(pDX, IDC_EDIT_EXTRANPC2TEMPLATE, m_extraNpc2Entry);
	DDX_Text(pDX, IDC_EDIT_EXTRANPC3TEMPLATE, m_extraNpc3Entry);
	DDX_Text(pDX, IDC_EDIT_EXTRANPC4TEMPLATE, m_extraNpc4Entry);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogExtra, CDialog)
	//{{AFX_MSG_MAP(DialogExtra)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEENCOUNTERDATATABLE, OnButtonBrowseencounterdatatable)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEEXTRASPAWNTEMPLATE, OnButtonBrowseextraspawntemplate)
	ON_BN_CLICKED(IDC_BUTTON_BROWSENPC1TEMPLATE, OnButtonBrowsenpc1template)
	ON_BN_CLICKED(IDC_BUTTON_BROWSENPC2TEMPLATE, OnButtonBrowsenpc2template)
	ON_BN_CLICKED(IDC_BUTTON_BROWSENPC3TEMPLATE, OnButtonBrowsenpc3template)
	ON_BN_CLICKED(IDC_BUTTON_BROWSENPC4TEMPLATE, OnButtonBrowsenpc4template)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogExtra::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	Configuration::populateActions (m_comboSpawnAction4);
	Configuration::populateActions (m_comboSpawnAction3);
	Configuration::populateActions (m_comboSpawnAction2);
	Configuration::populateActions (m_comboSpawnAction1);
	Configuration::populateDispositions (m_comboNpc4Disposition);
	Configuration::populateDispositions (m_comboNpc3Disposition);
	Configuration::populateDispositions (m_comboNpc1Disposition);
	Configuration::populateDispositions (m_comboNpc2Disposition);

	m_extraNpc1Entry = m_quest->getExtraNpc1Entry ().c_str ();
	m_comboNpc1Disposition.SelectString (0, m_quest->getExtraNpc1Disposition ().c_str ());
	m_extraNpc2Entry = m_quest->getExtraNpc2Entry ().c_str ();
	m_comboNpc2Disposition.SelectString (0, m_quest->getExtraNpc2Disposition ().c_str ());
	m_extraNpc3Entry = m_quest->getExtraNpc3Entry ().c_str ();
	m_comboNpc3Disposition.SelectString (0, m_quest->getExtraNpc3Disposition ().c_str ());
	m_extraNpc4Entry = m_quest->getExtraNpc4Entry ().c_str ();
	m_comboNpc4Disposition.SelectString (0, m_quest->getExtraNpc4Disposition ().c_str ());
	m_extraSpawnObjectTemplate = m_quest->getExtraSpawnObjectTemplate ().c_str ();
	m_spawnDelay = m_quest->getExtraSpawnDelay ();
	m_spawnFrequency = m_quest->getExtraSpawnFrequency ();
	m_spawnLimit = m_quest->getExtraSpawnLimit ();
	m_spawnRadius = m_quest->getExtraSpawnRadius ();
	m_spawnScript = m_quest->getExtraSpawnScript ().c_str ();
	m_comboSpawnAction1.SelectString (0, m_quest->getExtraSpawnAction1 ().c_str ());
	m_comboSpawnAction2.SelectString (0, m_quest->getExtraSpawnAction2 ().c_str ());
	m_comboSpawnAction3.SelectString (0, m_quest->getExtraSpawnAction3 ().c_str ());
	m_comboSpawnAction4.SelectString (0, m_quest->getExtraSpawnAction4 ().c_str ());
	m_encounterDataTableName = m_quest->getExtraSpawnEncounterDataTable ().c_str ();

	UpdateData (false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogExtra::OnButtonBrowseencounterdatatable() 
{
	UpdateData (true);

	CFileDialog dlg (true, "*.iff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("EncounterDataTable");
	dlg.m_ofn.lpstrTitle      = "Select DataTable";
    
	if (dlg.DoModal () == IDOK)
	{
		CString pathName = dlg.GetPathName ();
		pathName.Replace ('\\', '/');
		int index = pathName.Find ("sys.server/compiled/game/datatables");
		if (index == -1)
		{
			CString message;
			message.Format ("%s\r\nis not a valid server datatable", pathName);
			MessageBox (message);
		}
		else
		{
			IGNORE_RETURN (RecentDirectory::update ("EncounterDataTable", dlg.GetPathName ()));

			CString pathName = dlg.GetPathName ();
			FileName fileName (pathName);
			fileName.stripPathAndExt ();

			m_encounterDataTableName = fileName;
		}
		
		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void DialogExtra::OnButtonBrowseextraspawntemplate() 
{
	UpdateData (true);

	CFileDialog dlg (true, "*.iff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ExtraSpawnObject");
	dlg.m_ofn.lpstrTitle      = "Select Server Object Template";
    
	if (dlg.DoModal () == IDOK)
	{
		CString pathName = dlg.GetPathName ();
		pathName.Replace ('\\', '/');
		int index = pathName.Find ("sys.server/compiled/game/object");
		if (index == -1)
		{
			CString message;
			message.Format ("%s\r\nis not a valid server object template", pathName);
			MessageBox (message);
		}
		else
		{
			IGNORE_RETURN (RecentDirectory::update ("ExtraSpawnObject", dlg.GetPathName ()));

			index = pathName.Find ("object");

			m_extraSpawnObjectTemplate = pathName.Right (pathName.GetLength () - index);
			m_extraSpawnObjectTemplate.Replace (".tpf", ".iff");
		}
		
		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void DialogExtra::OnButtonBrowsenpc1template() 
{
	UpdateData (true);

	DialogSelectNpc dlg (DialogSelectNpc::NT_target);
	if (dlg.DoModal () == IDOK)
	{
		m_extraNpc1Entry = dlg.m_selection;

		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void DialogExtra::OnButtonBrowsenpc2template() 
{
	UpdateData (true);

	DialogSelectNpc dlg (DialogSelectNpc::NT_target);
	if (dlg.DoModal () == IDOK)
	{
		m_extraNpc2Entry = dlg.m_selection;

		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void DialogExtra::OnButtonBrowsenpc3template() 
{
	UpdateData (true);

	DialogSelectNpc dlg (DialogSelectNpc::NT_target);
	if (dlg.DoModal () == IDOK)
	{
		m_extraNpc3Entry = dlg.m_selection;

		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void DialogExtra::OnButtonBrowsenpc4template() 
{
	UpdateData (true);

	DialogSelectNpc dlg (DialogSelectNpc::NT_target);
	if (dlg.DoModal () == IDOK)
	{
		m_extraNpc4Entry = dlg.m_selection;

		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void DialogExtra::OnOK() 
{
	UpdateData (true);

	// TODO: Add extra validation here
	CString selection;
	m_quest->setExtraNpc1Entry (std::string (m_extraNpc1Entry));
	m_comboNpc1Disposition.GetLBText (m_comboNpc1Disposition.GetCurSel (), selection);
	m_quest->setExtraNpc1Disposition (std::string (selection));
	m_quest->setExtraNpc2Entry (std::string (m_extraNpc2Entry));
	m_comboNpc2Disposition.GetLBText (m_comboNpc2Disposition.GetCurSel (), selection);
	m_quest->setExtraNpc2Disposition (std::string (selection));
	m_quest->setExtraNpc3Entry (std::string (m_extraNpc3Entry));
	m_comboNpc3Disposition.GetLBText (m_comboNpc3Disposition.GetCurSel (), selection);
	m_quest->setExtraNpc3Disposition (std::string (selection));
	m_quest->setExtraNpc4Entry (std::string (m_extraNpc4Entry));
	m_comboNpc4Disposition.GetLBText (m_comboNpc4Disposition.GetCurSel (), selection);
	m_quest->setExtraNpc4Disposition (std::string (selection));
	m_quest->setExtraSpawnObjectTemplate (std::string (m_extraSpawnObjectTemplate));
	m_quest->setExtraSpawnDelay (m_spawnDelay);
	m_quest->setExtraSpawnFrequency (m_spawnFrequency);
	m_quest->setExtraSpawnLimit (m_spawnLimit);
	m_quest->setExtraSpawnRadius (m_spawnRadius);
	m_quest->setExtraSpawnScript (std::string (m_spawnScript));
	m_comboSpawnAction1.GetLBText (m_comboSpawnAction1.GetCurSel (), selection);
	m_quest->setExtraSpawnAction1 (std::string (selection));
	m_comboSpawnAction2.GetLBText (m_comboSpawnAction2.GetCurSel (), selection);
	m_quest->setExtraSpawnAction2 (std::string (selection));
	m_comboSpawnAction3.GetLBText (m_comboSpawnAction3.GetCurSel (), selection);
	m_quest->setExtraSpawnAction3 (std::string (selection));
	m_comboSpawnAction4.GetLBText (m_comboSpawnAction4.GetCurSel (), selection);
	m_quest->setExtraSpawnAction4 (std::string (selection));
	m_quest->setExtraSpawnEncounterDataTable (std::string (m_encounterDataTableName));
	
	CDialog::OnOK();
}

// ======================================================================

