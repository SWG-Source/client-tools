// PixelShaderProgramView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "shaderbuilder.h"
#include "PixelShaderProgramView.h"

#include "Node.h"
#include "RecentDirectory.h"
#include "IncludeHandler.h"

#include "..\..\..\Direct3d9\src\win32\Direct3d9_PixelShaderConstantRegisters.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef std::map<CString, CString>  PixelConstantMap;
static PixelConstantMap s_pixelConstantMap;

/////////////////////////////////////////////////////////////////////////////
// CPixelShaderProgramView

IMPLEMENT_DYNCREATE(CPixelShaderProgramView, CFormView)

CPixelShaderProgramView::CPixelShaderProgramView()
	: CFormView(CPixelShaderProgramView::IDD),
	m_font(0),
	m_pixelShaderProgramNode(NULL)
{
	//{{AFX_DATA_INIT(CPixelShaderProgramView)
	//}}AFX_DATA_INIT

	if (s_pixelConstantMap.empty())
	{
		char buffer[8];

		#define ADD_CONSTANT(a) sprintf(buffer, "c%d", PSCR_ ## a); IGNORE_RETURN(s_pixelConstantMap.insert(std::make_pair(static_cast<const char *>("c[" # a "]"), CString(buffer))))

		ADD_CONSTANT(dot3LightDiffuseColor);
		ADD_CONSTANT(dot3LightSpecularColor);
		ADD_CONSTANT(textureFactor);
		ADD_CONSTANT(textureFactor2);
		ADD_CONSTANT(materialSpecularColor);
		ADD_CONSTANT(userConstant);

		#undef ADD_CONSTANT

		sprintf(buffer, "c%d.xyz", PSCR_dot3LightDirection);
		IGNORE_RETURN(s_pixelConstantMap.insert(std::make_pair(static_cast<const char *>("c[dot3LightDirection]"), CString(buffer))));

		sprintf(buffer, "c%d.w", PSCR_dot3LightDirection);
		IGNORE_RETURN(s_pixelConstantMap.insert(std::make_pair(static_cast<const char *>("c[materialSpecularPower]"), CString(buffer))));

		sprintf(buffer, "c%d.a", PSCR_dot3LightSpecularColor);
		IGNORE_RETURN(s_pixelConstantMap.insert(std::make_pair(static_cast<const char *>("c[alphaFadeOpacity]"), CString(buffer))));
	}
}

// ----------------------------------------------------------------------

CPixelShaderProgramView::~CPixelShaderProgramView()
{
	m_font->DeleteObject();
	delete m_font;
}

// ----------------------------------------------------------------------

void CPixelShaderProgramView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPixelShaderProgramView)
	DDX_Control(pDX, IDC_PIXELSHADERPROGRAM_SOURCE, m_source);
	DDX_Control(pDX, IDC_PIXELSHADERPROGRAM_FILENAME, m_fileName);
	DDX_Control(pDX, IDC_PIXELSHADERPROGRAM_CREATE, m_create);
	DDX_Control(pDX, IDC_PIXELSHADERPROGRAM_COMPILE, m_compile);
	DDX_Control(pDX, IDC_PIXELSHADER_ERRORMESSAGE_EDIT, m_error);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPixelShaderProgramView, CFormView)
	//{{AFX_MSG_MAP(CPixelShaderProgramView)
	ON_BN_CLICKED(IDC_PIXELSHADERPROGRAM_COMPILE, OnPixelshaderprogramCompile)
	ON_EN_CHANGE(IDC_PIXELSHADERPROGRAM_SOURCE, OnChangePixelshaderprogramSource)
	ON_BN_CLICKED(IDC_PIXELSHADERPROGRAM_CREATE, OnPixelshaderprogramCreate)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPixelShaderProgramView diagnostics

#ifdef _DEBUG
void CPixelShaderProgramView::AssertValid() const
{
	CFormView::AssertValid();
}

