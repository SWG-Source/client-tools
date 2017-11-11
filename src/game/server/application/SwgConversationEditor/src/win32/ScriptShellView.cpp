// ======================================================================
//
// ScriptShellView.cpp
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ScriptShellView.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedUtility/FileName.h"
#include "SwgConversationEditorDoc.h"

// ======================================================================

IMPLEMENT_DYNCREATE(ScriptShellView, CShellView)

// ======================================================================

ScriptShellView::ScriptShellView()
{
}

// ----------------------------------------------------------------------

ScriptShellView::~ScriptShellView()
{
}

// ======================================================================

BEGIN_MESSAGE_MAP(ScriptShellView, CShellView)
	//{{AFX_MSG_MAP(ScriptShellView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

#ifdef _DEBUG
void ScriptShellView::AssertValid() const
{
	CShellView::AssertValid();
}

void ScriptShellView::Dump(CDumpContext& dc) const
{
	CShellView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void ScriptShellView::OnUpdate(CView * const pSender, LPARAM const lHint, CObject * const pHint) 
{
	if (pSender != this)
	{
		switch (lHint)
		{
		case SwgConversationEditorDoc::H_shellCompile:
			{
				char const * const mochaCommand = ConfigFile::getKeyString ("SwgConversationEditor", "mochaCommand", 0);
				if (mochaCommand)
				{
					SwgConversationEditorDoc::ShellData * const shellData = safe_cast<SwgConversationEditorDoc::ShellData *> (pHint);
					if (shellData)
					{
						CString const releaseOption = shellData->m_debug ? " " : " --buildRelease ";
						CString const commandLine = mochaCommand + releaseOption + shellData->m_fullScriptFileName;

						CString buffer;
						buffer.Format ("-------------------- Compilation: %s %s --------------------\r\n", shellData->m_shortFileName, shellData->m_debug ? "Debug" : "Release");
						AddTexts (buffer);

						buffer.Format ("Created %s.stf\r\n", shellData->m_shortFileName);
						AddTexts (buffer);

						buffer.Format ("Created %s.script\r\n", shellData->m_shortFileName);
						AddTexts (buffer);

						buffer.Format ("Compiling %s.script\r\n", shellData->m_shortFileName);
						AddTexts (buffer);

						CreateShellRedirect (commandLine);
					}
				}
				else
					MessageBox ("Could not spawn script compiler because 'mochaCommand' has not been defined in SwgConversationEditor.cfg");

				delete pHint;
			}
			break;

		case SwgConversationEditorDoc::H_shellP4edit:
			{
				SwgConversationEditorDoc::ShellData * const shellData = safe_cast<SwgConversationEditorDoc::ShellData *> (pHint);
				
				char const * const p4Command = ConfigFile::getKeyString ("SwgConversationEditor", "p4Command", 0);
				CString p4 = p4Command ? p4Command : "p4";

				if (shellData)
				{
					AddTexts ("-------------------- Perforce Edit/Add --------------------\r\n");

					CString pathName(GetDocument()->GetPathName());
					pathName.MakeLower();
					CreateShellRedirect (p4 + " edit " + pathName);
					CreateShellRedirect (p4 + " add " + pathName);
					CreateShellRedirect (p4 + " edit " + shellData->m_fullScriptFileName);
					CreateShellRedirect (p4 + " add " + shellData->m_fullScriptFileName);

					CString m_fullDataScriptFileName (shellData->m_fullScriptFileName);
					m_fullDataScriptFileName.Replace ("/dsrc/", "/data/");
					m_fullDataScriptFileName.Replace (".script", ".class");

					CreateShellRedirect (p4 + " edit " + m_fullDataScriptFileName);
					CreateShellRedirect (p4 + " add " + m_fullDataScriptFileName);

					CreateShellRedirect (p4 + " edit " + shellData->m_fullStringTableFileName);
					CreateShellRedirect (p4 + " add " + shellData->m_fullStringTableFileName);
				}

				delete pHint;
			}
			break;

		default:
			break;
		}
	}
}

// ======================================================================

