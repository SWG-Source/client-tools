// ======================================================================
//
// DialogConsole.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "DialogConsole.h"

#include "MainFrame.h"

// ======================================================================

DialogConsole::DialogConsole(CWnd* pParent /*=NULL*/) : 
	CDialog(DialogConsole::IDD, pParent),
	m_configurationEditCtrl(),
	m_outputEditCtrl(),
	m_warningEditCtrl(),
	m_shellEditCtrl(),
	m_selectorTabCtrl(),
	m_initialized(false)
{
	dwProcessId = DWORD(-1);
	m_pReadThread = NULL;

	//{{AFX_DATA_INIT(DialogConsole)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

DialogConsole::~DialogConsole()
{
}

// ----------------------------------------------------------------------

void DialogConsole::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogConsole)
	DDX_Control(pDX, IDC_EDIT_CONFIGURATION, m_configurationEditCtrl);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_outputEditCtrl);
	DDX_Control(pDX, IDC_EDIT_WARNING, m_warningEditCtrl);
	DDX_Control(pDX, IDC_EDIT_SHELL, m_shellEditCtrl);
	DDX_Control(pDX, IDC_TAB_SELECTOR, m_selectorTabCtrl);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogConsole, CDialog)
	//{{AFX_MSG_MAP(DialogConsole)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SELECTOR, OnSelchangeTabSelector)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void DialogConsole::addConfiguration(CString const & text)
{
	if (GetSelectorTabCtrl().GetCurSel() != T_configuration)
	{
		GetSelectorTabCtrl().SetCurSel(T_configuration);
		LRESULT result;
		OnSelchangeTabSelector(0, &result);
	}

	CString current;
	GetConfigurationEditCtrl().GetWindowText(current);

	CString line(current + text);
	line.Replace("\r\n", "\n");
	line.Replace("\n", "\r\n");
	GetConfigurationEditCtrl().SetWindowText(line);

	GetConfigurationEditCtrl().LineScroll(GetConfigurationEditCtrl().GetLineCount());
}

// ----------------------------------------------------------------------

void DialogConsole::addOutput(CString const & text)
{
	if (GetSelectorTabCtrl().GetCurSel() != T_output)
	{
		GetSelectorTabCtrl().SetCurSel(T_output);
		LRESULT result;
		OnSelchangeTabSelector(0, &result);
	}

	CString current;
	GetOutputEditCtrl().GetWindowText(current);

	CString line(current + text);
	line.Replace("\r\n", "\n");
	line.Replace("\n", "\r\n");
	GetOutputEditCtrl().SetWindowText(line);

	GetOutputEditCtrl().LineScroll(GetOutputEditCtrl().GetLineCount());
}

// ----------------------------------------------------------------------

void DialogConsole::addWarning(CString const & text)
{
	if (GetSelectorTabCtrl().GetCurSel() != T_warning)
	{
		GetSelectorTabCtrl().SetCurSel(T_warning);
		LRESULT result;
		OnSelchangeTabSelector(0, &result);
	}

	CString current;
	GetWarningEditCtrl().GetWindowText(current);

	CString line(current + text);
	line.Replace("\r\n", "\n");
	line.Replace("\n", "\r\n");
	GetWarningEditCtrl().SetWindowText(line);

	GetWarningEditCtrl().LineScroll(GetWarningEditCtrl().GetLineCount());
}

// ----------------------------------------------------------------------

void DialogConsole::execute(CString const & text)
{
	if (!text.IsEmpty())
	{
		if (GetSelectorTabCtrl().GetCurSel() != T_shell)
		{
			GetSelectorTabCtrl().SetCurSel(T_shell);
			LRESULT result;
			OnSelchangeTabSelector(0, &result);
		}

		CString line(text);
		if (line.ReverseFind('\n') != line.GetLength())
			line += "\n";

		line.Replace("\r\n", "\n");
		line.Replace("\n", "\r\n");

		WriteToPipe(line);
	}
}

// ----------------------------------------------------------------------

CTabCtrl & DialogConsole::GetSelectorTabCtrl() 
{
	return m_selectorTabCtrl;
}

// ----------------------------------------------------------------------

CEdit & DialogConsole::GetConfigurationEditCtrl() 
{
	return m_configurationEditCtrl;
}

// ----------------------------------------------------------------------

CEdit & DialogConsole::GetOutputEditCtrl() 
{
	return m_outputEditCtrl;
}

// ----------------------------------------------------------------------

CEdit & DialogConsole::GetWarningEditCtrl() 
{
	return m_warningEditCtrl;
}

// ----------------------------------------------------------------------

CEdit & DialogConsole::GetShellEditCtrl() 
{
	return m_shellEditCtrl;
}

// ----------------------------------------------------------------------

