// LaunchMeFirstDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LaunchMeFirst.h"
#include "LaunchMeFirstDlg.h"

#include "..\..\..\..\engine\shared\library\sharedFoundation\src\shared\Binary.h"
#include "..\..\..\..\engine\shared\library\sharedGame\src\shared\core\PlatformFeatureBits.h"

#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLaunchMeFirstDlg dialog

CLaunchMeFirstDlg::CLaunchMeFirstDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLaunchMeFirstDlg::IDD, pParent),
	m_bitsToClear(0)
{
	//{{AFX_DATA_INIT(CLaunchMeFirstDlg)
	m_radio = 0;
	m_custom = _T("");
	m_commandLine = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLaunchMeFirstDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLaunchMeFirstDlg)
	DDX_Control(pDX, IDC_CHECK_GAME_TOW_RETAIL, m_gameTowRetail);
	DDX_Control(pDX, IDC_CHECK_GAME_TOW_PREORDER, m_gameTowPreorder);
	DDX_Control(pDX, IDC_CHECK_GAME_TOW_BETA, m_gameTowBeta);
	DDX_Control(pDX, IDC_CHECK_GAME_JAPANESE_RECAPTURE, m_gameJapaneseRecapture);
	DDX_Control(pDX, IDC_CHECK_SUB_FREETRIAL, m_subCombatBalance);
	DDX_Control(pDX, IDC_CHECK_SUB_COMBATBALANCE, m_subFreeTrial);
	DDX_Control(pDX, IDC_CHECK_SUB_BASE, m_subBase);
	DDX_Control(pDX, IDC_CHECK_GAME_SPACEPREORDER, m_gameSpacePreorder);
	DDX_Control(pDX, IDC_CHECK_GAME_SPACERETAIL, m_gameSpaceRetail);
	DDX_Control(pDX, IDC_CHECK_GAME_SPACEBETA, m_gameSpaceBeta);
	DDX_Control(pDX, IDC_CHECK_GAME_COLLECTORS, m_gameCollectors);
	DDX_Control(pDX, IDC_CHECK_GAME_BASE, m_gameBase);
	DDX_Control(pDX, IDC_CHECK_GAME_SWGRETAIL, m_gameSwgRetail);
	DDX_Control(pDX, IDC_CHECK_GAME_JAPANESE_RETAIL, m_gameJapaneseRetail);
	DDX_Control(pDX, IDC_CHECK_GAME_JAPANESE_COLLECTORS, m_gameJapaneseCollectors);
	DDX_Control(pDX, IDC_CHECK_GAME_COMBAT_UPGRADE, m_gameCombatUpgrade);
	DDX_Control(pDX, IDC_CHECK_GAME_EP3_RETAIL, m_gameEp3Retail);
	DDX_Control(pDX, IDC_CHECK_GAME_EP3_PREORDER, m_gameEp3Preorder);
	DDX_Radio(pDX, IDC_RADIO1, m_radio);
	DDX_Text(pDX, IDC_EDIT1, m_custom);
	DDX_Text(pDX, IDC_COMMANDLINE, m_commandLine);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLaunchMeFirstDlg, CDialog)
	//{{AFX_MSG_MAP(CLaunchMeFirstDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLaunchMeFirstDlg message handlers