// ----------------------------------------------------------------------

void CPixelShaderProgramView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPixelShaderProgramView message handlers


void CPixelShaderProgramView::SetNode(CPixelShaderProgramNode &node)
{
	m_pixelShaderProgramNode = &node;

	m_fileName.SetWindowText(m_pixelShaderProgramNode->m_absoluteFileName);
	m_source.SetWindowText(m_pixelShaderProgramNode->m_text);
	m_error.SetWindowText("");
	m_source.LineScroll(m_pixelShaderProgramNode->m_scroll);

	if (m_pixelShaderProgramNode->m_absoluteFileName == "")
	{
		m_create.ShowWindow(SW_SHOW);
		m_compile.ShowWindow(SW_HIDE);
		m_error.ShowWindow(SW_HIDE);
		m_source.ShowWindow(SW_HIDE);
	}
	else
	{
		m_create.ShowWindow(SW_HIDE);
		m_compile.ShowWindow(SW_SHOW);
		m_error.ShowWindow(SW_SHOW);
		m_source.ShowWindow(SW_SHOW);

#if _MSC_VER < 1300
		const DWORD INVALID_FILE_ATTRIBUTES = 0xffffffff;
#endif

		DWORD attributes = GetFileAttributes(m_pixelShaderProgramNode->m_absoluteFileName);
		m_source.SetReadOnly(attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_READONLY) != 0);
	}
}

void CPixelShaderProgramView::SwitchAway()
{
	if (m_pixelShaderProgramNode)
	{
		m_pixelShaderProgramNode->m_scroll = m_source.GetFirstVisibleLine();
		m_pixelShaderProgramNode = NULL;
	}
}

// ----------------------------------------------------------------------

bool CPixelShaderProgramView::CompileAsm()
{
	ID3DXBuffer *compiledShader = NULL;
	ID3DXBuffer *errors = NULL;

	CString compileText = m_pixelShaderProgramNode->m_text;

	// replace all the constant register names with the constant register numbers
	const PixelConstantMap::const_iterator iEnd = s_pixelConstantMap.end();
	for (PixelConstantMap::const_iterator i = s_pixelConstantMap.begin(); i != iEnd; ++i)
		compileText.Replace(i->first, i->second);

	HRESULT result = D3DXAssembleShader(static_cast<const char *>(compileText), compileText.GetLength(), NULL, NULL, 0, &compiledShader, &errors);

	if (FAILED(result))
	{
		if (errors)
		{
			//-- clean up returned error
			CString error (reinterpret_cast<const char *>(errors->GetBufferPointer()));
			error.Replace ("\n", "\r\n");
			m_error.SetWindowText(error);

			//-- highlight line with error
			if (error [0] == '(')
			{
				//-- error line numbers are returned one of two ways, including and not including comments
				if (error [1] == 'S')
				{
					//-- handle not including comments (not worth it)
				}
				else
				{
					//-- handle the error case of the line number including comments
					const int index = error.Find (')');
					if (index != -1)
					{
						CString source;
						m_source.GetWindowText (source);

						CString desiredLineNumber = error.Mid (1, index - 1);
						int lineNumber = atoi (desiredLineNumber) - 1;

						int start = 0;
						while (lineNumber > 0)
						{
							start = source.Find ('\n', start + 1);
							--lineNumber;
						}

						int end = source.Find ('\n', start + 1);
						if (end == -1)
							end = source.GetLength ();

						if (start != end)
						{
							m_source.SetSel (start, end);
							m_source.SetFocus ();
						}
					}
				}
			}
		}
		else
			m_error.SetWindowText("No error message returned");
		return false;
	}

	delete [] m_pixelShaderProgramNode->m_exe;
	m_pixelShaderProgramNode->m_exeLength = compiledShader->GetBufferSize();
	m_pixelShaderProgramNode->m_exe = new byte[m_pixelShaderProgramNode->m_exeLength];
	m_pixelShaderProgramNode->m_compiledIndex = ++CPixelShaderNode::ms_compiledIndex;
	memcpy(m_pixelShaderProgramNode->m_exe, compiledShader->GetBufferPointer(), m_pixelShaderProgramNode->m_exeLength);

	if (!m_pixelShaderProgramNode->Save())
	{
		char buffer[1024];
		sprintf(buffer, "Save failed %s (%d)", m_pixelShaderProgramNode->m_absoluteFileName, m_pixelShaderProgramNode->m_compiledIndex);
		m_error.SetWindowText(buffer);
		return false;
	}

#if (D3D_SDK_VERSION & 0x7fffffff) >= 32
	ID3DXBuffer * disassembly = NULL;
	result = D3DXDisassembleShader(reinterpret_cast<unsigned long const *>(compiledShader->GetBufferPointer()), FALSE, NULL, &disassembly);
	if (SUCCEEDED(result) && disassembly)
	{
		CString windowText = reinterpret_cast<char const * >(disassembly->GetBufferPointer());
		windowText.Replace("\n", "\r\n");
		m_error.SetWindowText(windowText);
		m_error.LineScroll(2048);
		m_error.LineScroll(-1);
		m_error.Invalidate();

		disassembly->Release();
	}
	else
	{
		m_error.SetWindowText("Could not disassemble");
		m_error.Invalidate();
	}
#else
	char buffer[32];
	sprintf(buffer, "Compiled okay (%d)", m_pixelShaderProgramNode->m_compiledIndex);
	m_error.SetWindowText(buffer);
#endif

	if (compiledShader)
		compiledShader->Release();

	if (errors)
		errors->Release();

	return true;
}

