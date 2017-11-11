// VertexShaderProgram.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "shaderbuilder.h"
#include "VertexShaderProgram.h"

#include "sharedFile/TreeFile.h"

#include "Node.h"
#include "RecentDirectory.h"
#include "IncludeHandler.h"

#include "..\..\..\Direct3d9\src\win32\Direct3d9_VertexShaderConstantRegisters.h"
#include "..\..\..\Direct3d9\src\win32\Direct3d9_VertexShaderVertexRegisters.h"

#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVertexShaderProgram

IMPLEMENT_DYNCREATE(CVertexShaderProgram, CFormView)

CVertexShaderProgram::CVertexShaderProgram()
	: CFormView(CVertexShaderProgram::IDD),
	m_font(0),
	m_vertexShaderProgramNode(NULL)
{
	//{{AFX_DATA_INIT(CVertexShaderProgram)
	//}}AFX_DATA_INIT
}

CVertexShaderProgram::~CVertexShaderProgram()
{
	m_font->DeleteObject();
	delete m_font;
}

void CVertexShaderProgram::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVertexShaderProgram)
	DDX_Control(pDX, IDC_VERTEXSHADERPROGRAM_CREATE, m_create);
	DDX_Control(pDX, IDC_VERTEXSHADERPROGRAM_COMPILE, m_compileButton);
	DDX_Control(pDX, IDC_VERTEXSHADERPROGRAM_STATICTEXT, m_staticText);
	DDX_Control(pDX, IDC_VERTEXSHADERPROGRAM_SOURCE, m_source);
	DDX_Control(pDX, IDC_VERTEXSHADERPROGRAM_FILENAME, m_fileName);
	DDX_Control(pDX, IDC_VERTEXSHADERPROGRAM_ERRORMESSAGE, m_error);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVertexShaderProgram, CFormView)
	//{{AFX_MSG_MAP(CVertexShaderProgram)
	ON_BN_CLICKED(IDC_VERTEXSHADERPROGRAM_COMPILE, OnVertexshaderprogramCompile)
	ON_EN_CHANGE(IDC_VERTEXSHADERPROGRAM_SOURCE, OnChangeVertexshaderprogramSource)
	ON_BN_CLICKED(IDC_VERTEXSHADERPROGRAM_CREATE, OnVertexshaderprogramCreate)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static int compilation;

/////////////////////////////////////////////////////////////////////////////
// CVertexShaderProgram diagnostics

#ifdef _DEBUG
void CVertexShaderProgram::AssertValid() const
{
	CFormView::AssertValid();
}

void CVertexShaderProgram::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CVertexShaderProgram message handlers

void CVertexShaderProgram::SetNode(CVertexShaderProgramNode &node)
{
	m_vertexShaderProgramNode = &node;

	m_fileName.SetWindowText(m_vertexShaderProgramNode->m_absoluteFileName);
	m_source.SetWindowText(m_vertexShaderProgramNode->m_text);
	m_error.SetWindowText("");
	m_source.LineScroll(m_vertexShaderProgramNode->m_scroll);

	if (m_vertexShaderProgramNode->m_absoluteFileName == "")
	{
		m_create.ShowWindow(SW_SHOW);
		m_compileButton.ShowWindow(SW_HIDE);
		m_error.ShowWindow(SW_HIDE);
		m_source.ShowWindow(SW_HIDE);
	}
	else
	{
		m_create.ShowWindow(SW_HIDE);
		m_compileButton.ShowWindow(SW_SHOW);
		m_error.ShowWindow(SW_SHOW);
		m_source.ShowWindow(SW_SHOW);

#if _MSC_VER < 1300
		const DWORD INVALID_FILE_ATTRIBUTES = 0xffffffff;
#endif

		DWORD attributes = GetFileAttributes(m_vertexShaderProgramNode->m_absoluteFileName);
		m_source.SetReadOnly(attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_READONLY) != 0);
	}
}

// ----------------------------------------------------------------------

void CVertexShaderProgram::SwitchAway()
{
	if (m_vertexShaderProgramNode)
	{
		m_vertexShaderProgramNode->m_scroll = m_source.GetFirstVisibleLine();
		m_vertexShaderProgramNode = NULL;
	}
}

