// ShellView.cpp : implementation file
//

#include "FirstSwgConversationEditor.h"
#include "ShellView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShellView

IMPLEMENT_DYNCREATE(CShellView, CEditView)

CShellView::CShellView()
{
	dwProcessId = DWORD(-1);
	m_pReadThread = NULL;
	m_redirectsSetUp = false;

	//	Initialize LOGFONT structure
	memset(&m_lf, 0, sizeof(m_lf));
	m_lf.lfWeight = FW_NORMAL;
	m_lf.lfCharSet = ANSI_CHARSET;
	m_lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_lf.lfQuality = DEFAULT_QUALITY;
	m_lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy(m_lf.lfFaceName, "Courier");
}

CShellView::~CShellView()
{
}

BEGIN_MESSAGE_MAP(CShellView, CEditView)
	//{{AFX_MSG_MAP(CShellView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShellView drawing

void CShellView::OnDraw(CDC* /*pDC*/)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CShellView diagnostics

#ifdef _DEBUG
void CShellView::AssertValid() const
{
	CEditView::AssertValid();
}

void CShellView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CShellView message handlers

int CShellView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEditView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	GetEditCtrl().SetReadOnly( TRUE );
	DWORD dwStyle = GetWindowLong( GetEditCtrl().GetSafeHwnd(),GWL_STYLE );
	if( dwStyle )
	{
		dwStyle |= DS_LOCALEDIT;
		SetWindowLong( GetEditCtrl().GetSafeHwnd(),GWL_STYLE,dwStyle );
	}

	if( m_defFont.CreateFontIndirect( &m_lf ) )
		GetEditCtrl().SetFont( &m_defFont );

	return 0;
}

BOOL CShellView::SetUpRedirects()
{
	if(m_redirectsSetUp)
	{
		return false;
	}

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

	// Create a pipe for the child process's STDOUT.  
	if( !CreatePipe( &hChildStdoutRd, &hChildStdoutWr, &saAttr, 0) )
	{
		TRACE0( _T("Stdout pipe creation failed\n") );
		return FALSE;
	}

	// Create noninheritable read handle and close the inheritable read handle. 
	fSuccess = DuplicateHandle( GetCurrentProcess(), hChildStdoutRd,
		GetCurrentProcess(),  &hChildStdoutRdDup , 
		0,  FALSE,
		DUPLICATE_SAME_ACCESS );
	if( !fSuccess )
	{
		TRACE0( _T("DuplicateHandle failed\n") );
		return FALSE;
	}
	CloseHandle( hChildStdoutRd );
	hChildStdoutRd = 0;

	// The steps for redirecting child process's STDIN: 
	//     1.  Save current STDIN, to be restored later. 
	//     2.  Create anonymous pipe to be STDIN for child process. 
	//     3.  Set STDIN of the parent to be the read handle to the 
	//         pipe, so it is inherited by the child process. 
	//     4.  Create a noninheritable duplicate of the write handle, 
	//         and close the inheritable write handle.  

	// Create a pipe for the child process's STDIN.  
	if( !CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0) )
	{
		TRACE0( _T("Stdin pipe creation failed\n") );
		return FALSE;
	}
	// Duplicate the write handle to the pipe so it is not inherited.  
	fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr, 
		GetCurrentProcess(), &hChildStdinWrDup, 
		0, FALSE,                  // not inherited       
		DUPLICATE_SAME_ACCESS ); 
	if( !fSuccess ) 
	{
		TRACE0( _T("DuplicateHandle failed\n") );
		return FALSE;
	}
	CloseHandle(hChildStdinWr);  
	hChildStdinWr = 0;

	if(!m_pReadThread)
	{
		m_pReadThread = AfxBeginThread( (AFX_THREADPROC)ReadPipeThreadProc,(LPVOID)this );

		if( !m_pReadThread )
		{
			TRACE0( _T("Cannot start read-redirect thread!\n") );
			return FALSE;
		}
	}

	m_redirectsSetUp = true;
	return true;
}

#define BUFSIZE 4096 * 4

BOOL CShellView::CreateShellRedirect(CString const & commandLine)
{
	if(!m_redirectsSetUp)
	{
		SetUpRedirects();
	}

	// Save the handle to the current STDOUT.  
	hSaveStdout = GetStdHandle(STD_OUTPUT_HANDLE);  
	// Set a write handle to the pipe to be STDOUT.  
	if( !SetStdHandle(STD_OUTPUT_HANDLE, hChildStdoutWr) )
	{
		TRACE0( _T("Redirecting STDOUT failed\n") );
		return FALSE;
	}

	// Save the handle to the current STDIN. 
	hSaveStdin = GetStdHandle(STD_INPUT_HANDLE);  
	// Set a read handle to the pipe to be STDIN.  
	if( !SetStdHandle(STD_INPUT_HANDLE, hChildStdinRd) ) 
	{
		TRACE0( _T("Redirecting Stdin failed\n") );
		return FALSE;
	}

	// Now create the child process. 
	if( !CreateChildProcess(dwProcessId, commandLine) )
	{
		TRACE0( _T("CreateChildProcess failed\n") );
		return FALSE;
	}

	// After process creation, restore the saved STDIN and STDOUT.  
	if( !SetStdHandle(STD_INPUT_HANDLE, hSaveStdin) )
	{
		TRACE0( _T("Re-redirecting Stdin failed\n") );
		return FALSE;
	}
	if( !SetStdHandle(STD_OUTPUT_HANDLE, hSaveStdout) )
	{
		TRACE0( _T("Re-redirecting Stdout failed\n") );
		return FALSE;
	}

	return TRUE;
}