// ----------------------------------------------------------------------

bool CPixelShaderProgramView::CompileHlsl()
{
	m_error.SetWindowText("Compiling...");
	m_error.Invalidate();

	ID3DXBuffer *compiledShader = NULL;
	ID3DXBuffer *errors = NULL;
	ID3DXConstantTable * constantTable = NULL;

	// make sure all the include files are saved before compiling
	CTemplateTreeNode::GetInstance()->GetIncludeTree().SaveAll();

	// get the text to compile
	CString compileText = m_pixelShaderProgramNode->m_text;

	// make parsing easier
	compileText.Remove('\r');

	CString target;
	CString processText = compileText + "\n";
	std::vector<CString> textureCoordinateSetTags;
	while (!processText.IsEmpty())
	{
		CString line = processText.SpanExcluding("\n");
		processText = processText.Mid(line.GetLength() + 1);

		CString left7 = line.Left(7);
		left7.MakeLower();

		// figure out what our compliation target is
		if (left7 == "//hlsl ")
		{
			target = line.Mid(7);
			target.TrimLeft(" \t");
			target.TrimRight(" \t");
		}
	}

	// now actually assemble the shader
	IncludeHandler includeHandler;
	HRESULT result = D3DXCompileShader(
		compileText,
		compileText.GetLength(),
		NULL,
		&includeHandler,
		"main",
		target,
		0, // D3DXSHADER_DEBUG D3DXSHADER_SKIPVALIDATION D3DXSHADER_SKIPOPTIMIZATION
		&compiledShader,
		&errors,
		&constantTable);

	if (constantTable)
	{
		constantTable->Release();
		constantTable = NULL;
	}

	if (FAILED(result))
	{
		if (errors)
		{
			//-- clean up returned error
			CString error (reinterpret_cast<const char *>(errors->GetBufferPointer()));
			error.Replace ("\n", "\r\n");
			m_error.SetWindowText(error);

			//-- highlight line with error
			if (error [0] == '(')
			{
				//-- error line numbers are returned one of two ways, including and not including comments
				if (error [1] == 'S')
				{
					//-- handle not including comments (not worth it)
				}
				else
				{
					//-- handle the error case of the line number including comments
					const int index = error.Find (')');
					if (index != -1)
					{
						CString source;
						m_source.GetWindowText (source);

						CString desiredLineNumber = error.Mid (1, index - 1);
						int lineNumber = atoi (desiredLineNumber) - 1;

						int start = 0;
						while (lineNumber > 0)
						{
							start = source.Find ('\n', start + 1);
							--lineNumber;
						}

						int end = source.Find ('\n', start + 1);
						if (end == -1)
							end = source.GetLength ();

						if (start != end)
						{
							m_source.SetSel (start, end);
							m_source.SetFocus ();
						}
					}
				}
			}
		}
		else
			m_error.SetWindowText("No error message returned");

		if (compiledShader)
		{
			compiledShader->Release();
			compiledShader = NULL;
		}

		if (errors)
		{
			errors->Release();
			errors = NULL;
		}

		return false;
	}

	delete [] m_pixelShaderProgramNode->m_exe;
	m_pixelShaderProgramNode->m_exeLength = compiledShader->GetBufferSize();
	m_pixelShaderProgramNode->m_exe = new byte[m_pixelShaderProgramNode->m_exeLength];
	m_pixelShaderProgramNode->m_compiledIndex = ++CPixelShaderNode::ms_compiledIndex;
	memcpy(m_pixelShaderProgramNode->m_exe, compiledShader->GetBufferPointer(), m_pixelShaderProgramNode->m_exeLength);

	if (!m_pixelShaderProgramNode->Save())
	{
		char buffer[1024];
		sprintf(buffer, "Save failed %s (%d)", m_pixelShaderProgramNode->m_absoluteFileName, m_pixelShaderProgramNode->m_compiledIndex);
		m_error.SetWindowText(buffer);
		return false;
	}


#if (D3D_SDK_VERSION & 0x7fffffff) >= 32
	ID3DXBuffer * disassembly = NULL;
	result = D3DXDisassembleShader(reinterpret_cast<unsigned long const *>(compiledShader->GetBufferPointer()), FALSE, NULL, &disassembly);
	if (SUCCEEDED(result) && disassembly)
	{
		CString windowText = reinterpret_cast<char const * >(disassembly->GetBufferPointer());
		windowText.Replace("\n", "\r\n");
		m_error.SetWindowText(windowText);
		m_error.LineScroll(2048);
		m_error.LineScroll(-1);
		m_error.Invalidate();

		disassembly->Release();
		disassembly = NULL;
	}
	else
	{
		m_error.SetWindowText("Could not disassemble");
		m_error.Invalidate();
	}
#else
	char buffer[32];
	sprintf(buffer, "Compiled okay (%d)", m_pixelShaderProgramNode->m_compiledIndex);
	m_error.SetWindowText(buffer);
#endif

	if (compiledShader)
	{
		compiledShader->Release();
		compiledShader = NULL;
	}

	if (errors)
	{
		errors->Release();
		errors = NULL;
	}

	return true;
}

