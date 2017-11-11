// Commands.cpp : implementation file
//

#include "stdafx.h"
#include "hash_plugin.h"
#include "Commands.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommands

CCommands::CCommands()
{
	m_pApplication = NULL;
}

CCommands::~CCommands()
{
	ASSERT (m_pApplication != NULL);
	m_pApplication->Release();
}

void CCommands::SetApplicationObject(IApplication* pApplication)
{
	// This function assumes pApplication has already been AddRef'd
	//  for us, which CDSAddIn did in its QueryInterface call
	//  just before it called us.
	m_pApplication = pApplication;
}


/////////////////////////////////////////////////////////////////////////////
// CCommands methods

typedef unsigned int uint32;

uint32 calculate(const char *string);


STDMETHODIMP CCommands::Hash_pluginCommandMethod() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: Replace this with the actual code to execute this command
	//  Use m_pApplication to access the Developer Studio Application object,
	//  and VERIFY_OK to see error strings in DEBUG builds of your add-in
	//  (see stdafx.h)


	// ----------
	
	CComPtr<IDispatch> pDispDoc;
	m_pApplication->get_ActiveDocument(&pDispDoc);

	m_pDoc = pDispDoc;

	CString oldText;

	if (m_pDoc)
	{
		CComPtr<IDispatch> pDispSel;
		CComBSTR bstr;
		CString strText;
		HRESULT hr;

		m_pDoc->get_Selection(&pDispSel);

		m_pSel = pDispSel;

		hr = m_pSel->get_Text(&bstr);
		if (SUCCEEDED(hr))
		{
			oldText = bstr;

			if(!oldText.IsEmpty())
			{
				CString newText;

				// Write the value as a hexadecimal value to the string, pad it to 8 characters,
				// and tack on the 0x at the beginning

				newText.Format("%X",calculate(oldText));

				while(newText.GetLength() != 8)
				{
					newText = "0" + newText;
				}

				newText = "0x" + newText;

				CComBSTR newComString = newText;

				m_pSel->put_Text(newComString);
			}
		}
	}

	// ----------

//	VERIFY_OK(m_pApplication->EnableModeless(VARIANT_FALSE));
//	::MessageBox(NULL, oldText, "Hash_plugin", MB_OK | MB_ICONINFORMATION);
//	VERIFY_OK(m_pApplication->EnableModeless(VARIANT_TRUE));
	return S_OK;
}
