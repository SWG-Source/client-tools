// Commands.h : header file
//

#if !defined(AFX_COMMANDS_H__ED6FFA54_83AA_4DB7_BF58_488A54EA9283__INCLUDED_)
#define AFX_COMMANDS_H__ED6FFA54_83AA_4DB7_BF58_488A54EA9283__INCLUDED_

#include "hash_pluginTypes.h"

class CCommands : 
	public CComDualImpl<ICommands, &IID_ICommands, &LIBID_Hash_plugin>, 
	public CComObjectRoot,
	public CComCoClass<CCommands, &CLSID_Commands>
{
protected:

	IApplication* m_pApplication;

	CComQIPtr<ITextDocument, &IID_ITextDocument> m_pDoc;
	CComQIPtr<ITextSelection, &IID_ITextSelection> m_pSel;

public:
	CCommands();
	~CCommands();
	void SetApplicationObject(IApplication* m_pApplication);

	BEGIN_COM_MAP(CCommands)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ICommands)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CCommands)

public:
// ICommands methods
	STDMETHOD(Hash_pluginCommandMethod)(THIS);
};

typedef CComObject<CCommands> CCommandsObj;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMANDS_H__ED6FFA54_83AA_4DB7_BF58_488A54EA9283__INCLUDED)
