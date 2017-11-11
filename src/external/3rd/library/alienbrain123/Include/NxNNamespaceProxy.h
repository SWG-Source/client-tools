#ifndef INC_NXN_NAMESPACE_PROXY_H
#define INC_NXN_NAMESPACE_PROXY_H

/*	\file		NxNNamespaceProxy.h
 *	
 *	\brief		Encapsulation of the namespace COM object. 
 *              ONLY FOR INTERNAL USE!!!
 *	
 *	\author		Axel Pfeuffer, Helmut Klinger, Jens Riemschneider
 *	
 *	\version	1.0
 *	
 *	\date		2000
 *	
 *	\mod
 *		[jr]-03-Nov-2000 file created. Moved from NxNHelper.
 *	\endmod
 */


#include "NxNComPtr.h"


//---------------------------------------------------------------------------
//	Encapsulation of namespace.
//---------------------------------------------------------------------------
class CNxNNamespaceProxy
{
    friend class CNxNItem;

public:
    CNxNNamespaceProxy();
    virtual ~CNxNNamespaceProxy();

    HRESULT Request();
    HRESULT Release();

    bool ExistsInNamespace(const CNxNString& strNamespacePath);

    HRESULT GetFirstChild(const CNxNString& strNamespacePath, CNxNString& strFirstChild);
    HRESULT GetNextChild(const CNxNString& strNamespacePath, const CNxNString& strCurrentChild, CNxNString& strNextChild);
    HRESULT GetChildCount(const CNxNString& strNamespacePath, long& lChildCount);

    HRESULT GetProperty(CNxNProperty& propProperty, const CNxNString& strPath);
    HRESULT SetProperty(const CNxNProperty& propProperty, const CNxNString& strPath);
    HRESULT GetProperty(const CNxNString& sPath, const CNxNString& sPropName, CNxNString& sPropValue, long* pFlags);
    HRESULT SetProperty(const CNxNString& sPath, const CNxNString& sPropName, const CNxNString& sPropValue, long* pFlags);

    CNxNString GetNamespaceType(const CNxNString& strPath);
    CNxNString GetPropertyString(const CNxNString& strPropName, const CNxNString& strPath);

    bool OldInterfaceInstalled();

    HRESULT GetMenu(const CNxNString& strNamespacePath, CNxNMenuProxy& menu);

    HRESULT AddSelection(const CNxNString& strNamespacePath);
    HRESULT RemoveSelection(const CNxNString& strNamespacePath);
    HRESULT ClearSelection();
    HRESULT GetFirstSelection(CNxNString& strSelPath);
    HRESULT GetNextSelection(const CNxNString& strCurrentSelPath, CNxNString& strSelPath);

    HRESULT RunCommand( long hWnd, BSTR Path, BSTR OpCode, long Flags, CNxNResponseObjectProxy* pResponseObject, long cMaxParams, long* pcActualParams, BSTR* pParam );

    HRESULT Flush( const CNxNString& strNamespacePath, BOOL bRecursive, long Flags, BSTR Hint );

    HRESULT GetImageList(long hWnd, long lType, long* phImageList);
    HRESULT GetIcon(long  hWnd, 
                    BSTR  sPath, 
                    long* pNormal,
                    long* pSelected, 
                    long* pMask,
                    long* pState);
protected:
    HRESULT FindInternalObject(INxNNamespace_Internal* pNamespaceInternal, const CNxNString& strNamespacePath, INxNNamespaceObject_Internal** ppObjectInternal);

private:
    HRESULT DbObjRunCommand(long hWnd, BSTR Path, BSTR OpCode, long lFlags, CNxNParamArray& aParams);

private:
    INxNNamespace*           m_pNamespace;
    INxNNamespace_Internal*  m_pNamespaceInternal;

    int                      m_eApi;
};



#endif // INC_NXN_NAMESPACE_PROXY_H