// ----------------------------------------------------------------------

bool CPixelShaderProgramView::Compile()
{
	CString left = m_pixelShaderProgramNode->m_text.Left(6);
	left.MakeLower();
	if (left == "//hlsl")
		return CompileHlsl();

	return CompileAsm();
}

// ----------------------------------------------------------------------

void CPixelShaderProgramView::OnPixelshaderprogramCompile()
{
	Compile();
}

// ----------------------------------------------------------------------

void CPixelShaderProgramView::OnChangePixelshaderprogramSource()
{
	m_source.GetWindowText(m_pixelShaderProgramNode->m_text);
}

// ----------------------------------------------------------------------

void CPixelShaderProgramView::OnPixelshaderprogramCreate()
{
	CFileDialog fileDialog(FALSE, "*.psh", 0, OFN_NOCHANGEDIR, "Pixel Shaders *.psh|*.psh||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("PixelShaderProgram");
	fileDialog.m_ofn.lpstrTitle      = "Create Pixel Shader";

	char buffer[MAX_PATH];
	CString createFileName;
	m_pixelShaderProgramNode->GetName(createFileName);
	if (!createFileName.IsEmpty())
	{
		const char *fileName = createFileName;
		if (strrchr(fileName, '\\'))
			fileName = strrchr(fileName, '\\') + 1;
		if (strrchr(fileName, '/'))
			fileName = strrchr(fileName, '/') + 1;
		strcpy(buffer, fileName);
		fileDialog.m_ofn.lpstrFile = buffer;
		fileDialog.m_ofn.nMaxFile = sizeof(buffer);
	}

	if (fileDialog.DoModal() == IDOK)
	{
		// Get the requested file name and strip the tree file search path off of it
		CString requestedFileName = fileDialog.GetPathName();
		CString strippedFileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(requestedFileName)).c_str();

		// check what the file name is supposed to be
		CString requiredFileName;
		m_pixelShaderProgramNode->GetName(requiredFileName);

		if (strippedFileName == requiredFileName)
		{
			m_pixelShaderProgramNode->m_absoluteFileName = requestedFileName;

			m_fileName.SetWindowText(requestedFileName);

			m_create.ShowWindow(SW_HIDE);
			m_compile.ShowWindow(SW_SHOW);
			m_error.ShowWindow(SW_SHOW);
			m_source.ShowWindow(SW_SHOW);
			m_source.SetReadOnly(false);

			m_pixelShaderProgramNode->m_text = "ps.1.0\r\n\r\ntex t0\r\nmov r0, t0\r\n";
			m_source.SetWindowText(m_pixelShaderProgramNode->m_text);
			Compile();
		}
	}
}

