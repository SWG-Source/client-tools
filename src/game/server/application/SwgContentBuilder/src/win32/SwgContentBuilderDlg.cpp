// ======================================================================
//
// SwgContentBuilderDlg.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgContentBuilder.h"
#include "SwgContentBuilderDlg.h"

#include "Configuration.h"
#include "DialogExtra.h"
#include "DialogRewards.h"
#include "DialogMissionNpcConversation.h"
#include "DialogQuestNpcConversation.h"
#include "DialogSelectNpc.h"
#include "fileInterface/StdioFile.h"
#include "LocalizedStringTableReaderWriter.h"
#include "LocalizedString.h"
#include "Quest.h"
#include "RecentDirectory.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedUtility/FileName.h"
#include "SwgContentBuilder.h"
#include "UnicodeUtils.h"

#include <algorithm>

// ======================================================================

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ======================================================================

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

SwgContentBuilderDlg::SwgContentBuilderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SwgContentBuilderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(SwgContentBuilderDlg)
	m_questId = _T("");
	m_locationX = 0.0f;
	m_locationY = 0.0f;
	m_locationZ = 0.0f;
	m_overallObjvar = _T("");
	m_tempObjvar = _T("");
	m_retrieveObjectTemplate = _T("");
	m_deliverObjectTemplate = _T("");
	m_playerScript = _T("");
	m_questLeg = 0;
	m_questGiverEntry = _T("");
	m_questNpcEntry = _T("");
	m_questNpcName = _T("");
	m_questNpcScript = _T("");
	m_spawnerObjectTemplateName = _T("");
	m_dataTableName = _T("");
	m_conversationName = _T("");
	m_gatingFactionAmount = 0;
	m_gatingObjectTemplate = _T("");
	m_gatingObjvar = _T("");
	m_questGiverName = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SwgContentBuilderDlg)
	DDX_Control(pDX, IDC_EDIT_QUESTGIVERNAME, m_editCtrlQuestGiverName);
	DDX_Control(pDX, IDC_BUTTON_BROWSEOBJECTQUESTGIVER, m_buttonBrowseQuestGiver);
	DDX_Control(pDX, IDC_COMBO_GATINGFACTION, m_comboGatingFaction);
	DDX_Control(pDX, IDC_COMBO_QUESTTYPE, m_comboQuestType);
	DDX_Control(pDX, IDC_COMBO_QUESTGIVERCONVO, m_comboQuestGiverConversation);
	DDX_Control(pDX, IDC_COMBO_QUESTNPCCONVO, m_comboQuestNpcConversation);
	DDX_Control(pDX, IDC_COMBO_LOCATIONPLANET, m_comboLocationPlanet);
	DDX_Text(pDX, IDC_STATIC_QUESTID, m_questId);
	DDX_Text(pDX, IDC_EDIT_LOCATIONX, m_locationX);
	DDV_MinMaxFloat(pDX, m_locationX, -8192.f, 8192.f);
	DDX_Text(pDX, IDC_EDIT_LOCATIONY, m_locationY);
	DDV_MinMaxFloat(pDX, m_locationY, -8192.f, 8192.f);
	DDX_Text(pDX, IDC_EDIT_LOCATIONZ, m_locationZ);
	DDV_MinMaxFloat(pDX, m_locationZ, -8192.f, 8192.f);
	DDX_Text(pDX, IDC_EDIT_OVERALLOBJVAR, m_overallObjvar);
	DDX_Text(pDX, IDC_EDIT_TEMPOBJVAR, m_tempObjvar);
	DDX_Text(pDX, IDC_EDIT_RETRIEVEOBJECTTEMPLATE, m_retrieveObjectTemplate);
	DDX_Text(pDX, IDC_EDIT_DELIVEROBJECTTEMPLATE, m_deliverObjectTemplate);
	DDX_Text(pDX, IDC_EDIT_PLAYERSCRIPT, m_playerScript);
	DDX_Text(pDX, IDC_EDIT_QUESTLEG, m_questLeg);
	DDX_Text(pDX, IDC_EDIT_QUESTGIVERTEMPLATE, m_questGiverEntry);
	DDX_Text(pDX, IDC_EDIT_QUESTNPCTEMPLATE, m_questNpcEntry);
	DDX_Text(pDX, IDC_EDIT_QUESTNPCNAME, m_questNpcName);
	DDX_Text(pDX, IDC_EDIT_QUESTNPCSCRIPT, m_questNpcScript);
	DDX_Text(pDX, IDC_EDIT_SPAWNERNAME, m_spawnerObjectTemplateName);
	DDX_Text(pDX, IDC_EDIT_DATATABLENAME, m_dataTableName);
	DDX_Text(pDX, IDC_EDIT_CONVERSATIONNAME, m_conversationName);
	DDX_Text(pDX, IDC_EDIT_GATINGFACTIONAMOUNT, m_gatingFactionAmount);
	DDX_Text(pDX, IDC_EDIT_GATINGOBJECTTEMPLATE, m_gatingObjectTemplate);
	DDX_Text(pDX, IDC_EDIT_GATINGOBJVAR, m_gatingObjvar);
	DDX_Text(pDX, IDC_EDIT_QUESTGIVERNAME, m_questGiverName);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(SwgContentBuilderDlg, CDialog)
	//{{AFX_MSG_MAP(SwgContentBuilderDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BROWSEOBJECTDELIVER, OnButtonBrowseobjectdeliver)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEOBJECTQUESTGIVER, OnButtonBrowseobjectquestgiver)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEOBJECTQUESTNPC, OnButtonBrowseobjectquestnpc)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEOBJECTRETRIEVE, OnButtonBrowseobjectretrieve)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_EDITCONVOQUESTGIVER, OnButtonEditconvoquestgiver)
	ON_BN_CLICKED(IDC_BUTTON_EDITCONVOQUESTNPC, OnButtonEditconvoquestnpc)
	ON_BN_CLICKED(IDC_BUTTON_EDITEXTRANPCS, OnButtonEditextranpcs)
	ON_BN_CLICKED(IDC_BUTTON_EDITREWARDS, OnButtonEditrewards)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, OnButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_PREVIOUS, OnButtonPrevious)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_INSERT, OnButtonInsert)
	ON_BN_CLICKED(IDC_BUTTON_APPEND, OnButtonAppend)
	ON_BN_CLICKED(IDC_BUTTON_SAVEAS, OnButtonSaveas)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEGATINGOBJECT, OnButtonBrowsegatingobject)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL SwgContentBuilderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	Configuration::populateQuestTypes (m_comboQuestType);
	Configuration::populateMissionConversationTypes (m_comboQuestGiverConversation);
	Configuration::populateNpcConversationTypes (m_comboQuestNpcConversation);
	Configuration::populatePlanets (m_comboLocationPlanet);
	Configuration::populateFactionRewards (m_comboGatingFaction);

	m_currentQuest = 0;
	m_questList.push_back (new Quest);

	m_questGiverEntry = "none";
	m_questGiverName = "";

	updateCurrentQuest (false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ----------------------------------------------------------------------

HCURSOR SwgContentBuilderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonBrowseobjectdeliver() 
{
	UpdateData (true);

	CFileDialog dlg (true, "", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ObjectDeliver");
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
			IGNORE_RETURN (RecentDirectory::update ("ObjectDeliver", dlg.GetPathName ()));

			index = pathName.Find ("object");

			m_deliverObjectTemplate = pathName.Right (pathName.GetLength () - index);
			m_deliverObjectTemplate.Replace (".tpf", ".iff");
		}
		
		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonBrowseobjectquestgiver() 
{
	UpdateData (true);

	DialogSelectNpc dlg (DialogSelectNpc::NT_giver);
	if (dlg.DoModal () == IDOK)
	{
		m_questGiverEntry = dlg.m_selection;

		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonBrowseobjectquestnpc() 
{
	UpdateData (true);

	DialogSelectNpc dlg (DialogSelectNpc::NT_target);
	if (dlg.DoModal () == IDOK)
	{
		m_questNpcEntry = dlg.m_selection;

		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonBrowseobjectretrieve() 
{
	UpdateData (true);

	CFileDialog dlg (true, "", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ObjectRetrieve");
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
			IGNORE_RETURN (RecentDirectory::update ("ObjectRetrieve", dlg.GetPathName ()));

			index = pathName.Find ("object");

			m_retrieveObjectTemplate = pathName.Right (pathName.GetLength () - index);
			m_retrieveObjectTemplate.Replace (".tpf", ".iff");
		}
		
		UpdateData (false);
	}
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonDelete() 
{
	if (m_questList.size () == 1)
	{
		MessageBox ("You can't delete the only quest");
	}
	else
	{
		CString message;
		message.Format ("Are you sure you wish to delete quest %i?", m_currentQuest + 1);
		if (MessageBox (message, "Delete Quest", MB_YESNO) == IDYES)
		{
			delete m_questList [m_currentQuest];
			m_questList.erase (m_questList.begin () + m_currentQuest);
			if (m_currentQuest == static_cast<int> (m_questList.size ()))
				--m_currentQuest;

			updateCurrentQuest (false);
		}
	}
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonEditconvoquestgiver() 
{
	DialogMissionNpcConversation dlg (m_conversation, static_cast<DialogMissionNpcConversation::ConversationType> (m_comboQuestGiverConversation.GetCurSel ()), m_currentQuest + 1);
	dlg.DoModal ();
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonEditconvoquestnpc() 
{
	CString selection;
	m_comboQuestType.GetLBText (m_comboQuestType.GetCurSel (), selection);
	DialogQuestNpcConversation dlg (m_conversation, static_cast<DialogQuestNpcConversation::ConversationType> (m_comboQuestNpcConversation.GetCurSel ()), selection, m_currentQuest + 1);
	dlg.DoModal ();
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonEditextranpcs() 
{
	DialogExtra dlg (m_questList [m_currentQuest]);
	dlg.DoModal ();
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonEditrewards() 
{
	DialogRewards dlg (m_questList [m_currentQuest]);
	dlg.DoModal ();
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonLoad() 
{
	{
		CFileDialog dlg (true, "", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
		dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ObjectSpawner");
		dlg.m_ofn.lpstrTitle      = "Select Spawner Server Object Template";

		if (dlg.DoModal () == IDOK)
		{
			CString pathName = dlg.GetPathName ();
			pathName.Replace ('\\', '/');
			int index0 = pathName.Find ("dsrc/");
			int index1 = pathName.Find ("sys.server/compiled/game/object/tangible/spawning/static_npc");
			int index2 = pathName.Find (".tpf");
			if (index0 == -1 || index1 == -1 || index2 == -1)
			{
				CString message;
				message.Format ("%s\r\nis not a valid server object template .tpf or is not in the correct location ($/.../sys.server/compiled/game/object/tangible/spawning/static_npc)", pathName);
				MessageBox (message);

				return;
			}
			else
			{
				IGNORE_RETURN (RecentDirectory::update ("ObjectSpawner", dlg.GetPathName ()));

				std::for_each (m_questList.begin (), m_questList.end (), PointerDeleter ());
				m_questList.clear ();

				m_conversation.clear ();
				m_questGiverEntry = "none";
				m_questGiverName = "";

				m_currentQuest = 0;

				m_baseName = "";
				m_spawnerObjectTemplateName = "";
				m_spawnerObjectTemplateFullName = "";
				m_dataTableName = "";
				m_dataTableFullName = "";
				m_conversationName = "";
				m_conversationFullName = "";

				CString name = pathName;
				name.MakeLower ();
				createNames (name);
			}
		}
		else
			return;
	}

	//-- load spawner
	{
		CStdioFile infile;
		if (infile.Open (m_spawnerObjectTemplateFullName, CFile::modeRead | CFile::typeText))
		{
			//-- read each line...
			CString line;
			while (infile.ReadString (line))
			{
				int index = line.Find ("objvars");
				if (index != -1)
				{
					int indexSpawns = line.Find ("spawns");
					if (indexSpawns != -1)
					{
						int indexEquals = line.Find ("=", indexSpawns);
						int indexComma  = line.Find (",", indexEquals);

						if (indexEquals != -1 && indexComma != -1)
						{
							m_questGiverEntry = line.Mid (indexEquals + 1, indexComma - indexEquals - 1);
							m_questGiverEntry.Remove ('"');
							m_questGiverEntry.TrimLeft ();
							m_questGiverEntry.TrimRight ();
							OutputDebugString (m_questGiverEntry);

							indexEquals = line.Find ("=", indexComma);
							indexComma  = line.Find (",", indexEquals);

							if (indexEquals != -1 && indexComma != -1)
							{
								m_questGiverName = line.Mid (indexEquals + 1, indexComma - indexEquals - 1);
								m_questGiverName.Remove ('"');
								m_questGiverName.TrimLeft ();
								m_questGiverName.TrimRight ();
							}
						}
					}
				}
			}
		}
		else
		{
			CString const message = m_spawnerObjectTemplateFullName + " could not be read.  Does the file exist?  Is the file currently open in another application?";
			MessageBox (message);
		}
	}

	//-- load datatable
	{
		int index = 0;

		CStdioFile infile;
		if (infile.Open (m_dataTableFullName, CFile::modeRead | CFile::typeText))
		{
			//-- read each line...
			CString line;
			for (; infile.ReadString (line); ++index)
			{
				if (index < 3)
					continue;

				//-- see if the line is empty
				if (line.GetLength () == 0)
					break;

				Quest * const quest = new Quest;
				quest->read (std::string (line));
				m_questList.push_back (quest);
			}
		}
		else
		{
			CString const message = m_dataTableFullName + " could not be read.  Does the file exist?  Is the file currently open in another application?";
			MessageBox (message);

			m_questList.push_back (new Quest);
		}
	}

	//-- load stringtable
	{
		StdioFileFactory fileFactory;
		LocalizedStringTableRW * stringTable = LocalizedStringTableRW::loadRW (fileFactory, std::string (m_conversationFullName));
		if (stringTable)
		{
			const LocalizedStringTable::Map_t & stringMap = stringTable->getMap ();
			LocalizedStringTable::Map_t::const_iterator end = stringMap.end ();
			for (LocalizedStringTable::Map_t::const_iterator iter = stringMap.begin (); iter != end; ++iter)
			{	
				LocalizedString const * const value = iter->second;
				unsigned long const stringId = value->getId ();
				Unicode::NarrowString const * const key = stringTable->getNameById (stringId);

				m_conversation.setString (*key, Unicode::wideToNarrow (value->getString ()));
			}

			delete stringTable;
		}
		else
		{
			CString const message = m_conversationFullName + " could not be read.  Does the file exist?  Is the file currently open in another application?";
			MessageBox (message);
		}
	}

	updateCurrentQuest (false);
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonNew() 
{
	if (MessageBox ("Are you sure you wish to create a new static quest?", "New Static Quest", MB_YESNO) == IDYES)
	{
		std::for_each (m_questList.begin (), m_questList.end (), PointerDeleter ());
		m_questList.clear ();

		m_currentQuest = 0;
		m_questList.push_back (new Quest);

		m_conversation.clear ();
		m_questGiverEntry = "none";
		m_questGiverName = "";

		m_baseName = "";
		m_spawnerObjectTemplateName = "";
		m_spawnerObjectTemplateFullName = "";
		m_dataTableName = "";
		m_dataTableFullName = "";
		m_conversationName = "";
		m_conversationFullName = "";

		updateCurrentQuest (false);
	}
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonNext() 
{
	updateCurrentQuest (true);

	// TODO: Add your control notification handler code here
	++m_currentQuest;
	if (m_currentQuest > static_cast<int> (m_questList.size ()) - 1)
		m_currentQuest = static_cast<int> (m_questList.size ()) - 1;
	
	updateCurrentQuest (false);
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonPrevious() 
{
	updateCurrentQuest (true);

	// TODO: Add your control notification handler code here
	--m_currentQuest;
	if (m_currentQuest < 0)
		m_currentQuest = 0;

	updateCurrentQuest (false);
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonSaveas() 
{
	updateCurrentQuest (true);

	CFileDialog dlg (false, "tpf", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ObjectSpawner");
	dlg.m_ofn.lpstrTitle      = "Select Spawner Server Object Template";

	if (dlg.DoModal () == IDOK)
	{
		CString pathName = dlg.GetPathName ();
		pathName.Replace ('\\', '/');
		int index0 = pathName.Find ("dsrc/");
		int index1 = pathName.Find ("sys.server/compiled/game/object/tangible/spawning/static_npc");
		int index2 = pathName.Find (".tpf");
		if (index0 == -1 || index1 == -1 || index2 == -1)
		{
			CString message;
			message.Format ("%s\r\nis not a valid server object template .tpf or is not in the correct location ($/.../sys.server/compiled/game/object/tangible/spawning/static_npc)", pathName);
			MessageBox (message);
		}
		else
		{
			IGNORE_RETURN (RecentDirectory::update ("ObjectSpawner", dlg.GetPathName ()));

			CString name = pathName;
			name.MakeLower ();
			createNames (name);
		}
		
		UpdateData (false);

		save (true);
	}
	else
		return;
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::createNames (CString const & pathName)
{
	int index1 = pathName.Find ("object");

	m_spawnerObjectTemplateName = pathName.Right (pathName.GetLength () - index1);
	m_spawnerObjectTemplateFullName = pathName;
	m_spawnerObjectTemplateFullName.Replace ('/', '\\');
	OutputDebugString (m_spawnerObjectTemplateFullName + "\r\n");

	FileName fileName (m_spawnerObjectTemplateName);
	fileName.stripPathAndExt ();

	m_baseName = fileName;

	m_dataTableName = CString ("datatables/spawning/static_npc/") + fileName + ".tab";

	m_dataTableFullName = m_spawnerObjectTemplateFullName.Left (index1);
	m_dataTableFullName += m_dataTableName;
	m_dataTableFullName.Replace ('/', '\\');
	OutputDebugString (m_dataTableFullName + "\r\n");

	m_conversationName = CString ("spawning/static_npc/") + fileName;
	int index2 = m_spawnerObjectTemplateFullName.Find ("compiled");
	m_conversationFullName = m_spawnerObjectTemplateFullName.Left (index2);
	m_conversationFullName.Replace ('\\', '/');
	m_conversationFullName.Replace ("dsrc/", "data/");
	m_conversationFullName.Replace ("sys.server/", "sys.shared/");
	m_conversationFullName += CString ("built/game/string/en/spawning/static_npc/") + fileName + ".stf";
	m_conversationFullName.Replace ('/', '\\');
	OutputDebugString (m_conversationFullName + "\r\n");
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonSave() 
{
	updateCurrentQuest (true);

	bool verify = true;

	if (m_spawnerObjectTemplateName.GetLength () == 0)
	{
		OnButtonSaveas ();

		verify = false;
	}

	save (verify);
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::save (bool const verify)
{
	if (m_spawnerObjectTemplateName.GetLength () == 0)
		return;

	//-- verify data
	if (verify)
	{
		std::string result;

		if (m_questGiverEntry.GetLength () == 0 || m_questGiverEntry == "none")
			result += "Quest giver template is empty\r\n";

		uint i;
		for (i = 0; i < m_questList.size (); ++i)
			m_questList [i]->verify (i + 1, result);

		if (!result.empty ())
			MessageBox (result.c_str (), "Warning!", MB_ICONWARNING);
	}

	//-- write spawner
	{
		CStdioFile outfile;
		if (outfile.Open (m_spawnerObjectTemplateFullName, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		{
			outfile.WriteString ("@base object/tangible/poi/base/poi_base.iff\n\n");
			outfile.WriteString ("@class tangible_object_template 0\n\n");
			outfile.WriteString ("@class object_template 0\n\n");
			outfile.WriteString ("\tvisibleFlags = [VF_gm]\n");
			outfile.WriteString ("\tmoveFlags = [MF_gm]\n");
			outfile.WriteString ("\tobjvars = [ \"spawns\" = \"");
			outfile.WriteString (m_questGiverEntry);
			outfile.WriteString ("\", \"npc_name\" = \"");
			outfile.WriteString (m_questGiverName);
			outfile.WriteString ("\", \"quest_script\" = \"npc.static_quest.quest_convo\", \"quest_table\" = \"");
			outfile.WriteString (m_baseName);
			outfile.WriteString ("\" ]\n");
			outfile.WriteString ("\tscripts = [ \"npc.celebrity.celebrity_spawner\" ]\n");
		}
		else
		{
			CString const message = m_spawnerObjectTemplateFullName + " could not be written.  Does the directory exist?  Is the file currently open in another application?";
			MessageBox (message);
		}
	}

	//-- write datatable
	{
		CStdioFile outfile;
		if (outfile.Open (m_dataTableFullName, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		{
			CString const header1 ("quest_type	convo	quest_giver_convo	quest_npc_convo	overall_objvar	temp_objvar	deliver_object	retrieve_object	player_script	npc_script	npc_type	npc_name	reward	reward_objvar	reward_objvar_value	reward2	reward2_objvar	reward2_objvar_value	reward3	reward3_objvar	reward3_objvar_value	reward4	reward4_objvar	reward4_objvar_value	faction reward	faction_reward_amount	faction_reward2	faction_reward2_amount	faction_reward3	faction_reward3_amount	faction_reward4	faction_reward4_amount	credits	quest_x_loc	quest_y_loc	quest_z_loc	quest_planet_loc	quest_leg	extra_npc	extra_npc_disposition	extra_npc2	extra_npc2_disposition	extra_npc3	extra_npc3_disposition	extra_npc4	extra_npc4_disposition	extra_spawn	spawn_delay	spawn_frequency	spawn_cap	spawn_radius	spawn_script	spawn_action1	spawn_action2	spawn_action3	spawn_action4	encounter_table	gating_objvar	gating_object	gating_faction	gating_faction_amt\n");
			outfile.WriteString (header1);
			CString const header2 ("s	s	s[normal]	s[normal]	s	s	s[none]	s[none]	s	s	s	s	s[none]	s	i[0]	s[none]	s	i[0]	s[none]	s	i[0]	s[none]	s	i[0]	s[none]	i[0]	s[none]	i[0]	s[none]	i[0]	s[none]	i[0]	i[0]	i[0]	i[0]	i[0]	s	i[0]	s	s	s	s	s	s	s	s	s[none]	i[0]	i[0]	i[0]	i[0]	s	s	s	s	s	s	s[none]	s[none]	s[none]	i[0]\n");
			outfile.WriteString (header2);
			CString const header3 ("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\n");
			outfile.WriteString (header3);

			QuestList::iterator end = m_questList.end ();
			for (QuestList::iterator iter = m_questList.begin (); iter != end; ++iter)
			{
				Quest * const quest = *iter;
				quest->setConversationStfName (std::string (m_conversationName));

				std::string output;
				quest->write (output);
				outfile.WriteString (output.c_str ());
			}
		}	
		else
		{
			CString const message = m_dataTableFullName + " could not be written.  Does the directory exist?  Is the file currently open in another application?";
			MessageBox (message);
		}
	}

	//-- write stringtable
	{
		LocalizedStringTableRW stringTable ("temp");

		Conversation::StringList stringList;
		m_conversation.getStringList (stringList);

		int id = 0;

		Conversation::StringList::const_iterator end = stringList.end ();
		for (Conversation::StringList::const_iterator iter = stringList.begin (); iter != end; ++iter)
		{
			std::string result;
			stringTable.addString (new LocalizedString (++id, 0, Unicode::narrowToWide (iter->second)), iter->first, result);
		}
		
		StdioFileFactory fileFactory;
		if (!stringTable.writeRW (fileFactory, std::string (m_conversationFullName)))
		{
			CString const message = m_conversationFullName + " could not be written.  Does the directory exist?  Is the file currently open in another application?";
			MessageBox (message);
		}
	}
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonInsert() 
{
	updateCurrentQuest (true);

	m_questList.push_back (0);
	int i;
	for (i = m_questList.size () - 1; i > m_currentQuest; --i)
		m_questList [i] = m_questList [i - 1];

	m_questList [m_currentQuest] = new Quest;

	updateCurrentQuest (false);
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::updateCurrentQuest (bool updateData)
{
	m_buttonBrowseQuestGiver.EnableWindow (m_currentQuest == 0);
	m_editCtrlQuestGiverName.EnableWindow (m_currentQuest == 0);

	Quest * const quest = m_questList [m_currentQuest];

	if (updateData)
	{
		UpdateData (true);

		CString selection;
		m_comboQuestType.GetLBText (m_comboQuestType.GetCurSel (), selection);
		quest->setQuestType (std::string (selection));

		quest->setOverallObjvar (std::string (m_overallObjvar));
		quest->setTempObjvar (std::string (m_tempObjvar));

		quest->setDeliverObjectTemplate (std::string (m_deliverObjectTemplate));
		quest->setRetrieveObjectTemplate (std::string (m_retrieveObjectTemplate));

		quest->setPlayerScript (std::string (m_playerScript));

		quest->setQuestLeg (m_questLeg);

		m_comboQuestGiverConversation.GetLBText (m_comboQuestGiverConversation.GetCurSel (), selection);
		quest->setMissionConversationType (std::string (selection));

		m_comboQuestNpcConversation.GetLBText (m_comboQuestNpcConversation.GetCurSel (), selection);
		quest->setQuestNpcConversationType (std::string (selection));

		quest->setQuestNpcEntry (std::string (m_questNpcEntry));
		quest->setQuestNpcScript (std::string (m_questNpcScript));
		quest->setQuestNpcName (std::string (m_questNpcName));

		m_comboLocationPlanet.GetLBText (m_comboLocationPlanet.GetCurSel (), selection);		
		quest->setLocationPlanet (std::string (selection));
		quest->setLocationX (m_locationX);
		quest->setLocationY (m_locationY);
		quest->setLocationZ (m_locationZ);

		m_comboGatingFaction.GetLBText (m_comboGatingFaction.GetCurSel (), selection);		
		quest->setGatingFaction (std::string (selection));
		quest->setGatingObjvar (std::string (m_gatingObjvar));
		quest->setGatingObjectTemplate (std::string (m_gatingObjectTemplate));
		quest->setGatingFactionValue (m_gatingFactionAmount);
	}
	else
	{
		m_comboQuestType.SelectString (0, quest->getQuestType ().c_str ());

		m_overallObjvar = quest->getOverallObjvar ().c_str ();
		m_tempObjvar = quest->getTempObjvar ().c_str ();

		m_deliverObjectTemplate = quest->getDeliverObjectTemplate ().c_str ();
		m_retrieveObjectTemplate = quest->getRetrieveObjectTemplate ().c_str ();

		m_playerScript = quest->getPlayerScript ().c_str ();

		m_questLeg = quest->getQuestLeg ();

		m_comboQuestGiverConversation.SelectString (0, quest->getMissionConversationType ().c_str ());
		m_comboQuestNpcConversation.SelectString (0, quest->getQuestNpcConversationType ().c_str ());

		m_questNpcEntry = quest->getQuestNpcEntry ().c_str ();
		m_questNpcScript = quest->getQuestNpcScript ().c_str ();
		m_questNpcName = quest->getQuestNpcName ().c_str ();

		m_comboLocationPlanet.SelectString (0, quest->getLocationPlanet ().c_str ());
		m_locationX = quest->getLocationX ();
		m_locationY = quest->getLocationY ();
		m_locationZ = quest->getLocationZ ();

		m_comboGatingFaction.SelectString (0, quest->getGatingFaction ().c_str ());
		m_gatingObjvar = quest->getGatingObjvar ().c_str ();
		m_gatingObjectTemplate = quest->getGatingObjectTemplate ().c_str ();
		m_gatingFactionAmount = quest->getGatingFactionValue ();
	}

	m_questId.Format ("Quest %i of %i", m_currentQuest + 1, m_questList.size ());
	UpdateData (false);
}

// ----------------------------------------------------------------------

BOOL SwgContentBuilderDlg::DestroyWindow() 
{
	std::for_each (m_questList.begin (), m_questList.end (), PointerDeleter ());
	m_questList.clear ();
	
	return CDialog::DestroyWindow();
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonAppend() 
{
	updateCurrentQuest (true);

	m_currentQuest = m_questList.size ();
	m_questList.push_back (new Quest);

	updateCurrentQuest (false);
}

// ----------------------------------------------------------------------

void SwgContentBuilderDlg::OnButtonBrowsegatingobject() 
{
	UpdateData (true);

	CFileDialog dlg (true, "", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ObjectGating");
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
			IGNORE_RETURN (RecentDirectory::update ("ObjectGating", dlg.GetPathName ()));

			index = pathName.Find ("object");

			m_gatingObjectTemplate = pathName.Right (pathName.GetLength () - index);
			m_gatingObjectTemplate.Replace (".tpf", ".iff");
		}
		
		UpdateData (false);
	}
}

// ======================================================================

