// ======================================================================
//
// DialogRewards.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "DialogRewards.h"

#include "Configuration.h"
#include "Quest.h"
#include "RecentDirectory.h"

// ======================================================================

DialogRewards::DialogRewards(Quest * const quest, CWnd* pParent /*=NULL*/)
	: CDialog(DialogRewards::IDD, pParent),
	m_quest (quest)
{
	//{{AFX_DATA_INIT(DialogRewards)
	m_credits = 0;
	m_reward1ObjectTemplate = _T("");
	m_reward2ObjectTemplate = _T("");
	m_reward3ObjectTemplate = _T("");
	m_reward4ObjectTemplate = _T("");
	m_reward1Objvar = _T("");
	m_reward2Objvar = _T("");
	m_reward3Objvar = _T("");
	m_reward4Objvar = _T("");
	m_reward4ObjvarValue = _T("");
	m_reward3ObjvarValue = _T("");
	m_reward2ObjvarValue = _T("");
	m_reward1ObjvarValue = _T("");
	m_reward1FactionValue = 0;
	m_reward2FactionValue = 0;
	m_reward3FactionValue = 0;
	m_reward4FactionValue = 0;
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogRewards::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogRewards)
	DDX_Control(pDX, IDC_COMBO_REWARD4FACTION, m_comboFactionReward4);
	DDX_Control(pDX, IDC_COMBO_REWARD3FACTION, m_comboFactionReward3);
	DDX_Control(pDX, IDC_COMBO_REWARD2FACTION, m_comboFactionReward2);
	DDX_Control(pDX, IDC_COMBO_REWARD1FACTION, m_comboFactionReward1);
	DDX_Text(pDX, IDC_EDIT_CREDITS, m_credits);
	DDX_Text(pDX, IDC_EDIT_REWARD1OBJECT, m_reward1ObjectTemplate);
	DDX_Text(pDX, IDC_EDIT_REWARD2OBJECT, m_reward2ObjectTemplate);
	DDX_Text(pDX, IDC_EDIT_REWARD3OBJECT, m_reward3ObjectTemplate);
	DDX_Text(pDX, IDC_EDIT_REWARD4OBJECT, m_reward4ObjectTemplate);
	DDX_Text(pDX, IDC_EDIT_REWARD1OBJVAR, m_reward1Objvar);
	DDX_Text(pDX, IDC_EDIT_REWARD2OBJVAR, m_reward2Objvar);
	DDX_Text(pDX, IDC_EDIT_REWARD3OBJVAR, m_reward3Objvar);
	DDX_Text(pDX, IDC_EDIT_REWARD4OBJVAR, m_reward4Objvar);
	DDX_Text(pDX, IDC_EDIT_REWARD4VALUE, m_reward4ObjvarValue);
	DDX_Text(pDX, IDC_EDIT_REWARD3VALUE, m_reward3ObjvarValue);
	DDX_Text(pDX, IDC_EDIT_REWARD2VALUE, m_reward2ObjvarValue);
	DDX_Text(pDX, IDC_EDIT_REWARD1VALUE, m_reward1ObjvarValue);
	DDX_Text(pDX, IDC_EDIT_REWARD1FACTIONVALUE, m_reward1FactionValue);
	DDX_Text(pDX, IDC_EDIT_REWARD2FACTIONVALUE, m_reward2FactionValue);
	DDX_Text(pDX, IDC_EDIT_REWARD3FACTIONVALUE, m_reward3FactionValue);
	DDX_Text(pDX, IDC_EDIT_REWARD4FACTIONVALUE, m_reward4FactionValue);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogRewards, CDialog)
	//{{AFX_MSG_MAP(DialogRewards)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEOBJECT1, OnButtonBrowseobject1)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEOBJECT2, OnButtonBrowseobject2)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEOBJECT3, OnButtonBrowseobject3)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEOBJECT4, OnButtonBrowseobject4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogRewards::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	Configuration::populateFactionRewards (m_comboFactionReward4);
	Configuration::populateFactionRewards (m_comboFactionReward3);
	Configuration::populateFactionRewards (m_comboFactionReward2);
	Configuration::populateFactionRewards (m_comboFactionReward1);

	m_credits = m_quest->getCredits ();
	m_reward1ObjectTemplate = m_quest->getReward1ObjectTemplate ().c_str ();
	m_reward1Objvar = m_quest->getReward1Objvar ().c_str ();
	m_reward1ObjvarValue = m_quest->getReward1ObjvarValue ().c_str ();
	m_comboFactionReward1.SelectString (0, m_quest->getReward1Faction ().c_str ());
	m_reward1FactionValue = m_quest->getReward1FactionValue ();
	m_reward2ObjectTemplate = m_quest->getReward2ObjectTemplate ().c_str ();
	m_reward2Objvar = m_quest->getReward2Objvar ().c_str ();
	m_reward2ObjvarValue = m_quest->getReward2ObjvarValue ().c_str ();
	m_comboFactionReward2.SelectString (0, m_quest->getReward2Faction ().c_str ());
	m_reward2FactionValue = m_quest->getReward2FactionValue ();
	m_reward3ObjectTemplate = m_quest->getReward3ObjectTemplate ().c_str ();
	m_reward3Objvar = m_quest->getReward3Objvar ().c_str ();
	m_reward3ObjvarValue = m_quest->getReward3ObjvarValue ().c_str ();
	m_comboFactionReward3.SelectString (0, m_quest->getReward3Faction ().c_str ());
	m_reward3FactionValue = m_quest->getReward3FactionValue ();
	m_reward4ObjectTemplate = m_quest->getReward4ObjectTemplate ().c_str ();
	m_reward4Objvar = m_quest->getReward4Objvar ().c_str ();
	m_reward4ObjvarValue = m_quest->getReward4ObjvarValue ().c_str ();
	m_comboFactionReward4.SelectString (0, m_quest->getReward4Faction ().c_str ());
	m_reward4FactionValue = m_quest->getReward4FactionValue ();
	
	UpdateData (false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogRewards::OnOK() 
{
	UpdateData (true);

	CString selection;

	m_quest->setCredits (m_credits);
	m_quest->setReward1ObjectTemplate (std::string (m_reward1ObjectTemplate));
	m_quest->setReward1Objvar (std::string (m_reward1Objvar));
	m_quest->setReward1ObjvarValue (std::string (m_reward1ObjvarValue));
	m_comboFactionReward1.GetLBText (m_comboFactionReward1.GetCurSel (), selection);
	m_quest->setReward1Faction (std::string (selection));
	m_quest->setReward1FactionValue (m_reward1FactionValue);
	m_quest->setReward2ObjectTemplate (std::string (m_reward2ObjectTemplate));
	m_quest->setReward2Objvar (std::string (m_reward2Objvar));
	m_quest->setReward2ObjvarValue (std::string (m_reward2ObjvarValue));
	m_comboFactionReward2.GetLBText (m_comboFactionReward2.GetCurSel (), selection);
	m_quest->setReward2Faction (std::string (selection));
	m_quest->setReward2FactionValue (m_reward2FactionValue);
	m_quest->setReward3ObjectTemplate (std::string (m_reward3ObjectTemplate));
	m_quest->setReward3Objvar (std::string (m_reward3Objvar));
	m_quest->setReward3ObjvarValue (std::string (m_reward3ObjvarValue));
	m_comboFactionReward3.GetLBText (m_comboFactionReward3.GetCurSel (), selection);
	m_quest->setReward3Faction (std::string (selection));
	m_quest->setReward3FactionValue (m_reward3FactionValue);
	m_quest->setReward4ObjectTemplate (std::string (m_reward4ObjectTemplate));
	m_quest->setReward4Objvar (std::string (m_reward4Objvar));
	m_quest->setReward4ObjvarValue (std::string (m_reward4ObjvarValue));
	m_comboFactionReward4.GetLBText (m_comboFactionReward4.GetCurSel (), selection);
	m_quest->setReward4Faction (std::string (selection));
	m_quest->setReward4FactionValue (m_reward4FactionValue);

	CDialog::OnOK();
}

// ----------------------------------------------------------------------

void DialogRewards::OnButtonBrowseobject1() 
{
	UpdateData (true);

	CFileDialog dlg (true, "*.iff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ObjectReward1");
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
			IGNORE_RETURN (RecentDirectory::update ("ObjectReward1", dlg.GetPathName ()));

			index = pathName.Find ("object");

			m_reward1ObjectTemplate = pathName.Right (pathName.GetLength () - index);
			m_reward1ObjectTemplate.Replace (".tpf", ".iff");
		}
		
		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void DialogRewards::OnButtonBrowseobject2() 
{
	UpdateData (true);

	CFileDialog dlg (true, "*.iff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ObjectReward2");
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
			IGNORE_RETURN (RecentDirectory::update ("ObjectReward2", dlg.GetPathName ()));

			index = pathName.Find ("object");

			m_reward2ObjectTemplate = pathName.Right (pathName.GetLength () - index);
			m_reward2ObjectTemplate.Replace (".tpf", ".iff");
		}
		
		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void DialogRewards::OnButtonBrowseobject3() 
{
	UpdateData (true);

	CFileDialog dlg (true, "*.iff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ObjectReward3");
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
			IGNORE_RETURN (RecentDirectory::update ("ObjectReward3", dlg.GetPathName ()));

			index = pathName.Find ("object");

			m_reward3ObjectTemplate = pathName.Right (pathName.GetLength () - index);
			m_reward3ObjectTemplate.Replace (".tpf", ".iff");
		}
		
		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void DialogRewards::OnButtonBrowseobject4() 
{
	UpdateData (true);

	CFileDialog dlg (true, "*.iff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ObjectReward4");
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
			IGNORE_RETURN (RecentDirectory::update ("ObjectReward4", dlg.GetPathName ()));

			index = pathName.Find ("object");

			m_reward4ObjectTemplate = pathName.Right (pathName.GetLength () - index);
			m_reward4ObjectTemplate.Replace (".tpf", ".iff");
		}
		
		UpdateData (false);
	}
}

// ======================================================================