// ----------------------------------------------------------------------

void CPixelShaderProgramView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	if (!m_font)
	{
		m_font = new CFont();
		m_font->CreateFont(
		   -14,                       // nHeight
		   0,                         // nWidth
		   0,                         // nEscapement
		   0,                         // nOrientation
		   FW_NORMAL,                 // nWeight
		   FALSE,                     // bItalic
		   FALSE,                     // bUnderline
		   0,                         // cStrikeOut
		   ANSI_CHARSET,              // nCharSet
		   OUT_DEFAULT_PRECIS,        // nOutPrecision
		   CLIP_DEFAULT_PRECIS,       // nClipPrecision
		   DEFAULT_QUALITY,           // nQuality
		   DEFAULT_PITCH | FF_MODERN, // nPitchAndFamily
		   "Courier New");                // lpszFacename
	}

	m_source.SetFont(m_font, FALSE);
	m_error.SetFont(m_font, FALSE);
}

void CPixelShaderProgramView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	if (cy < 600)
		cy = 600;

	if (m_pixelShaderProgramNode)
	{
		RECT rect;
		m_compile.GetWindowRect(&rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		m_compile.MoveWindow((cx - width) / 2, cy - 60 - 4 - height, width, height);

		m_error.MoveWindow(4, cy - 64, cx - 4, 60);

		m_source.MoveWindow(4, 110, cx - 4, cy - 200);
	}
}

// ----------------------------------------------------------------------

BOOL CPixelShaderProgramView::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_TAB))
	{
		// get the char index of the caret position
		int nPos = LOWORD(m_source.CharFromPos(m_source.GetCaretPos()));

		// select zero chars
		m_source.SetSel(nPos, nPos);

		// then replace that selection with a TAB
		m_source.ReplaceSel("\t", TRUE);

		// no need to do a msg translation, so quit. that way no further processing gets done
		return TRUE;
	}
	else
		if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_F7))
			CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree().CompileAll(m_pixelShaderProgramNode);

	//just let other massages to work normally
	return CFormView::PreTranslateMessage(pMsg);
}

// ======================================================================