BOOL DialogConsole::OnInitDialog() 
{
	CDialog::OnInitDialog();

	{
		memset(&m_lf, 0, sizeof(m_lf));
		m_lf.lfHeight = -10;
		m_lf.lfWeight = FW_NORMAL;
		m_lf.lfCharSet = ANSI_CHARSET;
		m_lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		m_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		m_lf.lfQuality = DEFAULT_QUALITY;
		m_lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		strcpy(m_lf.lfFaceName, "Courier");
	}

	GetSelectorTabCtrl().InsertItem(0, "Configuration");
	GetSelectorTabCtrl().InsertItem(1, "Output");
	GetSelectorTabCtrl().InsertItem(2, "Warning");
	GetSelectorTabCtrl().InsertItem(3, "Shell");
	GetSelectorTabCtrl().SetCurSel(0);

	m_editCtrlList[0] = &GetConfigurationEditCtrl();
	m_editCtrlList[1] = &GetOutputEditCtrl();
	m_editCtrlList[2] = &GetWarningEditCtrl();
	m_editCtrlList[3] = &GetShellEditCtrl();

	for (int i = 0; i < T_COUNT; ++i)
	{
		if (m_fontList[i].CreateFontIndirect(&m_lf))
			m_editCtrlList[i]->SetFont(&m_fontList[i]);

		m_editCtrlList[i]->ShowWindow(SW_HIDE);
	}

	m_initialized = true;

	CreateShellRedirect();
	execute("p4 info");

	GetSelectorTabCtrl().SetCurSel(T_configuration);
	LRESULT result;
	OnSelchangeTabSelector(0, &result);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogConsole::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (!m_initialized)
		return;

	CRect sz;
	GetClientRect(sz);
	sz.DeflateRect(5,5);

	GetSelectorTabCtrl().MoveWindow(sz);
	sz.DeflateRect(2, 2);
	sz.bottom -= 21;

	for (int i = 0; i < T_COUNT; ++i)
		m_editCtrlList[i]->MoveWindow(sz);
}

// ----------------------------------------------------------------------

void DialogConsole::OnOK() 
{
}

// ----------------------------------------------------------------------

void DialogConsole::OnSelchangeTabSelector(NMHDR * const /*pNMHDR*/, LRESULT * const pResult) 
{
	// TODO: Add your control notification handler code here
	int const index = GetSelectorTabCtrl().GetCurSel();
	for (int i = 0; i < T_COUNT; ++i)
		m_editCtrlList[i]->ShowWindow(i == index ? SW_SHOW : SW_HIDE);

	GetSelectorTabCtrl().RedrawWindow();
	m_editCtrlList[index]->SetFocus();

	*pResult = 0;
}

// ======================================================================
// 
// ======================================================================

#define BUFSIZE 65536

BOOL DialogConsole::CreateShellRedirect()
{
	SECURITY_ATTRIBUTES saAttr;
	BOOL fSuccess;

	// Set the bInheritHandle flag so pipe handles are inherited.
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// The steps for redirecting child process's STDOUT: 
	//     1. Save current STDOUT, to be restored later. 
	//     2. Create anonymous pipe to be STDOUT for child process. 
	//     3. Set STDOUT of the parent process to be write handle to 
	//        the pipe, so it is inherited by the child process. 
	//     4. Create a noninheritable duplicate of the read handle and
	//        close the inheritable read handle.  
	
	// Save the handle to the current STDOUT.  
	hSaveStdout = GetStdHandle(STD_OUTPUT_HANDLE);  
	
	// Create a pipe for the child process's STDOUT.  
	if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
	{
		TRACE0(_T("Stdout pipe creation failed\n"));
		return FALSE;
	}

	// Set a write handle to the pipe to be STDOUT.  
	if (!SetStdHandle(STD_OUTPUT_HANDLE, hChildStdoutWr))
	{
		TRACE0(_T("Redirecting STDOUT failed\n"));
		return FALSE;
	}
   
	// Create noninheritable read handle and close the inheritable read handle. 
    fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
        GetCurrentProcess(),  &hChildStdoutRdDup , 
		0,  FALSE,
        DUPLICATE_SAME_ACCESS);
	if (!fSuccess)
	{
		TRACE0(_T("DuplicateHandle failed\n"));
        return FALSE;
	}
	CloseHandle(hChildStdoutRd);
	
	// The steps for redirecting child process's STDIN: 
	//     1.  Save current STDIN, to be restored later. 
	//     2.  Create anonymous pipe to be STDIN for child process. 
	//     3.  Set STDIN of the parent to be the read handle to the 
	//         pipe, so it is inherited by the child process. 
	//     4.  Create a noninheritable duplicate of the write handle, 
	//         and close the inheritable write handle.  

	// Save the handle to the current STDIN. 
	hSaveStdin = GetStdHandle(STD_INPUT_HANDLE);  

	// Create a pipe for the child process's STDIN.  
	if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
	{
		TRACE0(_T("Stdin pipe creation failed\n"));
		return FALSE;
	}
	// Set a read handle to the pipe to be STDIN.  
	if (!SetStdHandle(STD_INPUT_HANDLE, hChildStdinRd)) 
	{
		TRACE0(_T("Redirecting Stdin failed\n"));
		return FALSE;
	}
	// Duplicate the write handle to the pipe so it is not inherited.  
	fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr, 
		GetCurrentProcess(), &hChildStdinWrDup, 
		0, FALSE,                  // not inherited       
		DUPLICATE_SAME_ACCESS); 
	if (!fSuccess) 
	{
		TRACE0(_T("DuplicateHandle failed\n"));
		return FALSE;
	}
	CloseHandle(hChildStdinWr);  
	
	// Now create the child process. 
	if (!CreateChildProcess())
	{
		TRACE0(_T("CreateChildProcess failed\n"));
		return FALSE;
	}
	// After process creation, restore the saved STDIN and STDOUT.  
	if (!SetStdHandle(STD_INPUT_HANDLE, hSaveStdin))
	{
		TRACE0(_T("Re-redirecting Stdin failed\n"));
		return FALSE;
	}
	if (!SetStdHandle(STD_OUTPUT_HANDLE, hSaveStdout))
	{
		TRACE0(_T("Re-redirecting Stdout failed\n"));
		return FALSE;
	}
	m_pReadThread = AfxBeginThread((AFX_THREADPROC)ReadPipeThreadProc,(LPVOID)this);
	if (!m_pReadThread)
	{
		TRACE0(_T("Cannot start read-redirect thread!\n"));
		return FALSE;
	}
	return TRUE;
}

