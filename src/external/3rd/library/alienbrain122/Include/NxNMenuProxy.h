#ifndef INC_NXN_MENU_PROXY_H
#define INC_NXN_MENU_PROXY_H

/*	\file		NxNMenuProxy.h
 *	
 *	\brief		Encapsulation of the menu COM object. 
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
//	Encapsulation of response object.
//---------------------------------------------------------------------------
class CNxNMenuProxy
{
friend CNxNNamespaceProxy;

public:
    CNxNMenuProxy();
    virtual ~CNxNMenuProxy();

    HRESULT GetProperty(CNxNProperty& propProperty);
    HRESULT SetProperty(const CNxNProperty& propProperty);

    HRESULT GetFirst(CNxNMenuProxy*& pChildMenu);
    HRESULT GetNext(CNxNMenuProxy*& pChildMenu);

    HRESULT GetInfo(long& lMenuItemFlags, CNxNString& strOpCode, CNxNString& strDisplayString, CNxNString& strHelpString);
    HRESULT GetParameter(CNxNString& strParameter);
    HRESULT GetState(BOOL& bGrayed, BOOL& bDefault);
    HRESULT GetFlags(long& lFlags);

protected:
    HRESULT Attach( INxNMenu* pMenu );

private:
    INxNMenu* m_pMenu;
};



#endif // INC_NXN_MENU_PROXY_H
