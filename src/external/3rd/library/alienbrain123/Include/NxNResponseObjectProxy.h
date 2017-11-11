#ifndef INC_NXN_RESPONSE_OBJECT_PROXY_H
#define INC_NXN_RESPONSE_OBJECT_PROXY_H

/*	\file		NxNResponseObjectProxy.h
 *	
 *	\brief		Encapsulation of the response COM object. 
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
class CNxNResponseObjectProxy
{
friend CNxNNamespaceProxy;

public:
    CNxNResponseObjectProxy();
    virtual ~CNxNResponseObjectProxy();

    HRESULT Request();
    HRESULT Release();

    HRESULT GetProperty(const CNxNString& strName, CNxNString& strValue);
    HRESULT SetProperty(const CNxNString& strName, const CNxNString& strValue);

    HRESULT SetDisplayed(BOOL bDisplayed);
    HRESULT WasDisplayed();

    HRESULT SetCanceled(BOOL bCanceled);
    HRESULT WasCanceled();

    HRESULT SetParentWindow(long hWnd);

protected:
    INxNResponseObject* GetComInterface() { return m_pResponseObject; }

private:
    INxNResponseObject*  m_pResponseObject;
};



#endif // INC_NXN_RESPONSE_OBJECT_PROXY_H