BOOL DialogConsole::CreateChildProcess()
{
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;  

	// Set up members of STARTUPINFO structure.  
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);  
	
	siStartInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	siStartInfo.wShowWindow = SW_HIDE;
	siStartInfo.hStdInput = hChildStdinRd;
	siStartInfo.hStdOutput = hChildStdoutWr;
	siStartInfo.hStdError = hChildStdoutWr;

	TCHAR shellCmd[_MAX_PATH];
	if (!GetEnvironmentVariable(_T("ComSpec"), shellCmd, _MAX_PATH))
		  return FALSE;

	// Create the child process.  
	BOOL ret = CreateProcess(NULL, 
			shellCmd, 	   // applicatin name
			NULL,          // process security attributes 
			NULL,          // primary thread security attributes 
			TRUE,          // handles are inherited 
			CREATE_NEW_CONSOLE, // creation flags 
			NULL,          // use parent's environment 
			NULL,          // use parent's current directory 
			&siStartInfo,  // STARTUPINFO pointer 
			&piProcInfo);  // receives PROCESS_INFORMATION 

	if (ret)
		dwProcessId = piProcInfo.dwProcessId;

	return ret;
}

UINT DialogConsole::ReadPipeThreadProc(LPVOID pParam)
{
	DWORD dwRead;
	TCHAR chBuf[BUFSIZE]; 
	DialogConsole* pView =(DialogConsole*)pParam;

	TRACE0(_T("ReadPipe Thread begin run\n"));
	for (;;)    
	{ 
		if (!ReadFile(pView->hChildStdoutRdDup, chBuf, BUFSIZE, &dwRead, NULL) || dwRead == 0) 
			break; 
		chBuf[dwRead/sizeof(TCHAR)] = _T('\0');
		pView->AddTexts(chBuf);
	} 
	CloseHandle(pView ->hChildStdinRd);
	CloseHandle(pView ->hChildStdoutWr);
	CloseHandle(pView ->hChildStdinWrDup);
	CloseHandle(pView ->hChildStdoutRdDup);
	pView->m_pReadThread = NULL;
	pView->dwProcessId = DWORD(-1);
	pView->PostMessage(WM_CLOSE);

	return 1;
}

void DialogConsole::WriteToPipe(LPCTSTR line)
{
	DWORD dwWritten;

	WriteFile(hChildStdinWrDup, line, _tcslen(line)*sizeof(TCHAR), 
			&dwWritten, NULL);
}

void DialogConsole::OnDestroy() 
{
	if (dwProcessId!=DWORD(-1))
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
		if (hProcess)
		{
			TerminateProcess(hProcess,0);
			CloseHandle(hProcess);
		}
	}
	if (m_pReadThread)
	{
		TerminateThread(m_pReadThread->m_hThread,0);
		delete m_pReadThread;
	}
	CDialog::OnDestroy();
}

void DialogConsole::AddTexts(LPCTSTR string)
{
	CString line(string);
	line.Replace("\n", "\r\n");
	MoveToEnd();
	GetShellEditCtrl().ReplaceSel(line);

	GetShellEditCtrl().LineScroll(GetShellEditCtrl().GetLineCount());
}

void DialogConsole::MoveToEnd()
{
	int nLen = GetShellEditCtrl().SendMessage(WM_GETTEXTLENGTH);
	GetShellEditCtrl().SetSel(nLen,nLen);
}

// ======================================================================