BOOL CShellView::CreateChildProcess(DWORD& dwProcessId, CString const & commandLine)
{
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;  

	// Set up members of STARTUPINFO structure.  
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO);  
	
	siStartInfo.dwFlags = STARTF_USESTDHANDLES;
	siStartInfo.hStdInput = hChildStdinRd;
	siStartInfo.hStdOutput = hChildStdoutWr;
	siStartInfo.hStdError = hChildStdoutWr;

	TCHAR shellCmd[_MAX_PATH];
	strcpy (shellCmd, commandLine);

	// Create the child process.  
	BOOL ret = CreateProcess( NULL,
			shellCmd,       // applicatin name
			NULL,          // process security attributes 
			NULL,          // primary thread security attributes 
			TRUE,          // handles are inherited 
			DETACHED_PROCESS, // creation flags 
			NULL,          // use parent's environment 
			NULL,          // use parent's current directory 
			&siStartInfo,  // STARTUPINFO pointer 
			&piProcInfo);  // receives PROCESS_INFORMATION 
	if( ret )
		dwProcessId = piProcInfo.dwProcessId;
	return ret;
}

void CShellView::WriteToPipe( LPCTSTR line )
{
	DWORD dwWritten;

	WriteFile( hChildStdinWrDup, line, _tcslen(line)*sizeof(TCHAR), 
			&dwWritten, NULL );
}

UINT CShellView::ReadPipeThreadProc( LPVOID pParam )
{
	DWORD dwRead;
	TCHAR chBuf[BUFSIZE]; 
	CShellView* pView = (CShellView*)pParam;

	TRACE0( _T("ReadPipe Thread begin run\n") );
	for( ;; )    
	{ 
		if( !ReadFile( pView->hChildStdoutRdDup, chBuf, 
			BUFSIZE, &dwRead, NULL) || dwRead == 0) 
			break; 
		chBuf[dwRead/sizeof(TCHAR)] = _T('\0');
		pView->AddTexts( chBuf );
		pView->m_nLength = pView->GetEditCtrl().SendMessage( WM_GETTEXTLENGTH );
	} 
	CloseHandle( pView ->hChildStdinRd);
	CloseHandle( pView ->hChildStdoutWr);
	CloseHandle( pView ->hChildStdinWrDup );
	CloseHandle( pView ->hChildStdoutRdDup );
	pView->m_pReadThread = NULL;
	pView->dwProcessId = DWORD(-1);
	pView->PostMessage( WM_CLOSE );
	return 1;
}


void CShellView::OnDestroy() 
{
	if( dwProcessId!=DWORD(-1) )
	{
		HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwProcessId );
		if( hProcess )
		{
			TerminateProcess( hProcess,0 );
			CloseHandle( hProcess );
		}
	}
	if( m_pReadThread )
	{
		TerminateThread( m_pReadThread->m_hThread,0 );
		delete m_pReadThread;
	}

	//clean up the pipes we created
	if(m_redirectsSetUp)
	{
		CloseHandle(hChildStdinRd);
		CloseHandle(hChildStdinWrDup);
		CloseHandle(hChildStdoutWr);
		CloseHandle(hChildStdoutRdDup);
		m_redirectsSetUp = false;
	}


	CEditView::OnDestroy();
}

void CShellView::AddTexts(LPCTSTR string)
{
	MoveToEnd();
	GetEditCtrl().ReplaceSel( string );
}

void CShellView::AddTexts(TCHAR ch)
{
	TCHAR string[2];
	string[0] = ch;
	string[1] = _T('\0');
	AddTexts( (LPCTSTR)string );
}

void CShellView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int nPos = GetCurrentPosition();
	if( nChar==8 && nPos<=m_nLength )
		return;
	if( nPos<m_nLength )
		MoveToEnd();
	CEditView::OnChar(nChar, nRepCnt, nFlags);
	if( nChar==13 )
	{
		CString input;
		GetUserInput(input);
		WriteToPipe( input );
	}
}

void CShellView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if( nChar==VK_DELETE )
	{
		if( GetCurrentPosition()<m_nLength )
			return;
	}
	CEditView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CShellView::MoveToEnd()
{
	int nLen = GetEditCtrl().SendMessage( WM_GETTEXTLENGTH );
	GetEditCtrl().SetSel( nLen,nLen );
}

int CShellView::GetCurrentPosition()
{
	GetEditCtrl().SetSel(-1,-1);
	int nstart,nstop;
	GetEditCtrl().GetSel(nstart,nstop);
	return nstart;
}

void CShellView::GetUserInput(CString &input)
{
	int where = GetCurrentPosition();
	HLOCAL hBuffer = GetEditCtrl().GetHandle();
	if( hBuffer )
	{
		LPCTSTR szBuffer = (LPCTSTR)LocalLock(hBuffer);
		if( szBuffer )
		{
			input = CString( szBuffer+m_nLength,(where-m_nLength) );
			LocalUnlock( hBuffer );
		}
	}
}

int CShellView::GetSelLength()
{
	int nstart,nstop;
	GetEditCtrl().GetSel(nstart,nstop);
	return (nstart-nstop);
}

BOOL CShellView::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CEditView::PreCreateWindow(cs);
}