BOOL CLaunchMeFirstDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	CRegKey regKey;
	regKey.Open(HKEY_CURRENT_USER, "Software\\Sony Online Entertainment\\StarWarsGalaxies\\LaunchMeFirst");

	DWORD radio = 0;
	regKey.QueryValue(radio, "radio");
	m_radio = radio;

	char buffer[256];
	buffer[0] = '\0';
	DWORD size = sizeof(buffer);
	regKey.QueryValue(buffer, "custom", &size);
	m_custom = buffer;

	// AfxGetApp()->m_lpCmdLine = "-- -s Station sessionId=bw0X0syod5kV9bm0 stationId=611892363 subscriptionFeatures=1 gameFeatures=7";

	m_commandLine = AfxGetApp()->m_lpCmdLine;
	OutputDebugString(m_commandLine);


	{
		char const * const subscriptionFeaturesKey = "subscriptionFeatures=";
		char const * const found = strstr(m_commandLine, subscriptionFeaturesKey);

		int const subscriptionFeatures = found ? atoi(found + strlen(subscriptionFeaturesKey)) : 0;

		if ((subscriptionFeatures & ClientSubscriptionFeature::Base) != 0)
			m_subBase.SetCheck(TRUE);

		if ((subscriptionFeatures & ClientSubscriptionFeature::FreeTrial) != 0)
			m_subFreeTrial.SetCheck(TRUE);

		if ((subscriptionFeatures & ClientSubscriptionFeature::CombatBalanceAccess) != 0)
			m_subCombatBalance.SetCheck(TRUE);

		m_subBase.EnableWindow(FALSE);
		m_subFreeTrial.EnableWindow(FALSE);
		m_subCombatBalance.EnableWindow(FALSE);
	}

	{
		char const * const gameFeaturesKey = "gameFeatures=";
		char const * const found = strstr(m_commandLine, gameFeaturesKey);

		int const gameFeatures = found ? atoi(found + strlen(gameFeaturesKey)) : 0;

		if ((gameFeatures & ClientGameFeature::Base) != 0)
			m_gameBase.SetCheck(TRUE);
		m_gameBase.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::CollectorsEdition) != 0)
			m_gameCollectors.SetCheck(TRUE);
		else
			m_gameCollectors.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::SpaceExpansionBeta) != 0)
			m_gameSpaceBeta.SetCheck(TRUE);
		else
			m_gameSpaceBeta.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::SpaceExpansionPreOrder) != 0)
			m_gameSpacePreorder.SetCheck(TRUE);
		else
			m_gameSpacePreorder.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::SpaceExpansionRetail) != 0)
			m_gameSpaceRetail.SetCheck(TRUE);
		else
			m_gameSpaceRetail.EnableWindow(FALSE);

	
		if ((gameFeatures & ClientGameFeature::SWGRetail) != 0)
			m_gameSwgRetail.SetCheck(TRUE);
		else
			m_gameSwgRetail.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::JapaneseRetail) != 0)
			m_gameJapaneseRetail.SetCheck(TRUE);
		else
			m_gameJapaneseRetail.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::JapaneseCollectors) != 0)
			m_gameJapaneseCollectors.SetCheck(TRUE);
		else
			m_gameJapaneseCollectors.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::CombatUpgrade) != 0)
			m_gameCombatUpgrade.SetCheck(TRUE);
		else
			m_gameCombatUpgrade.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::Episode3ExpansionRetail) != 0)
			m_gameEp3Retail.SetCheck(TRUE);
		else
			m_gameEp3Retail.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::Episode3PreorderDownload) != 0)
			m_gameEp3Preorder.SetCheck(TRUE);
		else
			m_gameEp3Preorder.EnableWindow(FALSE);
		
		if ((gameFeatures & ClientGameFeature::JapaneseRecapture) != 0)
			m_gameJapaneseRecapture.SetCheck(TRUE);
		else
			m_gameJapaneseRecapture.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::TrialsOfObiwanRetail) != 0)
			m_gameTowRetail.SetCheck(TRUE);
		else
			m_gameTowRetail.EnableWindow(FALSE);

		if ((gameFeatures & ClientGameFeature::TrialsOfObiwanPreorder) != 0)
			m_gameTowPreorder.SetCheck(TRUE);
		else
			m_gameTowPreorder.EnableWindow(FALSE);
		
		if ((gameFeatures & ClientGameFeature::TrialsOfObiwanBeta) != 0)
			m_gameTowBeta.SetCheck(TRUE);
		else
			m_gameTowBeta.EnableWindow(FALSE);
	}

	UpdateData (FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLaunchMeFirstDlg::OnPaint() 
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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLaunchMeFirstDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

bool CLaunchMeFirstDlg::run(const char * programName)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	CString arguments = AfxGetApp()->m_lpCmdLine;
	if (m_bitsToClear)
	{
		CString bitsToClear;
		bitsToClear.Format(" -s ClientGame gameBitsToClear=%d", m_bitsToClear);
		arguments += bitsToClear;
	}
		
	char commandLine[1024];
	sprintf(commandLine, "%s %s", programName, arguments); 
	BOOL result = CreateProcess(programName, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (!result)
		MessageBox("Failed to start application", NULL, MB_ICONERROR | MB_OK);

	return result ? true : false;
}

bool CLaunchMeFirstDlg::writeStationConfig()
{
	char buffer[512];
	char * b = buffer;
	strcpy(buffer, AfxGetApp()->m_lpCmdLine);

	char const * sessionId = NULL;
	{
		char const * const sessionIdKey = "sessionId=";
		char const * found = strstr(b, sessionIdKey);
		if (!found)
			return false;
		sessionId = found + strlen(sessionIdKey);

		b = strstr(sessionId, " ");
		if (b)
		{
			*b = '\0';
			++b;
		}
	}

	char const * stationId = NULL;
	{
		char const * const stationIdKey = "stationId=";
		char const * found = strstr(b, stationIdKey);
		if (!found)
			return false;
		stationId = found + strlen(stationIdKey);

		b = strstr(stationId, " ");
		if (b)
		{
			*b = '\0';
			++b;
		}
	}

	char const * subscriptionFeatures = NULL;
	{
		char const * const subscriptionFeaturesKey = "subscriptionFeatures=";
		char const * found = strstr(b, subscriptionFeaturesKey);
		if (!found)
			return false;
		subscriptionFeatures = found + strlen(subscriptionFeaturesKey);

		b = strstr(subscriptionFeatures, " ");
		if (b)
		{
			*b = '\0';
			++b;
		}
	}

	char const * gameFeatures = NULL;
	{
		char const * const gameFeaturesKey = "gameFeatures=";
		char const * found = strstr(b, gameFeaturesKey);
		if (!found)
			return false;
		gameFeatures = found + strlen(gameFeaturesKey);

		b = strstr(gameFeatures, " ");
		if (b)
		{
			*b = '\0';
			++b;
		}
	}

	char const * launcherAvatarName = NULL;
	if (b)
	{
		char const * const launcherAvatarNameKey = "launcherAvatarName=\"";
		char const * found = strstr(b, launcherAvatarNameKey);
		if (found)
		{
			launcherAvatarName = found + strlen(launcherAvatarNameKey);
			b = strstr(launcherAvatarName, "\"");
			if (b)
			{
				*b = '\0';
				++b;
			}
		}
	}

	char const * launcherClusterId = NULL;
	if (b)
	{
		char const * const launcherClusterIdKey = "launcherClusterId=\"";
		char const * found = strstr(b, launcherClusterIdKey);
		if (found)
		{
			launcherClusterId = found + strlen(launcherClusterIdKey);
			b = strstr(launcherClusterId, "\"");
			if (b)
			{
				*b = '\0';
				++b;
			}
		}
	}


	FILE *f = fopen("station.cfg", "wt");
	if (!f)
		return false;

	if (sessionId || stationId || subscriptionFeatures || gameFeatures)
	{
		fprintf(f, "[Station]\n");
		
		if (sessionId)
			fprintf(f, "\tsessionId=%s\n", sessionId);

		if (stationId)
			fprintf(f, "\tstationId=%s\n", stationId);

		if (subscriptionFeatures)
			fprintf(f, "\tsubscriptionFeatures=%s\n", subscriptionFeatures);

		if (gameFeatures)
			fprintf(f, "\tgameFeatures=%s\n", gameFeatures);

	}

	if (launcherAvatarName || launcherClusterId || m_bitsToClear)
	{
		fprintf(f, "[ClientGame]\n");
		if (launcherAvatarName)
			fprintf(f, "\tlauncherAvatarName=\"%s\"\n", launcherAvatarName);
		if (launcherClusterId)
			fprintf(f, "\tlauncherClusterId=\"%s\"\n", launcherClusterId);
		if (m_bitsToClear)
			fprintf(f, "\tgameBitsToClear=%d\n", m_bitsToClear);
	}

	fclose(f);

	return true;
}


void CLaunchMeFirstDlg::OnOK() 
{
	UpdateData(TRUE);

	// figure out what bits the user wants to clear
	{
		if (m_gameBase.IsWindowEnabled() && m_gameBase.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::Base;

		if (m_gameCollectors.IsWindowEnabled() && m_gameCollectors.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::CollectorsEdition;

		if (m_gameSpaceBeta.IsWindowEnabled() && m_gameSpaceBeta.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::SpaceExpansionBeta;

		if (m_gameSpacePreorder.IsWindowEnabled() && m_gameSpacePreorder.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::SpaceExpansionPreOrder;

		if (m_gameSpaceRetail.IsWindowEnabled() && m_gameSpaceRetail.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::SpaceExpansionRetail;

		if (m_gameSwgRetail.IsWindowEnabled() && m_gameSwgRetail.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::SWGRetail;

		if (m_gameJapaneseRetail.IsWindowEnabled() && m_gameJapaneseRetail.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::JapaneseRetail;

		if (m_gameJapaneseCollectors.IsWindowEnabled() && m_gameJapaneseCollectors.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::JapaneseCollectors;

		if (m_gameCombatUpgrade.IsWindowEnabled() && m_gameCombatUpgrade.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::CombatUpgrade;

		if (m_gameEp3Retail.IsWindowEnabled() && m_gameEp3Retail.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::Episode3ExpansionRetail;

		if (m_gameEp3Preorder.IsWindowEnabled() && m_gameEp3Preorder.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::Episode3PreorderDownload;

		if (m_gameJapaneseRecapture.IsWindowEnabled() && m_gameJapaneseRecapture.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::JapaneseRecapture;

		if (m_gameTowRetail.IsWindowEnabled() && m_gameTowRetail.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::TrialsOfObiwanRetail;

		if (m_gameTowPreorder.IsWindowEnabled() && m_gameTowPreorder.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::TrialsOfObiwanPreorder;

		if (m_gameTowBeta.IsWindowEnabled() && m_gameTowBeta.GetCheck() == 0)
			m_bitsToClear |= ClientGameFeature::TrialsOfObiwanBeta;
	}

	bool result = false;
	switch (m_radio)
	{
		case 0:
			result = run("SwgClient_d.exe");
			break;

		case 1:
			result = run("SwgClient_o.exe");
			break;

		case 2:
			result = run("SwgClient_r.exe");
			break;

		case 3:
			result = writeStationConfig();
			break;

		case 4:
			result = run(m_custom);
			break;

		case 5:
			result = run("SwgClientSetup_d.exe");
			break;

		case 6:
			result = run("SwgClientSetup_o.exe");
			break;

		case 7:
			result = run("SwgClientSetup_r.exe");
			break;
	}

	if (result)
	{
		CRegKey regKey;
		regKey.Create(HKEY_CURRENT_USER, "Software\\Sony Online Entertainment\\StarWarsGalaxies\\LaunchMeFirst");
		regKey.SetValue(static_cast<DWORD>(m_radio), "radio");
		regKey.SetValue(m_custom, "custom");

		CDialog::OnOK();
	}
}