// ----------------------------------------------------------------------

bool CVertexShaderProgram::CompileAsm()
{
	m_error.SetWindowText("Compiling...");
	m_error.Invalidate();

	ID3DXBuffer *compiledShader = NULL;
	ID3DXBuffer *errors = NULL;

	// make sure all the include files are saved before compiling
	CTemplateTreeNode::GetInstance()->GetIncludeTree().SaveAll();

	// get the text to compile
	CString compileText = m_vertexShaderProgramNode->m_text;

	D3DXMACRO macros[3];
	macros[0].Name = "TARGET";
	macros[0].Definition = "vs_1_1";
	macros[1].Name = "VERTEX_SHADER_VERSION";
	macros[1].Definition = "11";
	macros[2].Name = NULL;
	macros[2].Definition = NULL;

	// now actually assemble the shader
	IncludeHandler includeHandler;
	HRESULT result = D3DXAssembleShader(static_cast<const char *>(compileText), compileText.GetLength(), macros, &includeHandler, 0, &compiledShader, &errors);

	if (SUCCEEDED(result))
	{
		macros[0].Definition = "vs_2_0";
		macros[1].Definition = "20";
		result = D3DXAssembleShader(static_cast<const char *>(compileText), compileText.GetLength(), macros, &includeHandler, 0, &compiledShader, &errors);
	}

	if (FAILED(result) || !compiledShader)
	{
		if (errors)
		{
			//-- clean up returned error
			CString error (reinterpret_cast<const char *>(errors->GetBufferPointer()));
			errors->Release();
			errors = 0;

			error.Replace ("\n", "\r\n");
			m_error.SetWindowText(error);
			m_error.Invalidate();

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
			m_error.SetWindowText("Failed, but no error message returned");

		if (compiledShader)
		{
			compiledShader->Release();
			compiledShader = 0;
		}

		return false;
	}

	if (!m_vertexShaderProgramNode->Save())
	{
		char buffer[1024];
		sprintf(buffer, "Save failed %s", m_vertexShaderProgramNode->m_absoluteFileName);
		m_error.SetWindowText(buffer);
		compiledShader->Release();
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
	char buffer[64];
	sprintf(buffer, "Compiled okay %d", ++compilation);
	m_error.SetWindowText(buffer);
#endif

	if (compiledShader)
	{
		compiledShader->Release();
		compiledShader = 0;
	}

	return true;
}

// ----------------------------------------------------------------------

static CString getToken(CString & s)
{
	s.TrimRight(" \t");

	CString token = s.SpanExcluding(" \t");
	s.Delete(0, token.GetLength());
	s.TrimLeft(" \t");
	token.TrimLeft(" \t");
	token.TrimRight(" \t");
	return token;
}

bool CVertexShaderProgram::CompileHlsl()
{
	m_error.SetWindowText("Compiling...");
	m_error.Invalidate();

	ID3DXBuffer *compiledShader = NULL;
	ID3DXBuffer *errors = NULL;

	// make sure all the include files are saved before compiling
	CTemplateTreeNode::GetInstance()->GetIncludeTree().SaveAll();

	// get the text to compile
	CString shaderText = m_vertexShaderProgramNode->m_text;

	// make parsing easier
	shaderText.Remove('\r');
	shaderText += "\n";

	int success = 0;
	CString targets = shaderText.SpanExcluding("\n");
	CString token = getToken(targets);
	for (token = getToken(targets); !token.IsEmpty(); token = getToken(targets))
	{
		if (compiledShader)
		{
			compiledShader->Release();
			compiledShader = 0;
		}

		// now compile the shader
		IncludeHandler includeHandler;
		HRESULT result = D3DXCompileShader(shaderText, shaderText.GetLength(), NULL, &includeHandler, "main", token, 0, &compiledShader, &errors, NULL);

		if (compiledShader)
			success += 1;

		if (FAILED(result))
		{
			if (errors)
			{
				//-- clean up returned error
				CString error (reinterpret_cast<const char *>(errors->GetBufferPointer()));
				errors->Release();
				errors = 0;

				error.Replace ("\n", "\r\n");
				m_error.SetWindowText(error);
				m_error.Invalidate();

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
				m_error.SetWindowText("Failed, but no error message returned");

			return false;
		}
	}

	if (success == 0)
	{
		m_error.SetWindowText("No compilation targets specified");
		m_error.Invalidate();
		return false;
	}

	if (!m_vertexShaderProgramNode->Save())
	{
		char buffer[1024];
		sprintf(buffer, "Save failed %s", m_vertexShaderProgramNode->m_absoluteFileName);
		m_error.SetWindowText(buffer);
		m_error.Invalidate();
		return false;
	}

#if (D3D_SDK_VERSION & 0x7fffffff) >= 32
	ID3DXBuffer * disassembly = NULL;
	HRESULT result = D3DXDisassembleShader(reinterpret_cast<unsigned long const *>(compiledShader->GetBufferPointer()), FALSE, NULL, &disassembly);
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
	char buffer[64];
	sprintf(buffer, "Compiled okay %d", ++compilation);
	m_error.SetWindowText(buffer);
#endif

	if (compiledShader)
	{
		compiledShader->Release();
		compiledShader = 0;
	}

	return true;
}

// ----------------------------------------------------------------------

bool CVertexShaderProgram::Compile()
{
	CString left = m_vertexShaderProgramNode->m_text.Left(5);
	left.MakeLower();

	if (left == "//hls")
		return CompileHlsl();

	if (left == "//asm")
		return CompileAsm();

	m_error.SetWindowText("Need to specify //hlsl or //asm");
	return false;
}

// ----------------------------------------------------------------------

void CVertexShaderProgram::OnVertexshaderprogramCompile()
{
	Compile();
}

// ----------------------------------------------------------------------

void CVertexShaderProgram::OnChangeVertexshaderprogramSource()
{
	m_source.GetWindowText(m_vertexShaderProgramNode->m_text);
}

// ----------------------------------------------------------------------

void CVertexShaderProgram::OnVertexshaderprogramCreate()
{
	CFileDialog fileDialog(FALSE, "*.vsh", 0, OFN_NOCHANGEDIR, "Vertex Shaders *.vsh|*.vsh||");
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("VertexShaderProgram");
	fileDialog.m_ofn.lpstrTitle      = "Create Vertex Shader";

	char buffer[MAX_PATH];
	CString createFileName;
	m_vertexShaderProgramNode->GetName(createFileName);
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
		m_vertexShaderProgramNode->GetName(requiredFileName);

		if (strippedFileName == requiredFileName)
		{
			m_vertexShaderProgramNode->m_absoluteFileName = requestedFileName;
			FILE *f = fopen(requestedFileName, "ab");
			if (f)
				fclose(f);

			m_fileName.SetWindowText(requestedFileName);

			m_create.ShowWindow(SW_HIDE);
			m_compileButton.ShowWindow(SW_SHOW);
			m_error.ShowWindow(SW_SHOW);
			m_source.ShowWindow(SW_SHOW);
			m_source.SetReadOnly(false);
		}
		else
		{
			MessageBox (strippedFileName + " != " + requiredFileName + " -- " + "could not create vertex shader program file.\n1) Make sure the file reference file name matches the disk file name, or\n2) Make sure the directory where you are creating the file is in a valid search path.");
		}
	}
}

// ----------------------------------------------------------------------

void CVertexShaderProgram::OnInitialUpdate()
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
		   "Courier New");            // lpszFacename
	}

	m_source.SetFont(m_font, FALSE);
	m_error.SetFont(m_font, FALSE);
}

// ----------------------------------------------------------------------

void CVertexShaderProgram::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	if (cy < 600)
		cy = 600;

	if (m_vertexShaderProgramNode)
	{
		RECT rect;
		m_compileButton.GetWindowRect(&rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		m_compileButton.MoveWindow((cx - width) / 2, cy - 60 - 4 - height, width, height);

		m_error.MoveWindow(4, cy - 64, cx - 4, 60);

		m_source.MoveWindow(4, 110, cx - 4, cy - 200);
	}

}

// ----------------------------------------------------------------------

BOOL CVertexShaderProgram::PreTranslateMessage(MSG* pMsg)
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
			CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree().CompileAll(m_vertexShaderProgramNode);

	//just let other massages to work normally
	return CFormView::PreTranslateMessage(pMsg);
}

// ======================================================================
