// DSAddIn.h : header file
//

#if !defined(AFX_DSADDIN_H__186E83CF_8131_4A82_898C_9A92974AA6E5__INCLUDED_)
#define AFX_DSADDIN_H__186E83CF_8131_4A82_898C_9A92974AA6E5__INCLUDED_

#include "commands.h"

// {80D94359-4337-469E-A789-8063FA3F94D5}
DEFINE_GUID(CLSID_DSAddIn,
0x80d94359, 0x4337, 0x469e, 0xa7, 0x89, 0x80, 0x63, 0xfa, 0x3f, 0x94, 0xd5);

/////////////////////////////////////////////////////////////////////////////
// CDSAddIn

class CDSAddIn : 
	public IDSAddIn,
	public CComObjectRoot,
	public CComCoClass<CDSAddIn, &CLSID_DSAddIn>
{
public:
	DECLARE_REGISTRY(CDSAddIn, "Hash_plugin.DSAddIn.1",
		"HASH_PLUGIN Developer Studio Add-in", IDS_HASH_PLUGIN_LONGNAME,
		THREADFLAGS_BOTH)

	CDSAddIn() {}
	BEGIN_COM_MAP(CDSAddIn)
		COM_INTERFACE_ENTRY(IDSAddIn)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CDSAddIn)

// IDSAddIns
public:
	STDMETHOD(OnConnection)(THIS_ IApplication* pApp, VARIANT_BOOL bFirstTime,
		long dwCookie, VARIANT_BOOL* OnConnection);
	STDMETHOD(OnDisconnection)(THIS_ VARIANT_BOOL bLastTime);

protected:
	CCommandsObj* m_pCommands;
	DWORD m_dwCookie;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSADDIN_H__186E83CF_8131_4A82_898C_9A92974AA6E5__INCLUDED)
